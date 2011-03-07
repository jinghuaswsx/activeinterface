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
 * Class that implements interface with application. Is the entry point to the library, and
 * gives users all methods to interact with the library and its components.
 *
 */

#ifndef ACTIVEINTERFACE_H_
#define ACTIVEINTERFACE_H_

#ifdef ACTIVEINTERFACE_DLL
 #ifdef ACTIVEINTERFACE_EXPORTS
  #define ACTIVEINTERFACE_API __declspec( dllexport )
 #else
  #define ACTIVEINTERFACE_API __declspec( dllimport )
 #endif
#else 
 #define ACTIVEINTERFACE_API
#endif

#include <list>

#include "core/ActiveLink.h"
#include "core/ActiveConnection.h"
#include "core/message/ActiveMessage.h"
#include "utils/exception/ActiveException.h"
#include "core/concurrent/ReadersWriters.h"

namespace ai{

	class ACTIVEINTERFACE_API ActiveInterface {

	public:

		/**
		 * Method that startup the library and creates all needed structures. It should be called
		 * before using it.
		 *
		 * @param configurationFile FileName for the configuration file. By default will be
		 * ActiveConfiguration.xml
		 *
		 * @throws ActiveException with description
		 */
		void startup(const std::string& configurationFile="ActiveConfiguration.xml")
			throw ( ActiveException );

		/**
		 * Method that send active message to a specific service id
		 *
		 * @param serviceId service id to which we are going to send the message
		 * @param activeMessage Message that the user have filled in his implementation.
		 *
		 * @throws ActiveException if something happens
		 */
		void send(	std::string& serviceId,
					ActiveMessage& activeMessage) throw (ActiveException);

		/**
		 * Method that send active message to a specific service id, but returning the position
		 * in which this message is placed.
		 *
		 * @param serviceId service id to which we are going to send the message
		 * @param activeMessage Message that the user have filled in his implementation.
		 * @param positionInQueue reference to a list that method will fill with the queue position for each
		 * connection that the message is sent.
		 *
		 * @throws ActiveException if something happens
		 */
		void send(	std::string& serviceId,
					ActiveMessage& activeMessage,
					std::list<int>& positionInQueue) throw (ActiveException);

		/**
		 * Method used to send replys to a specific connection id (not a service)
		 * this connection needs to be of types 2 o 3 (Producer with request reply or Consumer RR).
		 *
		 * @param connectionId is the connection identificator through im going to respond
		 * @param activeMessage message that the user filled up to respond to a message received.
		 *
		 * @throws ActiveException if something happens
		 */
		void sendResponse (	std::string& connectionId,
							ActiveMessage& activeMessage) throw (ActiveException);


		/**
		 * Method that creates a new JMS Producer
		 *
		 * @param id identification for each map
		 * @param ipBroker Address of the broker. For more documentation see activemq-cpp library
		 * @param destination Name of the queue/topic to which messages are going to send/receive
		 * @param requestReply Boolean var to identify if the producer that is going to be created is
		 * with response.
		 * @param topic To know if is a connection to topic (true) or queue (false)
		 * @param persistent flag to set if message is persistent(0) or not persistent (1)
		 * @param clientAck Flag to specify if the this library sends ack to each message or not (activemq-cpp will do it automatically).
		 * @param maxSizeQueue Max value for messages that will be stored in the intern queue.
		 * @param username Username that is going to be specified to stablish the connection
		 * @param password Password for stablish the connection.
		 * @param clientId String that sets the name of the client. Its should be defined if is a consumer of a topic.
		 * @param persistence Is a number that specifies the number of messages that will be stored in the persistence_file before
		 * delete it. This messages will be deleted when all messages were sent and messages wrotes and sent are equal.
		 * @param certificate Path to the certificate PEM. If SSL is going to be used.
		 *
		 * @throws ActiveException if something bad happens
		 */
		void newProducer (	std::string& id,
							std::string& ipBroker,
							std::string& destination,
							bool requestReply=false,
							bool topic=false,
							bool persistent=false,
							bool clientAck=false,
							int maxSizeQueue=0,
							const std::string& username="",
							const std::string& password="",
							const std::string& clientId="",
							long persistence=0,
							const std::string& certificate="") throw (ActiveException);

