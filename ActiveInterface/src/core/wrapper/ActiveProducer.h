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
 * Class that uses activemq-cpp. Is the real class that implements the producer
 * to the broker. Is a class that inherits from ActiveConnection and that
 * implements all his virtual pure methods like deliver, or send.
 * This class also implements methods of consumer for request reply uses.
 */

#ifndef ACTIVEPRODUCER_H_
#define ACTIVEPRODUCER_H_

#include "../queue/ActiveQueue.h"
#include "ActiveProducerThread.h"
#include "../callbacks/ActiveCallbackThread.h"
#include "../persistence/ActivePersistence.h"
#include "ActiveConsumerThread.h"

#include <cms/Session.h>
#include <cms/Connection.h>
#include <cms/MessageListener.h>
#include <cms/ExceptionListener.h>
#include <decaf/lang/Runnable.h>
#include <activemq/transport/DefaultTransportListener.h>

#include "log4cxx/logger.h"
#include "log4cxx/helpers/exception.h"

using namespace ai::message;
using namespace activemq;
using namespace activemq::transport;
using namespace cms;

namespace ai{

	class ActiveProducer :	public ExceptionListener,
							public DefaultTransportListener,
							public ActiveConnection{

	private:

		/**
		 * static var for logger
		 */
		static log4cxx::LoggerPtr logger;

		/**
		 * Pointer to messages producer
		 */
		MessageProducer* producer;

		/**
		 * Pointer to destination for temporary queue
		 */
		Destination* tempDest;

		/**
		 * Pointer to messages consumer
		 */
		MessageConsumer* responseConsumer;

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
		 * Class that manages the thread for  making callbacks
		 */
		ActiveCallbackThread activeCallbackThread;

		/**
		 * Class that controls the persistence of the messages produced
		 */
		ActivePersistence activePersistence;

		/**
		 * object that makes all work about consuming
		 */
		ActiveConsumerThread activeConsumerThread;

		/**
		 * mutex to control the activation of the recovery thread
		 */
		ActiveMutex activateRecoveryMutex;

		/**
		 * Method that delete all structures for broker connection, close connection
		 * and free resources
		 */
		void cleanup();

		/**
		 * Method that creates temporary queue for response
		 */
		void createTempQueue();

		/**
		 * Virtual method called by callback for each message consumed
		 *
		 * @param message activemq message pointer
		 * @throw ActiveException if something bad happens.
		 */
		//virtual void onMessage( const Message* message ) throw (ActiveException);

		/**
		 * virtual method called when an exception ocurrs
		 *
		 */
		virtual void onException( const CMSException& ex );

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
		 * @throws ActiveException if something bad happens
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
		 * Copy all default properties to each message when is stored in queue
		 *
		 * @param activeMessage message where we are going to set properties
		 * @param activeLink link where we are going to get all properties and sets to message
		 * @param defaultPropertiesAdd List of strings that stores the properties that
		 * are added to the message that comes from the configuration xml file (default properties).
		 *
		 * @throws ActiveException
		 */
		void copyDefaultProperties(	ActiveMessage& activeMessage,
									ActiveLink& activeLink,
									std::list<std::string>& defaultPropertiesAdd)
			throw (ActiveException);

		/**
		 * remove all default properties to each message when is stored in queue
		 *
		 * @param activeMessage message where we are going to set properties
		 * @param defaultPropertiesAdd List of strings that stores the properties that were added
		 * to the message from the configuration xml file that are going to be deleted
		 *
		 * @throws ActiveException
		 */
		void removeDefaultProperties(	ActiveMessage& activeMessage,
										std::list<std::string>& defaultPropertiesAdd)
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
		 *
		 */
		void isQueueReadyAgain(ActiveMessage& activeMessageR);

	public:

		/**
		 * Default constructor
		 *
		 * @param id identification for each map
		 * @param brokerURIRcvd Address of the broker. For more documentation see activemq-cpp library
		 * @param destURIRcvd Name of the queue/topic to which messages are going to send/receive
		 * @param maxSizeQueueR Maximun size for the internal queue of this consumer.
		 * @param usernameR Username for connections that requires it
		 * @param passwordR Password for connections that requires it
		 * @param clientIdR name of the client, it will be used for durable connections
		 * @param useTopicRcvd To know if is a connection to topic (true) or queue (false)
		 * @param getResponseRcvd flag to know if is a request reply producer
		 * @param clientAckRcvd Flag to specify if the this library sends ack to each message or not (activemq-cpp will do it automatically).
		 * @param deliveryModeRcvd Flag to specify if the message is persistent or not
		 * @param persistentR Is the flat that enable persistence for the library. Number indicates the
		 * number of messages that will be stored before delete it (if it is possible).
		 * @param certificate path to the certificate pem. If you use SSL you have to provide it.
		 */
		ActiveProducer(	std::string& id,
						std::string& brokerURIRcvd,
						std::string& destURIRcvd,
						int maxSizeQueueR,
						std::string& usernameR,
						std::string& passwordR,
						std::string& clientIdR,
						std::string& certificate,
						bool useTopicRcvd=false,
						bool getResponseRcvd=false,
						bool clientAckRcvd=false,
						bool deliveryModeRcvd=false,
						int persistentR=0);

		/**
		 * Method that is going to start the consumer
		 *
		 * @throw ActiveException if something bad happens.
		 */
		virtual void run() throw (ActiveException);

		/**
		 * Method used by the thread to send messages
		 */
		int send();

		/**
		 * Method that is going to insert the message into the internal queue
		 *
		 * @param activeMessageR message that is going to be inserted into queue.
		 * @param activeLink reference to link to copy the default parameters associated to this link
		 *
		 * @throws ActiveException
		 *
		 * @return position that this message has in the queue.
		 */
		int deliver(ActiveMessage& activeMessageR, ActiveLink& activeLink) throw (ActiveException);

		/**
		 * Method that is going to insert the message into the internal queue
		 *
		 * @param activeMessageR message that is going to be inserted into queue.
		 * @return position that this message has in the queue.
		 */
		int deliver (ActiveMessage& activeMessageR) throw (ActiveException);

		/**
		 * Method that initializes some things that connections needs
		 */
		void init ();

		/**
		 * Method to know if persistence is enabled
		 */
		bool isInRecoveryMode(){return activePersistence.getRecoveryMode();}

		/**
		 * method to stop the current connection
		 */
		void start();

		/**
		 * method to stop the current producer
		 */
		void stop();

		/**
		 * Close connections, free resources
		 */
		void close();

		/**
		 * Default destructor
		 */
		virtual ~ActiveProducer();

	};
}

#endif /* ACTIVEPRODUCER_H_ */
