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
 * Class that uses activemq-cpp. Is the real class that implements the producer
 * to the broker. Is a class that inherits from ActiveConnection and that
 * implements all his virtual pure methods like deliver, or send.
 * This class also implements methods of consumer for request reply uses.
 *
 */

#include "ActiveProducer.h"
#include "../../utils/exception/ActiveException.h"

#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/core/ActiveMQConnection.h>
#include <activemq/library/ActiveMQCPP.h>

using namespace activemq::core;
using namespace ai::message;
using namespace log4cxx;
using namespace log4cxx::helpers;


LoggerPtr ActiveProducer::logger(Logger::getLogger("ActiveProducer"));

ActiveProducer::ActiveProducer(	std::string& idR,
								std::string& brokerURIRcvd,
								std::string& destURIRcvd,
								int maxSizeQueueR,
								std::string& usernameR,
								std::string& passwordR,
								std::string& clientIdR,
								std::string& certificateR,
								bool useTopicRcvd,
								bool getResponseRcvd,
								bool clientAckRcvd,
								bool deliveryModeRcvd,
								int persistentR){

	setId(idR);
	setClientId(clientIdR);
	setIpBroker(brokerURIRcvd);
	setDestination(destURIRcvd);
	setTopic(useTopicRcvd);
	setRequestReply(getResponseRcvd);
	setClientAck(clientAckRcvd);
	setPersistent(deliveryModeRcvd);
	setType(ACTIVE_PRODUCER);
	setMaxSizeQueue(maxSizeQueueR);
	setUsername(usernameR);
	setPassword(passwordR);
	setSizePersistence(persistentR);
	setState(CONNECTION_NOT_INITIATED);
	setCertificate(certificateR);

	//structures for producer
	connection = NULL;
	session = NULL;
	destination = NULL;
	producer = NULL;
    tempDest=NULL;
    responseConsumer=NULL;

    init();
}

void ActiveProducer::init (){

	///////////////////////////////////////////////
    //initializing read queue
    activeQueue.init(getMaxSizeQueue());
    //initializing callback queue
    activeCallbackQueue.init(0);

    //initiaing thread to send from queue
    activeThread.init(this);

    //initializing callback thread
    activeCallbackThread.init(activeCallbackQueue);
    activeCallbackThread.runCallbackThread();

    activePersistence.init(*this);

	//method to know if the application crash and
	//we have to resend messages that was not sent but
	//was serialized.
	activePersistence.crashRecovery();

    //initializing ssl support
	#ifdef WITH_SSL
		initSSLSupport();
	#endif
    ////////////////////////////////////////////////
}

void ActiveProducer::run() throw (ActiveException){

	std::stringstream logMessage;

	try {

		//if connection is initiated before, dont do anything
		if (getState()==CONNECTION_RUNNING){

			logMessage.str("");
			logMessage << "ActiveProducer::run. Producer was started before: "<< getId();
			LOG4CXX_DEBUG(logger, logMessage.str().c_str());

		}else{

			// Create a ConnectionFactory
			std::auto_ptr<ActiveMQConnectionFactory> connectionFactory(
					new ActiveMQConnectionFactory(getIpBroker()));

			try{
				// Create a Connection
				connection = connectionFactory->createConnection(	getIpBroker(),
																	getUsername(),
																	getPassword(),
																	getClientId());
				connection->start();
				connection->setExceptionListener(this);
			} catch( CMSException& e ) {
				logMessage << "ERROR: I can't stablish connection with broker, broker is up? address is good formatted? " << e.what();
				ActiveException activeException(logMessage);
				throw activeException;
			}

			// Create a Session
			if( getClientAck() ) {
				session = connection->createSession(Session::CLIENT_ACKNOWLEDGE);
			} else {
				session = connection->createSession(Session::AUTO_ACKNOWLEDGE );
			}

			// Create the destination (Topic or Queue)
			if( getTopic() ) {
				destination = session->createTopic( getDestination() );
			} else {
				destination = session->createQueue( getDestination() );
			}

			//creating temporary queue if we have to
			if (getRequestReply()){
				setType(ACTIVE_PRODUCER_RR);
				createTempQueue();
			}else{
				logMessage << "Request reply is disabled for connection id: "<< getId();
				LOG4CXX_DEBUG(logger, logMessage.str().c_str());
				logMessage.str("");
			}

			// Create a MessageProducer from the Session to the Topic or Queue
			producer = session->createProducer( destination );

			//setting message delivery mode
			if (getPersistent()){
				producer->setDeliveryMode(DeliveryMode::PERSISTENT);
			}else{
				producer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);
			}

			//setting flag state to running
			setState(CONNECTION_RUNNING);

			//run send thread
			activeThread.runSendThread();

			logMessage << "ActiveProducer::run. Producer is started succesfully: "<< getId();
			LOG4CXX_DEBUG(logger, logMessage.str().c_str());
		}

	} catch (ActiveException& e){
		//throw exception about this
		setState(CONNECTION_NOT_INITIATED);
		throw e;
	} catch (CMSException& e) {
		//throw exception about this
		setState(CONNECTION_NOT_INITIATED);
		logMessage << "Consumer::run CMSException. Check connection parameters. Broker is up?"<<e.what();
		throw ActiveException(logMessage.str());
	} catch (...){
		//throw exception about this
		setState(CONNECTION_NOT_INITIATED);
		logMessage << "Consumer::run Unknown exception. Check connection parameters. Broker is up?";
		throw ActiveException(logMessage.str());
	}
}

