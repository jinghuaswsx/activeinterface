/**
 * @file
 * @author  Oscar Pernas <oscar@pernas.es>
 * @version 1.2.2
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
 * Class that implements interface with application. Is the entry point to the library, and
 * gives users all methods to interact with the library and its components.
 *
 */


#include "ActiveInterface.h"
#include "core/ActiveManager.h"
#include "utils/exception/ActiveException.h"
#include "utils/exception/ActiveInputException.h"
#include "core/concurrent/ReadersWriters.h"

#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/propertyconfigurator.h"

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace ai;


LoggerPtr ActiveInterface::logger(Logger::getLogger("ActiveInterface"));

void ActiveInterface::init(const std::string& configurationFile)
	throw (ActiveException){

	std::stringstream logMessage;
     try{
    	 //configuration for logging
    	 PropertyConfigurator::configure("log4j.properties");
    	 LOG4CXX_DEBUG(logger, "Starting up ActiveInterface Library.");

    	 //initialize activeManager in singleton invoke all initializing
    	 //of data memory structures read from xml file configuration
    	 readersWriters.writerLock();
		 //setting state
		 setState(NOT_INITIALIZED);
    	 try{
    		 //initializing...
    		 ActiveManager::getInstance()->init(configurationFile,this);
    	 }catch(...){
    		 logMessage << "ActiveException. Library is not initialized by XML File.";
    		 LOG4CXX_DEBUG(logger, logMessage.str().c_str());
    	 }
		 //setting state
		 setState(INITIALIZED);
    	 //unlocking
    	 readersWriters.writerUnlock();

     } catch(log4cxx::helpers::Exception&) {
		 //setting state
		 setState(NOT_INITIALIZED);
    	 readersWriters.writerUnlock();
    	 logMessage << "Logging Exception. Shutting down ActiveInterface Library.";
    	 LOG4CXX_DEBUG(logger, logMessage.str().c_str());
    	 throw ActiveException(logMessage);
     }catch(ActiveException& ae) {
		 //setting state
		 setState(NOT_INITIALIZED);
    	 readersWriters.writerUnlock();
    	 logMessage << "ActiveException. Error initializing...";
    	 LOG4CXX_DEBUG(logger, logMessage.str().c_str());
    	 throw ae;
     }catch (...){
		 //setting state
		 setState(NOT_INITIALIZED);
    	 readersWriters.writerUnlock();
    	 logMessage << "UnknownException. Initializing library";
    	 LOG4CXX_DEBUG(logger, logMessage.str().c_str());
    	 throw ActiveException(logMessage);
     }
}

void ActiveInterface::startup()	throw (ActiveException){
	//starting services, after unlock to protect the deadlock if
	//the user uses blocking connection to broker
	//Its not protected because if the connection is blocking could make a
	//deadlock
	std::stringstream logMessage;
	try{
		ActiveManager::getInstance()->startConnections();
	}catch(ActiveException& ae){
		logMessage << "ActiveException. Error starting up." << std::endl;
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());
		throw ae;
	}catch(...){
		logMessage << "UnknownException. Starting up..." << std::endl;
   		LOG4CXX_DEBUG(logger, logMessage.str().c_str());
   		throw ActiveException(logMessage);
	}
}

void ActiveInterface::send(	std::string& serviceId,
							ActiveMessage& activeMessage)
	throw (ActiveException){

	std::stringstream logMessage;
	try{
		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}
		readersWriters.readerLock();
		ActiveManager::getInstance()->sendData(serviceId,activeMessage);
		readersWriters.readerUnlock();
	}catch (ActiveException& ae){
		readersWriters.readerUnlock();
		LOG4CXX_ERROR(logger,ae.getMessage().c_str());
		throw ae;
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.readerUnlock();
		logMessage << "Unknown exception sending data";
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
}

