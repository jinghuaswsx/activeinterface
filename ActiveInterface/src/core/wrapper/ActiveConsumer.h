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
 * Class that uses activemq-cpp. Is the real class that implements the consumer
 * to the broker. Is a class that inherits from ActiveConnection and that
 * implements all his virtual pure methods like onMessage.
 * This class also implements methods of producer for request reply uses.
 */

#ifndef ACTIVECONSUMER_H_
#define ACTIVECONSUMER_H_

#include "../queue/ActiveQueue.h"
#include "ActiveProducerThread.h"
#include "../callbacks/ActiveCallbackThread.h"
#include "../persistence/ActivePersistence.h"
#include "ActiveConsumerThread.h"

#include <cms/Session.h>
#include <cms/Connection.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>
#include <decaf/lang/Runnable.h>
#include <activemq/transport/DefaultTransportListener.h>

#include "log4cxx/logger.h"
#include "log4cxx/helpers/exception.h"

using namespace ai::message;
using namespace activemq;
using namespace activemq::transport;
using namespace cms;

namespace ai{

	class ActiveConsumer :	public ExceptionListener,
							public DefaultTransportListener,
							public ActiveConnection{
	private:

		/**
		 * static var for logger
		 */
		static log4cxx::LoggerPtr logger;

		/**
		 * Pointer to messages consumer
		 */
		MessageConsumer* consumer;

		/**
		 * Pointer to destination for temporary queue
		 */
		MessageProducer* replyProducer;

		/**
		 * connection pointer
		 */
		cms::Connection* connection;

		/**
		 * Reference to session with broker
		 */
		cms::Session* session;

		/**
		 * Pointer to destination
		 */
		cms::Destination* destination;

		/**
		 * Object that stores the queue of messages
		 */
		ActiveQueue activeQueue;

		/**
		 * Object that stores the logic thread send
		 */
		ActiveProducerThread activeThread;

		/**
		 * Object that stores the queue of callbacks
		 */
		ActiveCallbackQueue activeCallbackQueue;

		/**
		 * Object that makes all work about the thread that sends
		 * callbacks to user part
		 */
		ActiveCallbackThread activeCallbackThread;

		/**
		 * object that makes all work about consuming
		 */
		ActiveConsumerThread activeConsumerThread;

		/**
		 * Method that delete all structures for broker connection, close connection
		 * and free resources
		 */
		void cleanup();

		/**
		 * Virtual method called by callback for each message consumed
		 *
		 * @throws ActiveException
		 *
		 * @param message activemq message pointer
		 */
		//virtual void onMessage( const Message* message ) throw (ActiveException);

		/**
		 * virtual method called when an exception ocurrs
		 */
		virtual void onException( const CMSException& ex);

		/**
		 * Virtual method called when the connection with broker is interrupted
		 */
		virtual void transportInterrupted();

		/**
		 * Virtual mehotd called when the connection with broker is up again
		 */
		virtual void transportResumed();

		/**
		 * virtual method called by the consuming thread to receive asynchronously
		 */
		int onReceive();

		/**
		 * Method that insert parameter list of activemessage into jms stream message
		 *
		 * @param streamMessage JMS StreamMessage. See JMS API.
		 * @param activeMessage message that is going to be extracted to JMS message
		 *
		 * @throw ActiveException if something bad happens.
		 */
		void insertParameters(StreamMessage* streamMessage,ActiveMessage& activeMessage)
			throw (ActiveException);

		/**
		 * Method that insert properties into stream message that user inserted in activeMessage
		 *
		 * @param streamMessage JMS StreamMessage. See JMS API.
		 * @param activeMessage message that its properties are going to be extracted to JMS message
		 *
		 * @throw ActiveException if something bad happens.
		 */
		void insertUserProperties(StreamMessage* streamMessage,ActiveMessage& activeMessage)
			throw (ActiveException);

		/**
		 * Method that insert properties into text message that user inserted in activeMessage
		 *
		 * @param streamMessage JMS TextMessage. See JMS API.
		 * @param activeMessage message that its properties are going to be extracted to JMS message
		 *
		 * @throws ActiveException if something bad happens
		 */
		void insertUserPropertiesText(TextMessage* textMessage,ActiveMessage& activeMessage)
			throw (ActiveException);

		/**
		 * Load all properties from text message into activeMessage
		 *
		 * @param textMessage
		 * @param activeMessage
		 *
		 * @throws ActiveException
		 */
		void loadProperties(TextMessage* textMessage, ActiveMessage& activeMessage)
			throw (ActiveException);

		/**
		 * Method that throws a callback with the last message that was stored
		 * in the queue. This callback only will be throwed when the queue was full before.
		 *
		 * @param activeMessageR message that is in the queue.
		 */
		void isQueueReadyAgain(ActiveMessage& activeMessageR);

	public:

		/**
		 * Default constructor
		 *
		 * @param id identification for each map
		 * @param brokerURIRcvd Address of the broker. For more documentation see activemq-cpp library
		 * @param destURIRcvd Name of the queue/topic to which messages are going to send/receive
		 * @param messageSelectorRcvd is the string selector. For more documentation see activemq-cpp library
		 * @param maxSizeQueueR Maximun size for the internal queue of this consumer.
		 * @param usernameR Username for connections that requires it
		 * @param passwordR Password for connections that requires it
		 * @param clientIdR Name for the client,it will be used for durable connections
		 * @param useTopicRcvd To know if is a connection to topic (true) or queue (false)
		 * @param clientAckRcvd Flag to specify if the this library sends ack to each message or not (activemq-cpp will do it automatically).
		 * @param responseToProducerRcvd Flag to specify if the consumer has request reply
		 * @param durable Flag to specify if is a durable connection to broker or not. Only applies to consumers.
		 * @param certificate Path to the pem certificate, if you want to use SSL.
		 */
		ActiveConsumer(	std::string& id,
						std::string& brokerURIRcvd,
						std::string& destURIRcvd,
						std::string& messageSelectorRcvd,
						int maxSizeQueueR,
						std::string& usernameR,
						std::string& passwordR,
						std::string& clientIdR,
						std::string& certificate,
						bool useTopicRcvd=false,
						bool clientAckRcvd=false,
						bool responseToProducerRcvd=false,
						bool durable=false);

		/**
		 * Method that is going to start the consumer
		 *
		 * @throw ActiveException if something bad happens.
		 */
		void run()	throw (ActiveException);

		/**
		 * Method used by the thread to send messages
		 *
		 * @return value of return of send, that is the position in the queue.
		 * if is -1 means that the message could not be inserted in queue.
		 */
		int send();

		/**
		 * Method that is going to insert the message into the internal queue
		 *
		 * @param activeMessageR message that is going to be inserted into queue.
		 *
		 * @throw ActiveException if something bad happens.
		 *
		 * @return value of return of send, that is the position in the queue.
		 * if is -1 means that the message could not be inserted in queue.
		 */
		int deliver(ActiveMessage& activeMessageR)	throw (ActiveException);

		/**
		 * Method that is going to insert the message into the internal queue
		 *
		 * @param activeMessageR message that is going to be inserted into queue.
		 * @param activeLink reference to link to copy the default parameters associated to this link
		 *
		 * @throw ActiveException if something bad happens.
		 *
		 * @return position that this message has in the queue.
		 */
		int deliver (ActiveMessage& activeMessageR, ActiveLink& activeLink){return -1;}

		/**
		 * Method that initializes some things that connections needs
		 */
		void init ();

		/**
		 * Method to know if persistence is enabled, for consumer there is no recovery mode
		 */
		bool isInRecoveryMode(){ return false;}

		/**
		 * method to stop the current connection
		 */
		void start();

		/**
		 * method to stop the current consumer
		 */
		void stop();

		/**
		 * Close connections, free resources
		 */
		void close();

		/**
		 * Default destructor
		 */
		~ActiveConsumer();
	};
}

#endif /* ACTIVECONSUMER_H_ */