int ActiveProducer::send(){

	std::stringstream currentTime;
	std::stringstream logMessage;
	bool dequeuedInRecovery=false;
	ActiveMessage activeMessageToSend;

	try{

		//mutex for starting recovery mode
		activateRecoveryMutex.lock();

		if (getState()==CONNECTION_CLOSED){
			activeThread.newMessage(false);
			activateRecoveryMutex.unlock();
			logMessage << "Producer::send. POSSIBLE DATA LOSS. Producer " << getId() << " is closed. Send is not possible. Persistence is On?";
			LOG4CXX_ERROR(logger, logMessage.str().c_str());
			return -1;
		}

		activeThread.newMessage(false);

		activeQueue.dequeue(activeMessageToSend);
		if (activePersistence.getRecoveryMode()){
			dequeuedInRecovery=true;
		}

		if (connection != NULL || session != NULL || destination != NULL || producer != NULL){

			if (activeMessageToSend.isTextMessage()){

				//sending a textMessage
				TextMessage* textMessage=session->createTextMessage(activeMessageToSend.getText());

				insertUserPropertiesText(textMessage,activeMessageToSend);

				//inserting properties for request reply
				//current time for determines each packet and only one
				if (getRequestReply()){
					//to know where to answer
					textMessage->setCMSReplyTo(tempDest);
					//user has defined correlation id
					textMessage->setCMSCorrelationID(activeMessageToSend.getCorrelationId());
				}

				//mutex for starting recovery mode
				activateRecoveryMutex.unlock();

				logMessage << "Text message sent from connection "<< getId() << " to queue " << getDestination() << std::endl;
				LOG4CXX_DEBUG(logger, logMessage.str().c_str());

				//sending message
				producer->send(	textMessage,
								getPersistent(),
								activeMessageToSend.getPriority(),
								activeMessageToSend.getTimeToLive());

				if (getState()!=CONNECTION_CLOSED){

					isQueueReadyAgain(activeMessageToSend);

					activePersistence.oneMoreSent(dequeuedInRecovery);
				}

				//deleting memory for message
				delete textMessage;
			}else{
				//charging all data description in message
				loadPacketDesc(activeMessageToSend);

				//creating message text
				StreamMessage* streamMessage=session->createStreamMessage();
				streamMessage->setCMSType("ActiveMessage");

				////////////////////////////////////////////////////////////////////////////////////////
				insertParameters(streamMessage,activeMessageToSend);
				insertUserProperties(streamMessage,activeMessageToSend);

				//inserting properties for request reply
				//current time for determines each packet and only one
				if (getRequestReply()){
					//to know where to answer
					streamMessage->setCMSReplyTo(tempDest);
					//user has defined correlation id
					streamMessage->setCMSCorrelationID(activeMessageToSend.getCorrelationId());
				}

				//mutex for starting recovery mode
				activateRecoveryMutex.unlock();

				//sending message
				producer->send(	streamMessage,
								getPersistent(),
								activeMessageToSend.getPriority(),
								activeMessageToSend.getTimeToLive());

				if (getState()!=CONNECTION_CLOSED){
					isQueueReadyAgain(activeMessageToSend);

					activePersistence.oneMoreSent(dequeuedInRecovery);

					logMessage << "ActiveMessage sent from connection "<< getId() << " to queue " << getDestination();
					LOG4CXX_DEBUG(logger, logMessage.str().c_str());
				}

				//deleting memory for message
				delete streamMessage;
			}
			//mutex for starting recovery mode
			activateRecoveryMutex.unlock();
			return 0;

		}else{
			logMessage << "Producer::send producer is not initialized.";
			LOG4CXX_ERROR(logger, logMessage.str().c_str());
			//mutex for starting recovery mode
			activateRecoveryMutex.unlock();
			return -1;
		}
		return 1;
	}catch ( ActiveException& ae ){
		//mutex for starting recovery mode
		activateRecoveryMutex.unlock();
		logMessage << "Producer::sendData CMSException: " << ae.getMessage();
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		return -1;
	}catch (CMSException& cmse){
		//mutex for starting recovery mode
		activateRecoveryMutex.unlock();
		logMessage.str("Producer received a CMSException. We are going to close it. Reason: ");
		logMessage << cmse.what() << getId();
		LOG4CXX_FATAL (logger,logMessage.str().c_str());
		return -1;
	}
}