void ActiveInterface::send(	std::string& serviceId,
							ActiveMessage& activeMessage,
							std::list<int>& positionInQueue)
	throw (ActiveException){

	std::stringstream logMessage;
	try{
		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}
		readersWriters.readerLock();

		logMessage << "New message sent by the user" << std::endl;
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());

		ActiveManager::getInstance()->sendData(	serviceId,
												activeMessage,
												positionInQueue);
		readersWriters.readerUnlock();
	}catch (ActiveException& ae){
		readersWriters.readerUnlock();
		LOG4CXX_ERROR(logger,ae.getMessage().c_str());
		throw ae;
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.readerUnlock();
		logMessage << "Unknown exception sending data";
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
}

void ActiveInterface::sendResponse(	std::string& connectionId,
									ActiveMessage& activeMessage)
	throw (ActiveException){

	std::stringstream logMessage;
	try{
		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}
		readersWriters.readerLock();
		ActiveManager::getInstance()->sendResponse(connectionId,activeMessage);
		readersWriters.readerUnlock();
	}catch (ActiveException& ae){
		readersWriters.readerUnlock();
		LOG4CXX_ERROR(logger,ae.getMessage().c_str());
		throw ae;
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.readerUnlock();
		logMessage << "Unknown exception sending data";
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
}

ActiveConnection* ActiveInterface::newProducer(	std::string& id,
												std::string& ipBroker,
												std::string& destination,
												bool requestReply,
												bool topic,
												bool persistent,
												bool clientAck,
												int maxSizeQueue,
												const std::string& username,
												const std::string& password,
												const std::string& clientId,
												long persistence,
												const std::string& certificate)
	throw (ActiveException){

	std::stringstream logMessage;

	try{

		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}

		int type=1;
		if (requestReply){
			type=3;
		}

		logMessage << "-------------------------- PRODUCER -----------------------------" << std::endl;
		logMessage << "Creating new producer to " << ipBroker << " " << destination << std::endl;
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());

		readersWriters.writerLock();
		ActiveConnection* connectionPtr=ActiveManager::getInstance()->newConnection(
																		id,ipBroker,type,destination,
																		topic,persistent,"",false,
																		clientAck,maxSizeQueue,username,
																		password,clientId,persistence,
																		certificate);
		if (connectionPtr){
			readersWriters.writerUnlock();
			return connectionPtr;
		}else{
			throw ActiveException("Connection couldn't be saved in map");
		}

		logMessage.str("");
		logMessage << "-------------------------------------------------------"<< std::endl;
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());
	}catch (ActiveException& ae){
		readersWriters.writerUnlock();
		throw ae;
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.writerUnlock();
		logMessage.str("Unknown exception creating producer. Check logs.");
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
	return NULL;
}

ActiveConnection* ActiveInterface::newConsumer(	std::string& id,
												std::string& ipBroker,
												std::string& destination,
												bool requestReply,
												bool topic,
												bool durable,
												bool clientAck,
												const std::string& selector,
												const std::string& username,
												const std::string& password,
												const std::string& clientId,
												const std::string& certificate)
	throw (ActiveException){

	std::stringstream logMessage;
	try{

		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}

		int type=0;
		if (requestReply){
			type=2;
		}

		logMessage << "-------------------------- CONSUMER -----------------------------" << std::endl;
		logMessage << "Creating new consumer to " << ipBroker << " " << destination << std::endl;
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());

		readersWriters.writerLock();
		ActiveConnection* connectionPtr=ActiveManager::getInstance()->newConnection(
																	id, ipBroker,type,destination,
																	topic,false,selector,durable,clientAck,
																	0,username,password,clientId,0,
																	certificate);
		if (connectionPtr){
			readersWriters.writerUnlock();
			return connectionPtr;
		}else{
			throw ActiveException("Connection couldn't be saved in map");
		}

		logMessage.str("");
		logMessage << "-------------------------------------------------------" << std::endl;
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());
	}catch (ActiveException& ae){
		readersWriters.writerUnlock();
		logMessage << "Exception creating new consumer to " << ipBroker << " " << destination;
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());
		throw ae;
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.writerUnlock();
		logMessage.str("Unknown exception creating consumer");
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
	return NULL;
}

