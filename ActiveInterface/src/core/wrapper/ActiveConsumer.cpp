/**
 * @file
 * @author  Oscar Pernas <oscar@pernas.es>
 * @version 1.2.3
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
 * Class that uses activemq-cpp. Is the real class that implements the consumer
 * to the broker. Is a class that inherits from ActiveConnection and that
 * implements all his virtual pure methods like onMessage.
 * This class also implements methods of producer for request reply uses.
 */

#include "ActiveConsumer.h"
#include "../../utils/exception/ActiveException.h"

#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/core/ActiveMQConnection.h>
#include <activemq/library/ActiveMQCPP.h>
#include <activemq/commands/ActiveMQMessage.h>

using namespace ai::message;
using namespace activemq::core;
using namespace log4cxx;
using namespace log4cxx::helpers;


LoggerPtr ActiveConsumer::logger(Logger::getLogger("ActiveConsumer"));


ActiveConsumer::ActiveConsumer( 	std::string& idR,
									std::string& brokerURIR,
									std::string& destURIR,
									std::string& messageSelectorR,
									int maxSizeQueueR,
									std::string& usernameR,
									std::string& passwordR,
									std::string& clientIdR,
									std::string& certificateR,
									bool useTopicR,
									bool clientAckR,
									bool responseToProducerR,
									bool durableR) {

	//////////////////////////////////////////////////////
	//settings for broker connection
	setId(idR);
	setClientId(clientIdR);
	setIpBroker(brokerURIR);
	setDestination(destURIR);
	setTopic(useTopicR);
	setSelector(messageSelectorR);
	setRequestReply(responseToProducerR);
	setDurable(durableR);
	setType(ACTIVE_CONSUMER);
	setPersistent(false);
	setClientAck(clientAckR);
	setMaxSizeQueue(maxSizeQueueR);
	setUsername(usernameR);
	setPassword(passwordR);
	setSizePersistence(0);
	setState(CONNECTION_NOT_INITIATED);
	setCertificate(certificateR);
	/////////////////////////////////////////////////////
	//structure pointers
	connection = NULL;
	session = NULL;
	destination = NULL;
	consumer = NULL;
	replyProducer = NULL;

	//calling to init function
	init();
}

void ActiveConsumer::init(){

	//////////////////////////////////////////////////
	// initializing consuming thread
	activeConsumerThread.init(this);

	//////////////////////////////////////////////////
    //initializing callback queue
    activeCallbackQueue.init(0);

    //initializing callback thread
    activeCallbackThread.init(activeCallbackQueue);
    activeCallbackThread.runCallbackThread();
    ///////////////////////////////////////////////////

    if (getRequestReply()){
    	//initializing read queue and read thread
    	activeQueue.init(getMaxSizeQueue());
    	//initiaing thread to send from queue
    	activeThread.init(this);
    }

    //initializing ssl support
    initSSLSupport();
}

void ActiveConsumer::run() 	throw (ActiveException){

	std::stringstream logMessage;

	try {

		//if connection is initiated before, dont do anything
		if (getState()==CONNECTION_RUNNING){

			logMessage.str("");
			logMessage << "ActiveConsumer::run. Consumer was started before: "<< getId();
			LOG4CXX_DEBUG(logger, logMessage.str().c_str());

		}else{
			// Create a ConnectionFactory
			ActiveMQConnectionFactory* connectionFactory =
					new ActiveMQConnectionFactory( getIpBroker() );

			// Create a Connection
			try{
				connection = connectionFactory->createConnection(	getIpBroker(),
																	getUsername(),
																	getPassword(),
																	getClientId());
			} catch( CMSException& e ) {
				//Delete connection factore
				delete connectionFactory;
				//giving traces
				logMessage << "ActiveConsumer::run I can't stablish connection with broker "<<e.what();
				ActiveException activeException(logMessage);
				throw activeException;
			}

			//deleting factory
			delete connectionFactory;

			ActiveMQConnection* amqConnection = dynamic_cast<ActiveMQConnection*>( connection );
			if( amqConnection != NULL ) {
				amqConnection->addTransportListener( this );
			}else{
				ActiveException activeException("Consumer::run. Exception, connection with broker is null.");
				throw activeException;
			}

			//starting connection with broker
			connection->start();

			// Create a Session
			if( getClientAck() ) {
				session = connection->createSession( Session::CLIENT_ACKNOWLEDGE );
			} else {
				session = connection->createSession( Session::AUTO_ACKNOWLEDGE );
			}

			//Setup a message producer to respond to messages from clients
			if (getRequestReply()){
				//setting the type
				setType(ACTIVE_CONSUMER_RR);
				//we will get the destination
				//to send to from the CMSReplyTo header field from a Message
				replyProducer = session->createProducer(NULL);
				replyProducer->setDeliveryMode(DeliveryMode::NON_PERSISTENT);
			}

			// Create the destination (Topic or Queue)
			if( getTopic() ) {
				// creating a durable consumer for be attached to a topic
				// and if connection resets forward all messages
				// be carefull, for durable consumers is not allowed client ids null
				destination = session->createTopic( getDestination() );
				if (isDurable()){
					consumer = session->createDurableConsumer(	(Topic*)destination,
																getClientId(),
																getSelector());
				}else{
					consumer =session->createConsumer((Topic*)destination,getSelector());
				}
			} else {
				destination = session->createQueue( getDestination() );
				//creating the consumer
				consumer = session->createConsumer( destination, getSelector() );
			}

			// where Im going to catch messages
			//consumer->setMessageListener( this );
			//defining this to catch exceptions on transport layer
			connection->setExceptionListener(this);

			//Connection is running
			setState(CONNECTION_RUNNING);

			//starting the consumer thread
			activeConsumerThread.runThread();

			if (getRequestReply()){
				//starting the producer thread
				activeThread.runSendThread();
			}

			logMessage.str("");
			logMessage << "ActiveConsumer::run. Consumer is started succesfully: "<< getId();
			LOG4CXX_DEBUG(logger, logMessage.str().c_str());
		}
	} catch (ActiveException& e){
		//up exception
		setState(CONNECTION_NOT_INITIATED);
		throw e;
	} catch (CMSException& e) {
		//throw exception about this
		setState(CONNECTION_NOT_INITIATED);
		logMessage << "Consumer::run CMSException. Check connection parameters. Broker is up?"<<e.what();
		throw ActiveException(logMessage.str());
	}
}