		/**
		 * Method that creates a new JMS Consumer
		 *
		 * @param id identification for each map
		 * @param ipBroker Address of the broker. For more documentation see activemq-cpp library
		 * @param destination Name of the queue/topic to which messages are going to send/receive
		 * @param requestReply Boolean var to identify if the producer that is going to be created is
		 * with response.
		 * @param topic To know if is a connection to topic (true) or queue (false)
		 * @param durable flag to set if the consumer is durable or not. see JMS documentation.
		 * @param clientAck Flag to specify if the this library sends ack to each message or not (activemq-cpp will do it automatically).
		 * @param selector Is a regular expression used to accept or not a determinate message that matchs with the expression.
		 * @param username Username that is going to be specified to stablish the connection
		 * @param password Password for stablish the connection.
		 * @param clientId String that sets the name of the client. Its should be defined if is a consumer of a topic.
		 * @param certificate Path to certificate PEM. If SSL is used.
		 *
		 * @throws ActiveException if something bad happens
		 */
		void newConsumer (	std::string& id,
							std::string& ipBroker,
							std::string& destination,
							bool requestReply=false,
							bool topic=false,
							bool durable=false,
							bool clientAck=false,
							const std::string& selector="",
							const std::string& username="",
							const std::string& password="",
							const std::string& clientId="",
							const std::string& certificate="") throw (ActiveException);

		/**
		 *  Method that creates a new link with its properties.
		 *
		 *  @param serviceId Service in which the link is going to be added
		 *  @param linkId Identifier of link
		 *  @param name Name of the link
		 *  @param connectionId Identifier of the connection to which the link is going
		 *  to be linked
		 *  @param parameterList List of default properties for this new link.
		 *
		 * @throws ActiveException if something bad happens
		 */
		void newLink(	std::string& serviceId,
						std::string& linkId,
						std::string& name,
						std::string& connectionId,
						ai::utils::ParameterList& parameterList) throw (ActiveException);

		/**
		 * Method that allows the user to add a existing link to a service point
		 *
		 * @param serviceId Integer to identify the serviceId in which the link is going to be set.
		 * @param linkId Identifier link that is going to be added to the service
		 *
		 * @throws ActiveException if something bad happens
		 */
		void addLink (std::string& serviceId, std::string& linkId) throw (ActiveException);

		/**
		 * Method that returns the list of links that are associated with a connectionId
		 *
		 * @param connectionId Identifier of the connection.
		 * @param linkList List that is going to be filled up by all links that are
		 * associated with this connectionId
		 *
		 * @throws ActiveException if something bad happens
		 */
		void getLinksByConn( std::string& connectionId, std::list<ActiveLink*>& linkList)
			throw (ActiveException);

		/**
		 *	Method that returns a list with all links associated with a service Id
		 *
		 * @param serviceId Identifier of service
		 * @param linkList List that is going to be filled up by all links that are
		 * associated with this serviceId
		 *
		 * @throws ActiveException if something bad happens
		 */
		void getLinksByService(	std::string& serviceId, std::list<ActiveLink*>& linkList)
			throw (ActiveException);

		/**
		 * Method that returns a list of integers that identifies the services that
		 * sends data through this link
		 *
		 * @param linkId Link identifier that is going to be used to search along services
		 * to know which services send through this linkId
		 * @param linkList List that is going to be filled up by all services ids that are
		 * associated with this linkId
		 *
		 * @throws ActiveException if something bad happens
		 */
		void getServicesByLink(	std::string& linkId, std::list<std::string>& linkList)
			throw (ActiveException);

		/**
		 * Method that returns a Connection to a broker found by the linkId associated
		 * with this connection.
		 *
		 * @param linkId that is associated with the connection.
		 *
		 * @return Connection associated with this linkId
		 *
		 * @throws ActiveException if something bad happens
		 */
		const ActiveConnection* getConnByLink(std::string& linkId)
			throw (ActiveException);

		/**
		 * Method used to return the list of connections associated with a serviceId.
		 *
		 * @param serviceId Identifier of the service id
		 * @param connectionListR list that is going to be filled up by the method
		 *
		 * @throws ActiveException if something bad happens
		 */
		void getConnsByService (std::string& serviceId,std::list<ActiveConnection*>& connectionListR)
			throw (ActiveException);

		/**
		 * Method used to return a list of connections that has a specific destination received
		 * by parameter.
		 *
		 * @param destination string that specifies the name of the queue/topic that we are looking for.
		 * @param connectionListR list of connections in which the method stores the connections that has
		 * the destination specificied by destination param.
		 *
		 * @throws ActiveException if something bad happens
		 */
		void getConnsByDestination(	std::string& destination,
									std::list<ActiveConnection*>& connectionListR)
			throw (ActiveException);

		/**
		 * Method that returns all services that are using in this time
		 *
		 * @param serviceList reference array in which method is going to write all
		 * serviceList
		 *
		 * @throws ActiveException if something bad happens
		 */
		void getServices(std::list<std::string>& servicesList) throw (ActiveException);