void ActiveInterface::newLink(	std::string& serviceId,
								std::string& linkId,
								std::string& name,
								std::string& connectionId,
								ParameterList& parameterList)
	throw (ActiveException){

	std::stringstream logMessage;
	try{

		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}

		logMessage << "Creating new link " << linkId << " to service " << serviceId
				<< " with connection " << connectionId;
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());

		readersWriters.writerLock();
		ActiveManager::getInstance()->newLink(	serviceId,linkId,name,
												connectionId,parameterList);
		readersWriters.writerUnlock();

	}catch (ActiveException& ae){
		readersWriters.writerUnlock();
		logMessage << "ActiveInterface::newLink. Exception " << connectionId
				<< " " << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
		throw ae;
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.writerUnlock();
		logMessage.str("Unknown exception creating link. Check logs.");
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
}

void ActiveInterface::addLink (std::string& serviceId, std::string& linkId)
	throw (ActiveException){

	std::stringstream logMessage;
	try{

		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}

		logMessage << "Adding new link " << linkId << " to service " << serviceId;
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());

		readersWriters.writerLock();
		ActiveManager::getInstance()->insertInMMap(serviceId,linkId);
		readersWriters.writerUnlock();

	}catch (ActiveException& ae){
		readersWriters.writerUnlock();
		logMessage 	<< "ActiveInterface::addLink. Exception " << linkId
					<< " " << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
		throw ae;
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.writerUnlock();
		logMessage.str("Unknown exception adding link. Check logs.");
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
}

void ActiveInterface::getLinksByConn(std::string& connectionId, std::list<ActiveLink*>& linkList)
	throw (ActiveException){

	std::stringstream logMessage;
	try{

		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}

		readersWriters.readerLock();
		ActiveManager::getInstance()->getLinksByConn(connectionId,linkList);
		readersWriters.readerUnlock();
	}catch (ActiveException& ae){
		readersWriters.readerUnlock();
		logMessage << "ActiveInterface::getLinksByConn. Exception " << connectionId
				<< " " << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.readerUnlock();
		logMessage.str("Unknown exception getting links. Check logs.");
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
}

void ActiveInterface::getLinksByService(std::string& serviceId, std::list<ActiveLink*>& linkList)
	throw (ActiveException){

	std::stringstream logMessage;
	try{
		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}
		readersWriters.readerLock();
		ActiveManager::getInstance()->getLinksByService(serviceId,linkList);
		readersWriters.readerUnlock();
	}catch (ActiveException& ae){
		readersWriters.readerUnlock();
		logMessage << "ActiveInterface::getLinksByService. Exception " <<
				serviceId << " " << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.readerUnlock();
		logMessage.str("Unknown exception getting link. Check logs.");
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
}

void ActiveInterface::getServicesByLink(std::string& linkId,
										std::list<std::string>& servicesIdList)
	throw (ActiveException){

	std::stringstream logMessage;
	try{
		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}
		readersWriters.readerLock();
		ActiveManager::getInstance()->getServicesByLink(linkId,servicesIdList);
		readersWriters.readerUnlock();
	}catch (ActiveException& ae){
		readersWriters.readerUnlock();
		logMessage << "ActiveInterface::getServicesByLink. Exception " <<
				linkId << " " << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.readerUnlock();
		logMessage.str("Unknown exception getting services. Check logs.");
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
}

const ActiveConnection* ActiveInterface::getConnByLink(std::string& linkId)
	throw (ActiveException){

	ActiveConnection* ac=NULL;
	std::stringstream logMessage;
	try{
		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}
		readersWriters.readerLock();
		ac=ActiveManager::getInstance()->getConnByLink(linkId);
		readersWriters.readerUnlock();
	}catch (ActiveException& ae){
		readersWriters.readerUnlock();
		logMessage << "ActiveInterface::getConnByLink. Exception " <<
				linkId << " " << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.readerUnlock();
		logMessage.str("Unknown exception getting connections. Check logs.");
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
	return ac;
}