void ActiveProducer::insertParameters(StreamMessage* streamMessage, ActiveMessage& activeMessage)
	throw (ActiveException){

	std::string key;
	try{
		//inserting packet description
		if (activeMessage.getPacketDesc().size()==0){
			throw ActiveException ("ERROR. Empty packet. Nothing to send");
		}
		streamMessage->writeBytes(activeMessage.getPacketDesc());

		for (int it=0; it<activeMessage.getParametersSize();it++){
			Parameter* parameter=activeMessage.getParameter(it,key);
			switch (parameter->getType()){
			case ACTIVE_INT_PARAMETER:{
				IntParameter* intParameter=(IntParameter*)parameter;
				streamMessage->writeString(key);
				streamMessage->writeInt(intParameter->getValue());
			}
			break;
			case ACTIVE_REAL_PARAMETER:{
				RealParameter* realParameter=(RealParameter*)parameter;
				streamMessage->writeString(key);
				streamMessage->writeFloat(realParameter->getValue());
			}
			break;
			case ACTIVE_STRING_PARAMETER:{
				StringParameter* stringParameter=(StringParameter*)parameter;
				streamMessage->writeString(key);
				streamMessage->writeString(stringParameter->getValue());
			}
			break;
			case ACTIVE_BYTES_PARAMETER:{
				BytesParameter* bytesParameter=(BytesParameter*)parameter;
				streamMessage->writeString(key);
				streamMessage->writeBytes(bytesParameter->getValue());
			}
			break;
			}
		}
	}catch (CMSException& e){
		throw ActiveException(e.what());
	}
}

void ActiveProducer::insertUserProperties(StreamMessage* streamMessage, ActiveMessage& activeMessage)
	throw (ActiveException){

	std::string key;

	try{
		for (int it=0; it<activeMessage.getPropertiesSize();it++){
			Parameter* property=activeMessage.getProperty(it,key);
			switch (property->getType()){
			case ACTIVE_INT_PARAMETER:{
				IntParameter* intParameter=(IntParameter*)property;
				streamMessage->setIntProperty(key,intParameter->getValue());
				streamMessage->writeString(key);
			}
			break;
			case ACTIVE_REAL_PARAMETER:{
				RealParameter* realParameter=(RealParameter*)property;
				streamMessage->setFloatProperty(key,realParameter->getValue());
				streamMessage->writeString(key);
			}
			break;
			case ACTIVE_STRING_PARAMETER:{
				StringParameter* stringParameter=(StringParameter*)property;
				streamMessage->setStringProperty(key,stringParameter->getValue());
				streamMessage->writeString(key);
			}
			break;
			}
		}
	}catch (CMSException& e){
		throw ActiveException(e.what());
	}
}

void ActiveProducer::insertUserPropertiesText(TextMessage* textMessage, ActiveMessage& activeMessage)
	throw (ActiveException){

	std::string key;
	try{

		for (int it=0; it<activeMessage.getPropertiesSize();it++){
			Parameter* parameter=activeMessage.getProperty(it,key);
			switch (parameter->getType()){
			case ACTIVE_INT_PARAMETER:{
				IntParameter* intParameter=(IntParameter*)parameter;
				textMessage->setIntProperty(key, intParameter->getValue());
			}
			break;
			case ACTIVE_REAL_PARAMETER:{
				RealParameter* realParameter=(RealParameter*)parameter;
				textMessage->setFloatProperty(key,realParameter->getValue());
			}
			break;
			case ACTIVE_STRING_PARAMETER:{
				StringParameter* stringParameter=(StringParameter*)parameter;
				textMessage->setStringProperty(key,stringParameter->getValue());
			}
			break;
			}
		}
	}catch (CMSException& e){
		throw ActiveException(e.what());
	}
}