int ActiveConsumer::onReceive(){

	int sizePacket=-1;

	std::stringstream logMessage;

	std::vector<unsigned char> packetDesc(MAX_PARAMETERS);

	try{
		//loop to get messages
		while (!getEndConsumerThread()){

			ActiveMessage activeMessage;
			activeMessage.setConnectionId(getId());
			std::auto_ptr<Message> message( consumer->receive() );

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

				}else if (message->getCMSType()=="Advisory"){
					//Advisory messages
					handleAdvisoryMessages(message, activeMessage);

				}else{
					/////////////////////////////////////////////////////////////
					// text message
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

void ActiveConsumer::handleAdvisoryMessages(std::auto_ptr<cms::Message> message,
											ActiveMessage& activeMessage)
	throw (ActiveException){

	std::stringstream logMessage;

	try{

//        const activemq::commands::ActiveMQMessage* amqMessage=(activemq::commands::ActiveMQMessage*)message.get();
//        if( amqMessage != NULL && amqMessage->getDataStructure() != NULL ) {
//        	if (amqMessage->)
//        	const activemq::commands::ConnectionInfo* connInfo=
//        			dynamic_cast<const activemq::commands::ConnectionInfo*>(amqMessage->getDataStructure().get());
//        	const activemq::commands::RemoveInfo* disconnInfo=
//        			dynamic_cast<const activemq::commands::RemoveInfo*>(amqMessage->getDataStructure().get());
//        }
//
//		std::vector<std::string> propertyNames=message->getPropertyNames();
//		std::vector<std::string>::iterator iteratorProperties;
//		for (iteratorProperties=propertyNames.begin(); iteratorProperties!=propertyNames.end();iteratorProperties++){
//			std::cout << *iteratorProperties << std::endl;
//		}

	}catch(...){
		logMessage << "Consumer::handleAdvisoryMessages. Unknown exception";
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
	}
}


void ActiveConsumer::loadProperties(TextMessage* textMessage, ActiveMessage& activeMessage)
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

int ActiveConsumer::deliver (ActiveMessage& activeMessageR)	throw (ActiveException){

	std::stringstream logMessage;
	try{

		//if connection is running accepting messages into the queue
		if (getState()==CONNECTION_CLOSED){
			logMessage << "ERROR: Consumer connection "<< getId() << " was closed.";
			LOG4CXX_ERROR(logger, logMessage.str().c_str());
			return -1;
		}

		//setting the connection id to the message to be marked
		activeMessageR.setConnectionId(getId());

		//enqueue the message
		int position=activeQueue.enqueue(activeMessageR);
		//checking if is enqueded or not
		if (position==-1){
			//preparing to make the callback
			ActiveCallbackObject activeCallbackObject(	ON_PACKET_DROPPED,
														getId(),
														activeMessageR);
			activeCallbackQueue.enqueue(activeCallbackObject);
			activeCallbackThread.newCallback(true);

			activeQueue.setWorkingState(false);

			logMessage << "POSSIBLE DATA LOSS. Message could not be inserted in the queue ";
			LOG4CXX_ERROR(logger, logMessage.str().c_str());
		}else{
			activeThread.newMessage(true);
			logMessage << "New response sent.";
			LOG4CXX_DEBUG(logger, logMessage.str().c_str());
		}
		return position;

	}catch(ActiveException e){
		logMessage << "POSIBLE LOSS OF DATA. Consumer could not enqueue response message "<< e.getMessage();
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		throw ActiveException (logMessage.str());
	}
	return -1;
}

int ActiveConsumer::send(){

	std::stringstream currentTime;
	std::stringstream logMessage;

	try{
		if (getState()==CONNECTION_CLOSED){
			activeThread.newMessage(false);
			logMessage << "Consumer::send. POSSIBLE DATA LOSS. Producer " << getId() << " is closed. Send is not possible. Persistence is On?";
			LOG4CXX_ERROR(logger, logMessage.str().c_str());
			return -1;
		}

		ActiveMessage activeMessageToSend;
		activeQueue.dequeue(activeMessageToSend);
		activeThread.newMessage(false);

		if (replyProducer != NULL || connection != NULL || session != NULL || destination != NULL){

			if (activeMessageToSend.isTextMessage()){
				//sending a textMessage
				TextMessage* textMessage=session->createTextMessage(activeMessageToSend.getText());

				insertUserPropertiesText(textMessage,activeMessageToSend);

				//Set the correlation ID from the received message to be the correlation id of the response message
				//this lets the client identify which message this is a response to if it has more than
				//one outstanding message to the server
				textMessage->setCMSCorrelationID(activeMessageToSend.getCorrelationId());

				//sending message
				if (activeMessageToSend.getDestination().getReplyTo()){
					replyProducer->send(activeMessageToSend.getDestination().getReplyTo(),textMessage);
					isQueueReadyAgain(activeMessageToSend);
				}else{
					throw ActiveException ("ERROR: Unknown request reply destination.");
				}

				logMessage << "Text message sent from connection "<< getId() << " to queue " << getDestination();
				LOG4CXX_DEBUG(logger, logMessage.str().c_str());

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

				//Set the correlation ID from the received message to be the correlation id of the response message
				//this lets the client identify which message this is a response to if it has more than
				//one outstanding message to the server
				streamMessage->setCMSCorrelationID(activeMessageToSend.getCorrelationId());

				//sending message
				if (activeMessageToSend.getDestination().getReplyTo()){
					replyProducer->send(activeMessageToSend.getDestination().getReplyTo(),streamMessage);
					isQueueReadyAgain(activeMessageToSend);
				}else{
					throw ActiveException ("ERROR: Unknown request reply destination.");
				}

				logMessage << "ActiveMessage sent from connection "<< getId() << " to queue " << getDestination();
				LOG4CXX_DEBUG(logger, logMessage.str().c_str());

				//deleting memory for message
				delete streamMessage;
			}
			return 0;

		}else{
			logMessage << "Producer::send producer is not initialized.";
			LOG4CXX_ERROR(logger, logMessage.str().c_str());
			return -1;
		}
		return 1;
	}catch ( ActiveException& ae ){
		logMessage << "Producer::sendData CMSException: " << ae.getMessage();
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		return -1;
	}catch (CMSException& cmse){
		logMessage.str("Consumer received a CMSException. We are going to close it. Reason:");
		logMessage << cmse.what() << getId();
		LOG4CXX_FATAL (logger,logMessage.str().c_str())
		return -1;
	}
}

void ActiveConsumer::insertParameters(StreamMessage* streamMessage, ActiveMessage& activeMessage)
	throw (ActiveException){

	std::string key;
	try{
		//inserting packet description
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

void ActiveConsumer::insertUserProperties(StreamMessage* streamMessage, ActiveMessage& activeMessage)
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

void ActiveConsumer::insertUserPropertiesText(TextMessage* textMessage, ActiveMessage& activeMessage)
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

void ActiveConsumer::onException( const CMSException& ex ) {

	std::stringstream logMessage;
	try{
		logMessage << "Producer::onException. Exception ocurred" << ex.what() << getClientId();
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());

		//making the object
		ActiveCallbackObject activeCallbackObject(	ON_EXCEPTION,
													getId(),
													ex.getMessage());
		activeCallbackQueue.enqueue(activeCallbackObject);
		activeCallbackThread.newCallback(true);

	}catch (ActiveException& ae){
		logMessage << "Producer::onException. Exception ocurred" << ae.getMessage() << getClientId();
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
	}
}

void ActiveConsumer::transportInterrupted() {
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

void ActiveConsumer::transportResumed() {
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

void ActiveConsumer::isQueueReadyAgain(ActiveMessage& activeMessageR){
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

void ActiveConsumer::start(){
	try{
		//starting all again
		startConsumerThread();
		//run all
		run();
	}catch (ActiveException& ae){
		throw ae;
	}
}

void ActiveConsumer::stop(){
	try{
		std::stringstream logMessage;
		logMessage << "Consumer::stop. Stopping consumer " << getId()<< " connected to " << getIpBroker() << " " << getDestination()<< "...";
		LOG4CXX_INFO(logger, logMessage.str().c_str());
		//setting state to close
		setState(CONNECTION_NOT_INITIATED);
		//closing the consumer thread
		endConsumerThread();
		//ending the producer thread
		activeThread.stop();
		//cleaning up
		cleanup();
		logMessage.str("");
		logMessage << "Consumer::stop. Consumer " << getId()<< " connected to " << getIpBroker() << " " << getDestination() << " stopped succesfully!";
		LOG4CXX_INFO(logger, logMessage.str().c_str());
	}catch (ActiveException& ae){
		throw ae;
	}
}

void ActiveConsumer::cleanup(){

	//*************************************************
	// Always close destination, consumers and producers before
	// you destroy their sessions and connection.
	//*************************************************
	std::stringstream message;

	try{

		// Destroy resources.
		try{
			message.str("");
			message << "Consumer::deleting destination...";
			LOG4CXX_INFO(logger, message.str().c_str());
			if( destination != NULL ) delete destination;
		}catch (CMSException& e) {
			message << "Consumer::cleanup. error removing destination .";
			LOG4CXX_INFO(logger, message.str().c_str());

		}
		destination = NULL;

		try{
			message.str("");
			message << "Consumer::deleting reply producer... ";
			LOG4CXX_INFO(logger, message.str().c_str());
			if( replyProducer != NULL ) delete replyProducer;
		}catch (CMSException& e) {
				message << "Consumer::cleanup. error removing reply producer object.";
				LOG4CXX_INFO(logger, message.str().c_str());
		}
		replyProducer=NULL;

		// Close open resources.
		try{
			message.str("");
			message << "Consumer::closing session... ";
			LOG4CXX_INFO(logger, message.str().c_str());
			if( session != NULL ) session->close();
			message.str("");
			message << "Consumer::closing connection... ";
			LOG4CXX_INFO(logger, message.str().c_str());
			if( connection != NULL ) connection->close();
		}catch (CMSException& e) {
			message << "Consumer::cleanup. error closing session.";
			LOG4CXX_INFO(logger, message.str().c_str());
		}

		// Now Destroy them
		try{
			message.str("");
			message << "Consumer::deleting session... ";
			LOG4CXX_INFO(logger, message.str().c_str());
			if( session != NULL ) delete session;
		}catch (CMSException& e) {
			message << "Consumer::cleanup. error removing session .";
			LOG4CXX_INFO(logger, message.str().c_str());
		}
		session = NULL;

		try{
			message.str("");
			message << "Consumer::deleting connection... ";
			LOG4CXX_INFO(logger, message.str().c_str());
			if( connection != NULL ) delete connection;
		}catch (CMSException& e) {
			message << "Consumer::cleanup. error removing connection .";
			LOG4CXX_INFO(logger, message.str().c_str());
		}
		connection = NULL;

		try{
			message.str("");
			message << "Consumer::deleting consumer... ";
			LOG4CXX_INFO(logger, message.str().c_str());
			if( consumer != NULL ) delete consumer;
		}catch (CMSException& e) {
			message << "Consumer::cleanup. error removing consumer object.";
			LOG4CXX_INFO(logger, message.str().c_str());
		}

		consumer = NULL;

		message.str("");
	}catch(...){
		message << "Consumer::closed. Consumer " << getId()<< " connected to " << getIpBroker() << " " << getDestination() << " closed with Exception!";
		LOG4CXX_INFO(logger, message.str().c_str());
	}

}

void ActiveConsumer::close() {
	//closing consumer
	std::stringstream logMessage;
	logMessage << "Consumer::close. Closing consumer " << getId()<< " connected to " << getIpBroker() << " " << getDestination()<< "...";
	LOG4CXX_INFO(logger, logMessage.str().c_str());

	//setting state to close
	setState(CONNECTION_CLOSED);
	//deletint the references to this connection
	ActiveManager::getInstance()->removeLinkBindingTo(getId());
	//closing the consumer thread
	endConsumerThread();
	//ending the producer thread
	activeThread.stop();
	//ending the callback thread
	activeCallbackThread.stop();
	//cleaning up
	cleanup();

	logMessage.str("");
	logMessage << "Consumer::close. Consumer " << getId()<< " connected to " << getIpBroker() << " " << getDestination() << " closed succesfully!";
	LOG4CXX_INFO(logger, logMessage.str().c_str());
}

ActiveConsumer::~ActiveConsumer(){
	close();
}