void ActiveInterface::getConnsByService (	std::string& serviceId,
											std::list<ActiveConnection*>& connectionListR)
	throw (ActiveException){

	std::stringstream logMessage;
	try{
		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}
		readersWriters.readerLock();
		ActiveManager::getInstance()->getConnsByService(serviceId,connectionListR);
		readersWriters.readerUnlock();
	}catch (ActiveException& ae){
		readersWriters.readerUnlock();
		logMessage << "ActiveInterface::getConnsByService. Exception " <<
				serviceId << " " << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.readerUnlock();
		logMessage.str("Unknown exception getting connections. Check logs.");
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
}

void ActiveInterface::getConnsByDestination(	std::string& destination,
												std::list<ActiveConnection*>& connectionListR)
	throw (ActiveException){

	std::stringstream logMessage;
	try{
		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}
		readersWriters.readerLock();
		ActiveManager::getInstance()->getConnsByDestination(destination,connectionListR);
		readersWriters.readerUnlock();
	}catch (ActiveException& ae){
		readersWriters.readerUnlock();
		logMessage << "ActiveInterface::getConnsByDestination. Exception " <<
				destination << " " << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.readerUnlock();
		logMessage.str("Unknown exception getting connections. Check logs.");
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
}

void ActiveInterface::getConnections(std::list<ActiveConnection*>& connectionListR) throw (ActiveException){
	std::stringstream logMessage;
	try{
		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}
		readersWriters.readerLock();
		ActiveManager::getInstance()->getConnections(connectionListR);
		readersWriters.readerUnlock();
	}catch (ActiveException& ae){
		readersWriters.readerUnlock();
		logMessage << "ActiveInterface::getConnections. Exception " << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.readerUnlock();
		logMessage.str("Unknown exception getting connections. Check logs.");
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
}

void ActiveInterface::getConnection(std::string& connectionId) throw (ActiveException){
	std::stringstream logMessage;
	try{
		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}
		readersWriters.readerLock();
		ActiveManager::getInstance()->getConnection(connectionId);
		readersWriters.readerUnlock();
	}catch (ActiveException& ae){
		readersWriters.readerUnlock();
		logMessage << "ActiveInterface::getConnection. Exception " << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.readerUnlock();
		logMessage.str("Unknown exception getting connections. Check logs.");
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
}

void ActiveInterface::getServices(std::list<std::string>& servicesList) throw (ActiveException){
	std::stringstream logMessage;
	try{
		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}
		readersWriters.readerLock();
		ActiveManager::getInstance()->getServices(servicesList);
		readersWriters.readerUnlock();
	}catch (ActiveException& ae){
		readersWriters.readerUnlock();
		logMessage << "ActiveInterface::getServices. Exception " << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.readerUnlock();
		logMessage.str("Unknown exception getting connections. Check logs.");
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
}

bool ActiveInterface::destroyConnection(std::string& connectionId) throw (ActiveException){

	bool returnValue=false;
	std::stringstream logMessage;
	try{
		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}
		readersWriters.writerLock();
		returnValue=ActiveManager::getInstance()->destroyConnection(connectionId);
		readersWriters.writerUnlock();
		return returnValue;
	}catch (ActiveException& ae){
		readersWriters.writerUnlock();
		logMessage << "ActiveInterface::destroyConnection. Exception " <<
				connectionId << " " << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.writerUnlock();
		logMessage.str("Unknown exception destroying connection. Check logs.");
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
	return false;
}

bool ActiveInterface::destroyLink(std::string& linkId) throw (ActiveException){

	bool returnValue=false;
	std::stringstream logMessage;
	try{
		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}
		readersWriters.writerLock();
		returnValue=ActiveManager::getInstance()->destroyLink(linkId);
		readersWriters.writerUnlock();
		return returnValue;
	}catch (ActiveException& ae){
		readersWriters.writerUnlock();
		logMessage << "ActiveInterface::destroyLink. Exception " <<
				linkId << " " << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.writerUnlock();
		logMessage.str("Unknown exception destroying links. Check logs.");
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
	return false;
}

