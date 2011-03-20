/**
 * @file
 * @author  Oscar Pernas <oscar@pernas.es>
 * @version 0.1
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * Class that implements the persistence module of this library. Persistence file is
 * implemented using boost-serialization library and is written to a file per
 * connection to broker.
 * Each connection will be persisted in file persistence_file_1 for connection 1
 *
 */

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "ActivePersistence.h"
#include "../ActiveConnection.h"
#include "../../utils/exception/ActiveException.h"

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace ai;

LoggerPtr ActivePersistence::logger(Logger::getLogger("ActivePersistence"));



ActivePersistence::ActivePersistence(){
	dataFilename.str("");
	controlFilename.str("");
	//initializing the last packet sent
	lastSent=0;
	lastEnqueue=0;
	lastWrote=0;
	//initialized to false
	initialized=false;
	//recovery mode to false
	recoveryMode=false;

	//positin in file to 0
	positionInFile=0;

	activeConnection=NULL;
}

void ActivePersistence::init(ActiveConnection& activeConnectionR) {

	//Setting the persistence filename
	dataFilename.str("");
	dataFilename << "persistence_file_"<<activeConnectionR.getId();

	//setting the name for control persistence
	controlFilename.str("");
	controlFilename << "control_file_"<<activeConnectionR.getId();

	//initializing the last packet sent
	lastSent=0;
	lastEnqueue=0;
	lastWrote=0;

	//persistence is initialized and ready to use
	setSizePersistence(activeConnectionR.getSizePersistence());
	initialized=true;

	activeConnection=&activeConnectionR;

	if (isEnabled()){
		//initializing thread
		activePersistenceThread.init(*this);
		//starting thread
		activePersistenceThread.runPersistenceThread();
	}
}

void ActivePersistence::crashRecovery(){

	long howManyToSend=0;
	std::stringstream logMessage;
	try{
		persistenceMutex.lock();
		if (isEnabled()){
			long controlFileSent=getLastSentFromFile();
			long controlFileWrote=getNumberMessagesSerialized();
			lastEnqueue=controlFileSent;
			lastSent=controlFileSent;
			lastWrote=controlFileWrote;
			if (lastWrote>lastSent){
				logMessage<< "RECOVERING DATA: We need to recover from:"<<lastSent <<" to "<<lastWrote;
				LOG4CXX_DEBUG(logger,logMessage.str().c_str());
				setPositionToSend();
				setRecoveryMode(true);
				//we are going to send one message for each space
				//in the queue that we have
				//fix: but if the queue is bigger than the number of messages waiting
				//we have to calculate how many
				//for (int i=0; i<activeConnection->getMaxSizeQueue();i++){
				if (lastWrote-lastSent>activeConnection->getMaxSizeQueue()){
					howManyToSend=activeConnection->getMaxSizeQueue();
				}else{
					howManyToSend=lastWrote-lastSent;
				}
				//calculated and dispatching signals
				for (int i=0; i<howManyToSend; i++ ){
					newMessage(true);
				}
			}else{
				logMessage<< "PERSISTENCE: Nothing to recover :"<<lastSent <<" to "<<lastWrote;
				LOG4CXX_DEBUG(logger,logMessage.str().c_str());
			}
		}
		persistenceMutex.unlock();
	}catch (...){
		persistenceMutex.unlock();
	}
}

long ActivePersistence::getLastSentFromFile(){
	std::stringstream logMessage;

	long lastSentFromFile=0;
	if (isEnabled()){
		try{
			std::ifstream ifs (controlFilename.str().c_str(),std::ios::in);
			boost::archive::text_iarchive controlPersistence(ifs);
			controlPersistence>>lastSentFromFile;
//			logMessage << "Recovering we were in position "<<lastSentFromFile;
//			LOG4CXX_DEBUG(logger,logMessage.str().c_str());
			return lastSentFromFile;
		}catch (...){
			logMessage << "Error getting last sent from control file. File exists? Recovering from 0.";
			LOG4CXX_DEBUG (logger,logMessage.str().c_str());
			return 0;
		}
	}
	return 0;
}