void ActiveProducer::createTempQueue(){

	std::stringstream logMessage;

	tempDest = session->createTemporaryQueue();
	responseConsumer = session->createConsumer(tempDest);
	// initializing consuming thread
	activeConsumerThread.init(this);

	//starting the consumer thread
	activeConsumerThread.runThread();

	logMessage << "Request reply is enable for connection id: "<< getId();
	LOG4CXX_DEBUG(logger, logMessage.str().c_str());
}

int ActiveProducer::onReceive(){
	int sizePacket=-1;

	std::stringstream logMessage;

	std::vector<unsigned char> packetDesc(MAX_PARAMETERS);

	try{
		//loop to get messages
		while (!getEndConsumerThread()){

			ActiveMessage activeMessage;
			activeMessage.setConnectionId(getId());
			std::auto_ptr<Message> message( responseConsumer->receive() );
			if (message.get()!=NULL){
				////////////////////////////////////////////////////////////////////
				/// Active message
				if (message->getCMSType()=="ActiveMessage"){
					StreamMessage* streamMessage=(StreamMessage*)message.get();
					sizePacket=streamMessage->readBytes(packetDesc);
					if (sizePacket!=-1){
						for (int it=0; it<sizePacket;it++){
							switch (packetDesc[it]){
							case ACTIVE_INT_PARAMETER:{
								std::string key=streamMessage->readString();
								int value=streamMessage->readInt();
								activeMessage.insertIntParameter(key,value);
							}
							break;
							case ACTIVE_REAL_PARAMETER:{
								std::string key=streamMessage->readString();
								float value=streamMessage->readFloat();
								activeMessage.insertRealParameter(key,value);
							}
							break;
							case ACTIVE_STRING_PARAMETER:{
								std::string key=streamMessage->readString();
								std::string value=streamMessage->readString();
								activeMessage.insertStringParameter(key,value);
							}
							break;
							case ACTIVE_BYTES_PARAMETER:{
								std::string key=streamMessage->readString();
								int sizeBytesData=packetDesc.at(++it);
								std::vector<unsigned char> data(sizeBytesData);
								streamMessage->readBytes(data);
								streamMessage->readBytes(data);
								activeMessage.insertBytesParameter(key,data);
							}
							break;
							case ACTIVE_INT_PROPERTY:{
								std::string key=streamMessage->readString();
								int value=streamMessage->getIntProperty(key);
								activeMessage.insertIntProperty(key,value);
							}
							break;
							case ACTIVE_REAL_PROPERTY:{
								std::string key=streamMessage->readString();
								float value=streamMessage->getFloatProperty(key);
								activeMessage.insertRealProperty(key,value);
							}
							break;
							case ACTIVE_STRING_PROPERTY:{
								std::string key=streamMessage->readString();
								std::string value=streamMessage->getStringProperty(key);
								activeMessage.insertStringProperty(key,value);
							}
							break;
							}
						}
					}else{
						logMessage << "Consumer::onMessage. Exceptions ocurred when message received. Packet description error";
						LOG4CXX_DEBUG(logger, logMessage.str().c_str());
					}
				/////////////////////////////////////////////////////////////
				// text message
				}else{
					TextMessage* textMessage=(TextMessage*)message.get();
					std::string textReceived=textMessage->getText();
					activeMessage.setText(textReceived);
					activeMessage.setMessageAsText();
					loadProperties(textMessage,activeMessage);
				}

				logMessage << "Message received from connection "<< getId() << std::endl;
				LOG4CXX_DEBUG(logger, logMessage.str().c_str());

				//inserting in message if i can answer if is a request reply consumer
				if (getRequestReply()){
					//setting parameter for how to make the answer
					activeMessage.setRequestReply(true);
					//Set the correlation ID from the received message
					std::string corId=message->getCMSCorrelationID();
					activeMessage.setCorrelationId(corId);
					//setting requestReply destination
					activeMessage.cloneDestination(message->getCMSReplyTo());
				}

				//setting others parameters to the message
				activeMessage.setLinkId(getLinkId());
				//sending callback to user with message
				ActiveManager::getInstance()->onMessageCallback(activeMessage);

				//message read sending acknowledge
				if( getClientAck() ) {
					message->acknowledge();
				}
			}else{
				///////////////////////////////////////////////////////////////
				logMessage << "Message received was null "<< getId();
				LOG4CXX_DEBUG(logger, logMessage.str().c_str());
			}
			//clearing all
			logMessage.str("");
		}
	} catch (CMSException& e) {
		logMessage << "Consumer::onMessage. Exceptions ocurred when message received. "<< e.what();
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		return -1;
	} catch (std::exception& e){
		logMessage << "Consumer::onMessage. Out of range retrieving packet. "<< e.what();
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		return -1;
	} catch (...){
		logMessage << "Consumer::onMessage. Unknown exception";
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		return -1;
	}

	return 0;
}