bool ActiveInterface::destroyService(std::string& serviceId) throw (ActiveException){

	bool returnValue=false;
	std::stringstream logMessage;
	try{
		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}
		readersWriters.writerLock();
		returnValue=ActiveManager::getInstance()->destroyService(serviceId);
		readersWriters.writerUnlock();
		return returnValue;
	}catch (ActiveException& ae){
		readersWriters.writerUnlock();
		logMessage << "ActiveInterface::destroyService. Exception " <<
				serviceId << " " << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.writerUnlock();
		logMessage.str("Unknown exception destroying services. Check logs.");
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
	return false;
}

bool ActiveInterface::destroyServiceLink (	std::string& linkId,
											std::string& serviceId)
	throw (ActiveException){

	bool returnValue=false;
	std::stringstream logMessage;
	try{
		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}
		readersWriters.writerLock();
		returnValue=ActiveManager::getInstance()->destroyServiceLink(linkId, serviceId);
		readersWriters.writerUnlock();
		return returnValue;
	}catch (ActiveException& ae){
		readersWriters.writerUnlock();
		logMessage << "ActiveInterface::destroyServiceLink. Exception " <<
				linkId << " " << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.writerUnlock();
		logMessage.str("Unknown exception destroying service-links. Check logs.");
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
	return false;
}

bool ActiveInterface::destroyLinkConnection (std::string& linkId) throw (ActiveException){

	bool returnValue=false;
	std::stringstream logMessage;
	try{
		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}
		readersWriters.writerLock();
		returnValue=ActiveManager::getInstance()->destroyLinkConnection(linkId);
		readersWriters.writerUnlock();
		return returnValue;
	}catch (ActiveException& ae){
		readersWriters.writerUnlock();
		logMessage << "ActiveInterface::destroyLinkConnection. Exception " <<
				linkId << " " << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.writerUnlock();
		logMessage.str("Unknown exception destroying link-connections. Check logs.");
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
	return false;
}

bool ActiveInterface::setLinkConnection(std::string& linkId,
										std::string& connectionId)
	throw (ActiveException){

	bool returnValue=false;
	std::stringstream logMessage;
	try{
		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}
		readersWriters.writerLock();
		returnValue=ActiveManager::getInstance()->setLinkConnection(linkId, connectionId);
		readersWriters.writerUnlock();
		return returnValue;
	}catch (ActiveException& ae){
		readersWriters.writerUnlock();
		logMessage << "ActiveInterface::addLinkConnection. Exception " <<
				linkId << " " << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}catch(ActiveInputException& aie){
		LOG4CXX_ERROR(logger,aie.getMessage().c_str());
		throw ActiveException(aie.getMessage());
	}catch (...){
		readersWriters.writerUnlock();
		logMessage.str("Unknown exception getting connections. Check logs.");
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
	return false;
}

bool ActiveInterface::shutdown() throw (ActiveException){

	std::stringstream logMessage;
	try{
		logMessage << "ActiveInterface::shutdown. Shutting down singleton manager. ";
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
		if (getState()!=INITIALIZED){
			AI_THROW_AIE;
		}
		readersWriters.writerLock();

		//setting the flag that the library is closed for the user part
		setState(CLOSING);
		//deleting active manager
		delete ActiveManager::getInstance();
		//State to closed
		setState(CLOSED);

		readersWriters.writerUnlock();
	}catch (ActiveException& ae){
		readersWriters.writerUnlock();
		logMessage << "Exception shutting down library.";
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}catch (...){
		readersWriters.writerUnlock();
		logMessage.str("Unknown exception shutting down library. Check logs.");
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException(logMessage);
	}
	//logging it
	logMessage.str("ActiveInterface::shutdown. ActiveInterface is closed!");
	LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	return true;
}