long ActivePersistence::getNumberMessagesSerialized(){

	std::stringstream logMessage;
	bool moreToRead=true;
	long position=0;
	long localPositionInFile=0;
	ActiveMessage messageAux;
	try{
		std::ifstream ifs(dataFilename.str().c_str(), std::ios::in | std::ios::binary );
		while (moreToRead){
			try{
				ifs.seekg(localPositionInFile);
				boost::archive::binary_iarchive persistenceFile(ifs);
				persistenceFile >> messageAux;
				position++;
				localPositionInFile=ifs.tellg();
			}catch(...){
				moreToRead=false;
			}
		}
		return position;
	}catch (...){
		logMessage << "Error reading persistence_file. File exists? Messages cant be recovered.";
		LOG4CXX_DEBUG (logger,logMessage.str().c_str());
		return 0;
	}
	return 0;
}

void ActivePersistence::enqueue(){
	std::stringstream logMessage;
	try{
		persistenceMutex.lock();
		if (isEnabled()){
			ActiveMessage messageToEnqueue;
			if (getNextMessage(messageToEnqueue)){
				//std::cout << "antes del deliver"<< std::endl;
				if (activeConnection->deliver(messageToEnqueue)!=-1){
					newMessage(false);
					logMessage << "Message recovered from "<<getDataFilename();
					LOG4CXX_DEBUG (logger,logMessage.str().c_str());
				}else{
					newMessage(false);
					logMessage.str("DATA LOSS. Message was rejected by the queue.");
					LOG4CXX_FATAL (logger,logMessage.str().c_str());
				}
				//std::cout << "despues del deliver"<< std::endl;
			}else{
				newMessage(false);
				logMessage << "DATA LOSS. Error reading entry " << lastEnqueue << " from persistence file " << getDataFilename();
				LOG4CXX_FATAL (logger,logMessage.str().c_str());
			}
		}
		persistenceMutex.unlock();
		//sleep to a insignificant time to try to
		//give the processor to the producer thread to
		//try to dont be blocked without enqueue
		apr_sleep (1);
	}catch (ActiveException& ae){
		logMessage << ae.getMessage();
		LOG4CXX_FATAL (logger,logMessage.str().c_str());
		persistenceMutex.unlock();
	}catch (...){
		logMessage << "DATA LOSS. Message was not recovered from file with "<< lastEnqueue << " from persistence file " << getDataFilename();
		LOG4CXX_FATAL (logger,logMessage.str().c_str());
		persistenceMutex.unlock();
	}
}

bool ActivePersistence::getNextMessage(ActiveMessage& activeMessageR){

	std::stringstream logMessage;
	if (isEnabled()){
		try{
			std::ifstream ifs(dataFilename.str().c_str(), std::ios::in | std::ios::binary );
			ifs.seekg(positionInFile);
			boost::archive::binary_iarchive persistenceFile(ifs);
			persistenceFile >> activeMessageR;
			//persistenceFile.next_object_pointer(ifs);
			positionInFile=ifs.tellg();
			return true;
		}catch (boost::exception& be){
			logMessage << "POSSIBLE DATA LOSS. Message was not found in position "<<positionInFile;
			LOG4CXX_DEBUG (logger,logMessage.str().c_str());
			return false;
		}catch (...){
			logMessage << "POSSIBLE DATA LOSS. Message was not found in position "<<positionInFile;
			LOG4CXX_DEBUG (logger,logMessage.str().c_str());
			return false;
		}
	}
	return false;
}

void ActivePersistence::newMessage(bool received){
	try{
		activePersistenceThread.newMessage(received);
	}catch (ActiveException& ae){
		throw ae;
	}
}

void ActivePersistence::oneMoreEnqueued(){
	if (isEnabled()){
		lastEnqueue++;
	}
}