void ActiveProducer::loadProperties(TextMessage* textMessage, ActiveMessage& activeMessage)
	throw (ActiveException){

	bool found=false;
	std::string propertyName;
	int intProperty=0;
	float floatProperty=0;
	std::string stringProperty;

	try{
		std::vector<std::string> properties=textMessage->getPropertyNames();
		std::vector<std::string>::const_iterator cii;

		for(cii=properties.begin(); cii!=properties.end(); cii++){
			 propertyName=*cii;
			 try{
				 //getting an integer property
				 intProperty=textMessage->getIntProperty(propertyName);
				 activeMessage.insertIntProperty(propertyName,intProperty);
				 found=true;
			 }catch (CMSException& cms){
				 found=false;
			 }
			 if (!found){
				 try{
					 floatProperty=textMessage->getFloatProperty(propertyName);
					 activeMessage.insertRealProperty(propertyName,floatProperty);
					 found=true;
				 }catch (CMSException& cms){
					 found=false;
				 }
			 }
			 if (!found){
				 try{
					 stringProperty=textMessage->getStringProperty(propertyName);
					 activeMessage.insertStringProperty(propertyName,stringProperty);
					 found=true;
				 }catch (CMSException& cms){
					 found=false;
				 }
			 }
		}
	}catch(CMSException& cmsE){
		throw ActiveException(cmsE.what());
	}
}

int ActiveProducer::deliver (ActiveMessage& activeMessageR, ActiveLink& activeLink)
	throw (ActiveException){

	std::stringstream logMessage;
	std::list<std::string> defaultPropertysAdd;
	int position=-1;
	try{

		//if connection is running accepting messages into the queue
		if (getState()==CONNECTION_CLOSED){
			logMessage << "ERROR: Producer connection "<< getId() << " was closed.";
			LOG4CXX_ERROR(logger, logMessage.str().c_str());
			return -1;
		}

		//setting the connection id to the message to be marked
		activeMessageR.setConnectionId(getId());

		//copying properties to activemessage from activelink properties
		//this properties are default properties
		copyDefaultProperties(activeMessageR,activeLink,defaultPropertysAdd);

		//serializing the object into persistence file
		activePersistence.serialize(activeMessageR);

		//enqueue the message we are going to return the position in the queue
		if (!activePersistence.getRecoveryMode()){

			position=activeQueue.enqueue(activeMessageR);

			if (position==-1){

				activateRecoveryMutex.lock();

				//we've lost a packet we have to set library into
				//recovery mode (persistence on)
				if (activeQueue.isFull()){
					activePersistence.startRecoveryMode();
					//preparing to make the callback
					ActiveCallbackObject activeCallbackObject(	ON_PACKET_DROPPED,
																getId(),
																activeMessageR);
					activeCallbackQueue.enqueue(activeCallbackObject);
					activeCallbackThread.newCallback(true);

					activeQueue.setWorkingState(false);
				}else{
					position=activeQueue.enqueue(activeMessageR);
					if (position==-1){
						logMessage.str( "ERROR. POSSIBLE DATA LOSSS. Reenqueuing to the queue was full");
						LOG4CXX_DEBUG (logger,logMessage.str().c_str());
					}
					activePersistence.oneMoreEnqueued();
					activeThread.newMessage(true);
					logMessage.str( "Reenqueuing to the queue ok!");
					LOG4CXX_DEBUG (logger,logMessage.str().c_str());
				}
				activateRecoveryMutex.unlock();
			}else{
				activePersistence.oneMoreEnqueued();
				activeThread.newMessage(true);
				logMessage.str("");
				logMessage << "New message enqueued in connection " <<getId() << std::endl;
				LOG4CXX_DEBUG (logger,logMessage.str().c_str());
			}
		}
		//removing default properties
		removeDefaultProperties(activeMessageR,defaultPropertysAdd);
		return position;

	}catch(ActiveException e){
		removeDefaultProperties(activeMessageR,defaultPropertysAdd);
		logMessage 	<< "POSSIBLE DATA LOSS.. Error inserting message into the queue.  "
					<< e.getMessage();
		throw ActiveException (logMessage.str());
	}catch (...){
		removeDefaultProperties(activeMessageR,defaultPropertysAdd);
		logMessage 	<< "POSSIBLE DATA LOSS. Unknown error delivering data into the queue.  ";
		throw ActiveException (logMessage.str());
	}
	return -1;
}