		/**
		 * This method is going to be used to destroy a connection specified by the identifier of this
		 * connection. This method deletes the connection, link associated with this connection and
		 * all references to this links in services configuration.
		 *
		 * @param connectionId identifier of the connection that is going to be destroy.
		 *
		 * @return true if the connection was destroyed or false if not.
		 *
		 * @throws ActiveException if something bad happens
		 */
		bool destroyConnection(std::string& connectionId) throw (ActiveException);

		/**
		 * This method is used to destroy a specific link using its link identifier.
		 *
		 * @param linkId Link identifier that is going to be destroy.
		 *
		 * @return true if link was destroyed succesfully, else false.
		 *
		 * @throws ActiveException if something bad happens
		 */
		bool destroyLink(std::string& linkId)  throw (ActiveException);

		/**
		 *	This method is used to destroy a specific service
		 *
		 * @param serviceId Identifier of the service that is going to be destroy.
		 *
		 * @return true if link was destroyed succesfully, else false.
		 *
		 * @throws ActiveException if something bad happens
		 */
		bool destroyService(std::string& serviceId) throw (ActiveException);

		/**
		 *	Method that destroy a link specified in services block. If we have one service with
		 *	two links specified to send through it, use this method to delete one of it.
		 *
		 *	@param linkId Identifier of the link that is going to be destroy.
		 *	@param serviceId Identifier of the service that use this link-
		 *
		 * @return true if links were destroyed succesfully, else false.
		 *
		 * @throws ActiveException if something bad happens
		 */
		bool destroyServiceLink (std::string& linkId, std::string& serviceId) throw (ActiveException);

		/**
		 * Method that destroy the binding between a connection and a link
		 *
		 * @param linkId link that is going to be destroyed the connection to.
		 *
		 * @return true if binding was destroyed succesfully, else false.
		 *
		 * @throws ActiveException if something bad happens
		 */
		bool destroyLinkConnection (std::string& linkId) throw (ActiveException);

		/**
		 * Method that set a connection between link and connection
		 *
		 * @param linkId link that is going to be linked to a connection
		 * @param connectionId connection that is going to be linked by a link
		 *
		 * @return true if binding was created succesfully, else false.
		 *
		 * @throws ActiveException if something bad happens
		 */
		bool setLinkConnection(std::string& linkId, std::string& connectionId) throw (ActiveException);

		/**
		 * Callback that the library will invoke when messages are received for his associated
		 * consumers ids.
		 *
		 * @param activeMessage is the message that the library sends to the user
		 */
		virtual void onMessage(const ActiveMessage& activeMessage) abstract;

		/**
		 * Callback that the library will invoke when connection with one of his associated
		 * brokers is interrupted.
		 *
		 * @param connectionId is the connection id for the connection that is down
		 */
		virtual void onConnectionInterrupted (std::string& connectionId) abstract;

		/**
		 * Callback that the library will invoke when connection with one of his associated
		 * brokers is up again.
		 *
		 * @param connectionId is the connection id for the connection that is up again
		 */
		virtual void onConnectionRestore(std::string& connectionId) abstract;

		/**
		 * Callback that the library will invoke when the library intern queue is full.
		 * From the moment that the library invoke this method, message wont be stored
		 * to send later
		 *
		 * @param activeMessage is the message that the library deny
		 */
		virtual void onQueuePacketDropped(const ActiveMessage& activeMessage)abstract;

		/**
		 * Callback that the library will invoke when the library intern queue is not full again.
		 * From the moment that the library invoke this method, message wont be stored
		 * to send later
		 *
		 * @param connectionId is the connection that drops the message
		 */
		virtual void onQueuePacketReady(std::string& connectionId)abstract;

		/**
		 * Callback that the library calls when some exception ocurres in
		 * producer or consumer
		 *
		 * @param connectionId is the connection that drops the message
		 */
		virtual void onException(std::string& connectionId)abstract;

		/**
		 * Method that shutdown all the library, close all connections and free all resources
		 * used by the library
		 *
		 * @throws ActiveException if something bad happens
		 */
		bool shutdown() throw (ActiveException);

		/**
		 * Constructor is empty. To start the library use startup() method
		 */
		ActiveInterface(){}

		/**
		 * Virtual destructor of the class
		 */
		virtual ~ActiveInterface(){}

	private:
		/**
		 * Static var use by log4cxx for the logging system
		 */
		static log4cxx::LoggerPtr logger;

		/**
		 * state of the library
		 */
		int state;

		/**
		 * getter of the state of the library
		 */
		int getState(){ return state;}

		/**
		 * setter of the state of the library
		 */
		void setState(int stateR) { state=stateR;}

		/**
		 * Class that implements the reader & writer thread safe method to
		 * access to a code block
		 */
		ReadersWriters readersWriters;
	};
}

#endif /* ACTIVEINTERFACE_H_ */