void ActivePersistence::oneMoreSent (bool dequeueInRecovery){
	std::stringstream logMessage;
	try{
		persistenceMutex.lock();
		if (isEnabled()){
			increaseSent();
			if (getRecoveryMode() && dequeueInRecovery){
				if (lastEnqueue==lastWrote){
					logMessage << "Recovery: All data is sent. Going back to normal mode";
					LOG4CXX_DEBUG(logger,logMessage.str().c_str());
					setRecoveryMode(false);
					activeConnection->setState(CONNECTION_RUNNING);
					positionInFile=0;
				}else if (activePersistenceThread.getActiveSharedObject()->getMessagesReady()<(lastWrote-lastEnqueue)){
						newMessage(true);
				}
			}
			rollFile();
		}
		persistenceMutex.unlock();
	}catch (ActiveException& ae){
		persistenceMutex.unlock();
		LOG4CXX_FATAL (logger,ae.getMessage());
	}
}

void ActivePersistence::increaseSent()  throw (ActiveException) {
	std::stringstream logMessage;

	if (isEnabled()){
		try{
			lastSent++;
			std::ofstream ofs (controlFilename.str().c_str(),std::ios::out | std::ios::trunc);
			boost::archive::text_oarchive controlPersistence(ofs);
			controlPersistence<< lastSent;
			//std::cout << "wrote last sent to control file " << lastSent << std::endl;
		}catch (...){
			logMessage << "Unknown exception increasing control file of persistence. Disk is full?";
			throw ActiveException(logMessage.str());
		}
	}
}

void ActivePersistence::rollFile() throw (ActiveException){
	std::stringstream logMessage;

	if (isEnabled()){
		try{
			//if we have the same number of messages
			//acked and sent and the file is over the
			//maximun size we are going to truncate
			//the persistence file
			if ((lastEnqueue==lastSent) && (lastEnqueue==lastWrote)
					&&(lastSent>=sizePersistence)){

				logMessage << "Rolling file " << dataFilename.str().c_str() << " with last sent "<< lastSent << "and lastEnqueue "<< lastEnqueue;
				LOG4CXX_DEBUG (logger,logMessage.str().c_str());

				std::ofstream ofsData (dataFilename.str().c_str(),std::ios::out | std::ios::trunc);
				std::ofstream ofsControl (controlFilename.str().c_str(),std::ios::out | std::ios::trunc);
				lastSent=0;
				lastEnqueue=0;
				lastWrote=0;
				positionInFile=0;
			}
		}catch(...){
			logMessage << "Unknown exception rolling persistence file. THIS IS BAD.";
			throw ActiveException(logMessage.str());
		}
	}
}

int ActivePersistence::serialize (ActiveMessage& activeMessage){
	std::stringstream logMessage;

	if (isEnabled()){
		try{
			persistenceMutex.lock();
			// create and open a character archive for output
			{
				std::ofstream ofs(dataFilename.str().c_str(), std::ios::binary | std::ios::app);
				boost::archive::binary_oarchive persistenceFile(ofs);
				persistenceFile << activeMessage;
			}
			lastWrote++;
			logMessage << "Object serialized :" << activeMessage.getText() << " in position " << lastWrote;
			LOG4CXX_DEBUG (logger,logMessage.str().c_str());
			//unlocking mutex
			persistenceMutex.unlock();
		}catch (boost::exception& be){
			//unlocking mutex
			persistenceMutex.unlock();
			logMessage << "POSSIBLE DATA LOSS. Error writing persistence file " << getDataFilename();
			LOG4CXX_FATAL (logger,logMessage.str().c_str());
			throw ActiveException (logMessage.str());
		}catch (...){
			//unlocking mutex
			persistenceMutex.unlock();
			logMessage << "POSSIBLE DATA LOSS. Error writing persistence file " << getDataFilename();
			LOG4CXX_FATAL (logger,logMessage.str().c_str());
			throw ActiveException (logMessage.str());
		}
	}
	return 0;
}