int ActiveProducer::deliver (ActiveMessage& activeMessageR)	throw (ActiveException){

	std::stringstream logMessage;
	std::list<std::string> defaultPropertysAdd;
	int position=-1;
	try{

		//if connection is running accepting messages into the queue
		if (getState()==CONNECTION_CLOSED){
			logMessage << "ERROR: Producer in persistence was enqueuing data to connection "<<
							getId() << ", but was closed.";
			LOG4CXX_ERROR(logger, logMessage.str().c_str());
			return -1;
		}

		position=activeQueue.enqueue(activeMessageR);

		if (position==-1){

			//preparing to make the callback
			ActiveCallbackObject activeCallbackObject(	ON_PACKET_DROPPED,
														getId(),
														activeMessageR);
			activeCallbackQueue.enqueue(activeCallbackObject);
			activeCallbackThread.newCallback(true);

			activeQueue.setWorkingState(false);

			logMessage << "POSSIBLE DATA LOSS. Message could not be inserted in the queue by persistence";
			LOG4CXX_ERROR(logger, logMessage.str().c_str());

		}else{
			//std::cout << "antes del one more enqueued" << std::endl;
			activePersistence.oneMoreEnqueued();
			//std::cout << "despues y antes del one more enqueued" << std::endl;
			activeThread.newMessage(true);
			//std::cout << "despues del newmessage" << std::endl;
			logMessage << "Recovered message from. Enqueued.";
			LOG4CXX_DEBUG(logger, logMessage.str().c_str());
		}

		return position;
	}catch(ActiveException e){
		logMessage 	<< "POSSIBLE DATA LOSS.. Error inserting message into the queue.  "
					<< e.getMessage();
		throw ActiveException (logMessage.str());
	}catch (...){
		logMessage 	<< "POSSIBLE DATA LOSS. Unknown error delivering data into the queue.  ";
		throw ActiveException (logMessage.str());
	}
	return -1;
}

void ActiveProducer::removeDefaultProperties(	ActiveMessage& activeMessage,
												std::list<std::string>& defaultPropertiesAdd)
	throw (ActiveException){

	std::stringstream logMessage;

	try{
		std::list<std::string>::iterator it;
		//looping through default properties and inserting into activeMessage
		for(it=defaultPropertiesAdd.begin(); it != defaultPropertiesAdd.end(); ++it){
			//aqui borrare los parametros k tengan la key de active link
			activeMessage.deleteProperty(*it);
		}
	}catch (ActiveException& ae){
		logMessage << "ERROR. Cannot copy default properties to message" << ae.getMessage();
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ae;
	}

}

void ActiveProducer::copyDefaultProperties(	ActiveMessage& activeMessage,
											ActiveLink& activeLink,
											std::list<std::string>& defaultKeysList)
	throw (ActiveException){

	std::stringstream logMessage;

	std::string key;
	try{
		//looping through default properties and inserting into activeMessage
		for (int it=0; it<activeLink.getPropertySize();it++){
			Parameter* property=activeLink.getProperty(it,key);
			switch (property->getType()){
			case ACTIVE_INT_PARAMETER:{
				IntParameter* intProperty=(IntParameter*)property;
				try{
					activeMessage.insertIntProperty(key,intProperty->getValue());
					defaultKeysList.push_front(key);
				}catch (ActiveException& ae){
					throw ae;
				}
			}
			break;
			case ACTIVE_REAL_PARAMETER:{
				RealParameter* realProperty=(RealParameter*)property;
				try {
					activeMessage.insertRealProperty(key,realProperty->getValue());
					defaultKeysList.push_front(key);
				}catch (ActiveException& ae){
					throw ae;
				}
			}
			break;
			case ACTIVE_STRING_PARAMETER:{
				StringParameter* stringProperty=(StringParameter*)property;
				try{
					activeMessage.insertStringProperty(key,stringProperty->getValue());
					defaultKeysList.push_front(key);
				}catch (ActiveException& ae){
					throw ae;
				}
			}
			break;
			}
		}
	}catch (ActiveException& ae){
		logMessage << "ERROR. Cannot copy default properties to message" << ae.getMessage();
		throw ActiveException(logMessage.str());
	}
}

void ActiveProducer::onException( const CMSException& ex ) {
	std::stringstream logMessage;
	try{
		logMessage << "Producer::onException. Exception ocurred" << ex.what();
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());
		//making the object
		ActiveCallbackObject activeCallbackObject(	ON_EXCEPTION,
													getId(),
													ex.getMessage());
		activeCallbackQueue.enqueue(activeCallbackObject);
		activeCallbackThread.newCallback(true);
	}catch (ActiveException& ae){
		logMessage << "Producer::onException. Exception ocurred" << ae.getMessage();
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
	}
}

void ActiveProducer::transportInterrupted() {
	std::stringstream logMessage;
	try{
		logMessage << "Producer::transportInterrupted. The Connection's Transport has been interrupted."<< getClientId();
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());
		//making the object
		ActiveCallbackObject activeCallbackObject(	ON_TRANSPORT_INTERRUPT,
													getId());

		//sending response to manager
		activeCallbackQueue.enqueue(activeCallbackObject);
		activeCallbackThread.newCallback(true);
	}catch (ActiveException& ae){
		logMessage << "Producer::transportInterrupted. Exception ocurred" << ae.getMessage();
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
	}
}

void ActiveProducer::transportResumed() {
	std::stringstream logMessage;
	try{
		logMessage << "Producer::transportResumed. The Connection's Transport has been Restored."<< getClientId();
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());
		//making the object
		ActiveCallbackObject activeCallbackObject(	ON_TRANSPORT_RESUMED,
													getId());
		activeCallbackQueue.enqueue(activeCallbackObject);
		activeCallbackThread.newCallback(true);
	}catch (ActiveException& ae){
		logMessage << "Producer::transportResumed. Exception ocurred" << ae.getMessage();
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
	}
}

void ActiveProducer::isQueueReadyAgain(ActiveMessage& activeMessageR){
	if (!activeQueue.getWorkingState()){
		activeQueue.setWorkingState(true);
		//sends the callback that the queue is accepting
		//messages again
		ActiveCallbackObject activeCallbackObject(	ON_QUEUE_READY,
													getId(),
													activeMessageR);
		activeCallbackQueue.enqueue(activeCallbackObject);
		activeCallbackThread.newCallback(true);
	}
}

void ActiveProducer::start(){
	try{
		//starting perssistence
		activePersistence.init(*this);
		//run all
		run();
	}catch(ActiveException& ae){
		throw ae;
	}
}

void ActiveProducer::stop(){
	try{
		std::stringstream logMessage;
		logMessage << "Producer::stop. Stopping producer " << getIpBroker() <<" " << getDestination()<< "...";
		LOG4CXX_INFO(logger, logMessage.str().c_str());

		//setting state to close
		setState(CONNECTION_NOT_INITIATED);
		//ending consumer thread
		endConsumerThread();
		//ending persistence thread
		activePersistence.stopThread();
		//ending the producer thread
		activeThread.stop();
		//clean up
		cleanup();

		logMessage.str("");
		logMessage << "Producer::stop. Producer " <<  getId()<< " connected to " << getIpBroker() << " " << getDestination()<< " stopped succesfully!";
		LOG4CXX_INFO(logger, logMessage.str().c_str());

	}catch (ActiveException& ae){
		throw ae;
	}
}