void ActivePersistence::deserialize (ActiveMessage& activeMessageR){
	std::stringstream logMessage;

	if (isEnabled()){
		try{
			persistenceMutex.lock();
			std::ifstream ifs(dataFilename.str().c_str(), std::ios::binary );
			boost::archive::binary_iarchive persistenceFile(ifs);
			persistenceFile >> activeMessageR;
			//unlocking mutex
			persistenceMutex.unlock();
		}catch (boost::exception& be){
			//unlocking mutex
			persistenceMutex.unlock();
			logMessage << "POSSIBLE DATA LOSS. Error writing persistence file " << getDataFilename();
			LOG4CXX_FATAL (logger,logMessage.str().c_str());
			throw ActiveException (logMessage.str());
		}catch (...){
			//unlocking mutex
			persistenceMutex.unlock();
			logMessage << "POSSIBLE DATA LOSS. Error writing persistence file " << getDataFilename();
			LOG4CXX_FATAL (logger,logMessage.str().c_str());
			throw ActiveException (logMessage.str());
		}
	}
}

int ActivePersistence::startRecoveryMode(){
	std::stringstream logMessage;
	if (isEnabled()){
		persistenceMutex.lock();
		setRecoveryMode(true);
		logMessage << "Started RecoveryMode. Message could not be inserted in the queue.";
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		//is started, now we are going to set the first position in the file to send
		//to set the cursor to this position (is an optimization)
		setPositionToSend();
		//setting flat to in persistence
		activeConnection->setState(CONNECTION_IN_PERSISTENCE);
		//unlocking mutex
		persistenceMutex.unlock();
		return 0;
	}
	return 1;
}


void ActivePersistence::setPositionToSend(){
	std::stringstream logMessage;
	long l=0;
	ActiveMessage activeMessageAux;
	if (isEnabled()){
		try{
			std::ifstream ifs(dataFilename.str().c_str(), std::ios::in | std::ios::binary );
			for (l=0; l<lastEnqueue; l++){
				ifs.seekg(positionInFile);
				boost::archive::binary_iarchive persistenceFile(ifs);
				persistenceFile >> activeMessageAux;
				positionInFile=ifs.tellg();
			}
			//positionInFile=ifs.tellg();
			logMessage.str("");
			logMessage << "Started recovery from file position "<<positionInFile << std::endl;
			LOG4CXX_DEBUG (logger,logMessage.str().c_str());
		}catch (boost::exception& be){
			logMessage << "Serializing exception. POSSIBLE DATA LOSS. Imposible to positioning to the position "<<positionInFile << std::endl;
			LOG4CXX_DEBUG (logger,logMessage.str().c_str());
		}catch (ActiveException& ae){
			logMessage << "Active exception. POSSIBLE DATA LOSS. " << ae.getMessage() << std::endl;
			LOG4CXX_DEBUG (logger,logMessage.str().c_str());
		}catch (boost::archive::archive_exception& stdexc){
			logMessage << "Boost archive exception. POSSIBLE DATA LOSS. "<<dataFilename.str() << std::endl;
			LOG4CXX_DEBUG (logger,logMessage.str().c_str());
		}catch (...){
			logMessage << "Unknown exception. POSSIBLE DATA LOSS. File exists? "<<dataFilename.str() << std::endl;
			LOG4CXX_DEBUG (logger,logMessage.str().c_str());
		}
	}
}

void ActivePersistence::resetFiles(){
	std::ofstream ofsData (dataFilename.str().c_str(),std::ios::out | std::ios::trunc);
	std::ofstream ofsControl (controlFilename.str().c_str(),std::ios::out | std::ios::trunc);
}

bool ActivePersistence::isEnabled(){
	if (getInitialized() && getSizePersistence()>0){
		return true;
	}
	return false;
}

void ActivePersistence::stopThread(){
	if (isEnabled()){
		activePersistenceThread.stop();
	}
}

ActivePersistence::~ActivePersistence() {

}