void ActiveProducer::close() {
	std::stringstream logMessage;
	logMessage << "Producer::close. Closing producer " << getIpBroker() <<" " << getDestination()<< "...";
	LOG4CXX_INFO(logger, logMessage.str().c_str());

	//setting state to close
	setState(CONNECTION_CLOSED);
	//removing link connected to this producer
	ActiveManager::getInstance()->removeLinkBindingTo(getId());
	//ending consumer thread
	endConsumerThread();
	//ending persistence thread
	activePersistence.stopThread();
	//ending the producer thread
	activeThread.stop();
	//ending the callback thread
	activeCallbackThread.stop();
	//clean up
	cleanup();

	logMessage.str("");
	logMessage << "Producer::close. Producer " <<  getId()<< " connected to " << getIpBroker() << " " << getDestination()<< " closed succesfully!";
	LOG4CXX_INFO(logger, logMessage.str().c_str());
}

void ActiveProducer::cleanup(){

	/////////////////////////////////////////////////////////////////////////////
	// Destroy all resources
	std::stringstream logMessage;
	try{
		try{
			logMessage.str("");
			logMessage << "Producer::deleting destination...";
			LOG4CXX_INFO(logger, logMessage.str().c_str());
			if( destination != NULL ) delete destination;
		}catch ( CMSException& e ) {
			logMessage << "Producer::cleaning up CMSException: "<<e.what();
			LOG4CXX_INFO(logger, logMessage.str().c_str());
		}

		destination = NULL;

		try{
			logMessage.str("");
			logMessage << "Producer::deleting producer... ";
			LOG4CXX_INFO(logger, logMessage.str().c_str());
			if( producer != NULL ) delete producer;
		}catch ( CMSException& e ) {
			logMessage << "Producer::cleaning up CMSException: "<<e.what();
			LOG4CXX_INFO(logger, logMessage.str().c_str());
		}

		producer = NULL;

		// Close open resources.
		try{
			logMessage.str("");
			logMessage << "Producer::closing session... ";
			LOG4CXX_INFO(logger, logMessage.str().c_str());
			if( session != NULL ) session->close();
			logMessage.str("");
			logMessage << "Producer::closing connection... ";
			LOG4CXX_INFO(logger, logMessage.str().c_str());
			if( connection != NULL ) connection->close();
		}catch ( CMSException& e ) {
			logMessage << "Producer::cleaning up CMSException: "<<e.what();
			LOG4CXX_INFO(logger, logMessage.str().c_str());
		}

		try{
			logMessage.str("");
			logMessage << "Producer::deleting response consumer... ";
			LOG4CXX_INFO(logger, logMessage.str().c_str());
			if( responseConsumer != NULL ) delete responseConsumer;
		}catch (CMSException& e) {
			logMessage << "Producer::cleaning up CMSException: "<<e.what();
			LOG4CXX_INFO(logger, logMessage.str().c_str());
		}
		responseConsumer = NULL;

		try{
			logMessage.str("");
			logMessage << "Producer::deleting temporal destination... ";
			LOG4CXX_INFO(logger, logMessage.str().c_str());
			if( tempDest != NULL ) delete tempDest;
		}catch ( CMSException& e ) {
			logMessage << "Producer::cleaning up CMSException: "<<e.what();
			LOG4CXX_INFO(logger, logMessage.str().c_str());
		}
		tempDest=NULL;

		try{
			logMessage.str("");
			logMessage << "Producer::deleting session... ";
			LOG4CXX_INFO(logger, logMessage.str().c_str());
			if( session != NULL ) delete session;
		}catch ( CMSException& e ) {
			logMessage << "Producer::cleaning up CMSException: "<<e.what();
			LOG4CXX_INFO(logger, logMessage.str().c_str());
		}
		session = NULL;

		try{
			logMessage.str("");
			logMessage << "Producer::deleting connection... ";
			LOG4CXX_INFO(logger, logMessage.str().c_str());
			if( connection != NULL ) delete connection;
		}catch ( CMSException& e ) {
			logMessage << "Producer::cleaning up CMSException: "<<e.what();
			LOG4CXX_INFO(logger, logMessage.str().c_str());
		}catch (...){
			logMessage << "Producer::cleaning up. Unknown exception ocurred!!" << std::endl;
			LOG4CXX_INFO(logger, logMessage.str().c_str());
		}
		connection = NULL;

	}catch(...){
		logMessage << "Consumer::closed. Producer " << getId()<< " connected to " << getIpBroker() << " " << getDestination() << " closed with Exception!";
		LOG4CXX_INFO(logger, logMessage.str().c_str());
	}
}

ActiveProducer::~ActiveProducer(){
	//closing producer
	close();
}

