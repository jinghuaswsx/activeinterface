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
 * Class that is the core of the library, implements a singleton to provides
 * a unique access pointer and stores all data structures needed for the library
 * to store data.
 *
 */

#ifndef ACTIVEMANAGER_H_
#define ACTIVEMANAGER_H_

#include <map>

#include "xml/ActiveXML.h"
#include "../ActiveInterface.h"

#include "log4cxx/logger.h"
#include "log4cxx/helpers/exception.h"

namespace ai{

	class ActiveManager {
	public:
		/**
		 * Method that implement the singleton, all communication between
		 * outside and inside the library is calling this method.
		 *
		 * @return A new or a unique existing reference to itself
		 */
		static ActiveManager* getInstance(){
			if (!instanceFlag){
				mySelf = new ActiveManager();
				instanceFlag=true;
				return mySelf;
			}else{
				return mySelf;
			}
		}

		////////////////////////////////////////////////////////////////////////////////
		// Primitives
		///////////////////////////////////////////////////////////////////////////////

		/**
		 * Method that initializes the library. Initializing all data structures
		 * and starts the services defined by the configuration file.
		 *
		 * @param configurationFile Is the configuration file that wrapper will use to initialize itself
		 * @param activeInterfacePtr is the pointer to make callback to the user interface.
		 *
		 * @throws ActiveException if something bad happens
		 */
		void init(	const std::string& configurationFile,
					ActiveInterface* activeInterfacePtr,
					bool messageSerializedInConsumptionR)
			throw (ActiveException);

		/**
		 * Method that send active message to a specific service id
		 *
		 * @param serviceId service id to which we are going to send the message
		 * @param activeMessage Message that the user have filled in his implementation.
		 *
		 * @throws ActiveException if something bad happens
		 */
		void sendData(	std::string& serviceId,
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
		 * @throws ActiveException if something bad happens
		 */
		void sendData(	std::string& serviceId,
						ActiveMessage& activeMessage,
						std::list<int>& positionInQueue) throw (ActiveException);

		/**
		 * Method used to send replys to a specific connection id (not a service)
		 * this connection needs to be of types 2 o 3 (Producer with request reply or Consumer RR).
		 *
		 * @param connectionId is the connection identificator through im going to respond
		 * @param activeMessage message that the user filled up to respond to a message received.
		 *
		 * @throws ActiveException if something bad happens
		 */
		void sendResponse (	std::string& connectionId,
							ActiveMessage& activeMessage) throw (ActiveException);


		/////////////////////////////////////////////////////////////////////////////////////
		// methods that manages the topology in hot mode
		/////////////////////////////////////////////////////////////////////////////////////

		/**
		 * Method that creates a new connection
		 *
		 * @param id identification for each map
		 * @param ipBroker Address of the broker. For more documentation see activemq-cpp library
		 * @param type type of connection to broker. 0 consumer 1 producer 2 consumer with Request Reply
		 * 3 producer with Request reply
		 * @param topic To know if is a connection to topic (true) or queue (false)
		 * @param destination Name of the queue/topic to which messages are going to send/receive
		 * @param persistent flag to set if message is persistent(0) or not persistent (1)
		 * @param selector is the string selector. For more documentation see activemq-cpp library
		 * @param durable Flag to specify if is a durable connection to broker or not. Only applies to consumers.
		 * @param clientAck Flag to specify if the this library sends ack to each message or not (activemq-cpp will do it automatically).
		 * @param maxSizeQueue Maximun size for the internal queue of this consumer.
		 * @param username Username for connections that requires it
		 * @param password Password for connections that requires it
		 * @param clientId name of the client, it will be used for durable connections
		 * @param persistence Is the flat that enable persistence for the library. Number indicates the
		 * number of messages that will be stored before delete it (if it is possible).
		 * @param certificate Path to certificate used to SSL connection to broker.
		 *
		 * @return true if new connection is created, else false.
		 *
		 * @throws ActiveException is something bad happens
		 */
		ActiveConnection* newConnection (	std::string& id,
											std::string& ipBroker,
											int type,
											std::string& destination,
											bool topic=false,
											bool persistent=false,
											const std::string& selector="",
											bool durable=false,
											bool clientAck=false,
											int maxSizeQueue=0,
											const std::string& username="",
											const std::string& password="",
											const std::string& clientId="",
											int persistence=0,
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
		 *  @return If link is created succesfully true, else false.
		 *
		 *  @throws ActiveException is something bad happens
		 */
		bool newLink(	std::string& serviceId,
						std::string& linkId,
						std::string& name,
						std::string& connectionId,
						ParameterList& parameterList) throw (ActiveException);

		/**
		 * Method to get a connection for the given id
		 *
		 * @param id is the id for the connection in the map
		 *
		 * @return Pointer to connection with this id
		 *
		 * @throws ActiveException is something bad happens
		 */
		ActiveConnection* getConnection (std::string& id) throw (ActiveException);

		/**
		 * Method to get a link for the given id
		 *
		 * @param id is the id for the link in the map
		 *
		 * @return Pointer to link with this id.
		 *
		 * @throws ActiveException is something bad happens
		 */
		ActiveLink* getLink(std::string& id) throw (ActiveException);

		/**
		 * Method that returns the list of links that are associated with a connectionId
		 *
		 * @param connectionId Identifier of the connection.
		 * @param linkList List that is going to be filled up by all links that are
		 * associated with this connectionId
		 *
		 * @throws ActiveException is something bad happens
		 */
		void getLinksByConn(std::string& connectionId, std::list<ActiveLink*>& linkList)
			throw (ActiveException);

		/**
		 *	Method that returns a list with all links associated with a service Id
		 *
		 * @param serviceId Identifier of service
		 * @param linkList List that is going to be filled up by all links that are
		 * associated with this serviceId
		 *
		 * @throws ActiveException is something bad happens
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
		 * @throws ActiveException is something bad happens
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
		 * @throws ActiveException is something bad happens
		 */
		ActiveConnection* getConnByLink(std::string& linkId) throw (ActiveException);

		/**
		 * Method used to return the list of connections associated with a serviceId.
		 *
		 * @param serviceId Identifier of the service id
		 * @param connectionListR list that is going to be filled up by the method
		 *
		 *
		 * @throws ActiveException is something bad happens
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
		 * @throws ActiveException is something bad happens
		 */
		void getConnsByDestination (std::string& destination,
									std::list<ActiveConnection*>& connectionListR)
			throw (ActiveException);

		/**
		 * Method used to return the list of all connections defined
		 *
		 * @param connectionListR list of connections in which the method stores the connections that has
		 * the destination specificied by destination param.
		 *
		 * @throws ActiveException is something bad happens
		 */
		void getConnections (std::list<ActiveConnection*>& connectionListR)
			throw (ActiveException);

		/**
		 * Method that returns a list of services in a std list.
		 *
		 * @param serviceList reference array in which method is going to write all
		 * serviceList
		 *
		 * @throws ActiveException is something bad happens
		 */
		void getServices (std::list<std::string>& servicesList) throw (ActiveException);

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
		bool destroyLink(std::string& linkId) throw (ActiveException);

		/**
		 *	This method is used to destroy a specific service
		 *
		 *	@param serviceId Identifier of the service that is going to be destroy.
		 *
		 *  @return true if link was destroyed succesfully, else false.
		 *
		 *  @throws ActiveException if something bad happens
		 */
		bool destroyService(std::string& serviceId) throw (ActiveException);

		/**
		 * This method sets a null the binding between link and connection. This method is not going
		 * to delete the link.
		 *
		 * @param connectionId identification of the connection to remove link.
		 *
		 * @return true if binding from connection a link was sucessfully deleted.
		 *
		 * @throws ActiveException if something  bad happens
		 */
		bool removeLinkBindingTo (std::string& connectionId) throw (ActiveException);

		/**
		 * Method that deletes a list of links from all services.
		 *
		 * @param linkList List of Links that are going to be deleted from all the services
		 *
		 * @return true if link was destroyed succesfully, else false.
		 *
		 * @throws ActiveException if something bad happens.
		 */
		bool destroyServiceLink (std::list<ActiveLink*>& linkList)  throw (ActiveException);

		/**
		 *	Method that destroy a link specified in services block. If we have one service with
		 *	two links specified to send through it, use this method to delete one of it.
		 *
		 *	@param linkId Identifier of the link that is going to be destroy.
		 *	@param serviceId Identifier of the service that use this link-
		 *
		 *  @return true if links were destroyed succesfully, else false.
		 *
		 *  @throws ActiveException if something bad happens.
		 */
		bool destroyServiceLink (	std::string& linkId,
									std::string& serviceId)  throw (ActiveException);

		/**
		 * Method that destroy the binding between a link and a connection
		 *
		 * @param linkId Link that binding is going to be removed to connection
		 * @return true if binding was destroyed, else false.
		 *
		 * @throws ActiveException if something bad happens
		 */
		bool destroyLinkConnection (std::string& linkId) throw (ActiveException);

		/**
		 *	Method that set the binding between a link and a connection.
		 *
		 *	@param linkId link that we want to bind with a connection
		 *	@param connectionId connection that is going to be bound with a link
		 *
		 *	@return true if binding set was succesfull.
		 *	@throws ActiveException if something bad happens.
		 */
		bool setLinkConnection (std::string& linkId,
								std::string& connectionId) throw (ActiveException);

		/**
		 * Method that insert the reference to activelink in services multimap
		 *
		 * @param serviceId serviceId to insert the connection
		 * @param linkId is the link id that will be associated with connection
		 *
		 * @return true if link was inserted succesfully, else false.
		 */
		bool insertInMMap(	std::string& serviceId,
							std::string& linkId);

		/**
		 * Method that save connection to connections map
		 *
		 * @param id identification for each map
		 * @param ipBroker Address of the broker. For more documentation see activemq-cpp library
		 * @param type type of connection to broker. 0 consumer 1 producer 2 consumer with Request Reply
		 * 3 producer with Request reply
		 * @param topic To know if is a connection to topic (true) or queue (false)
		 * @param destination Name of the queue/topic to which messages are going to send/receive
		 * @param persistent flag to set if message is persistent(0) or not persistent (1)
		 * @param selector is the string selector. For more documentation see activemq-cpp library
		 * @param durable Flag to specify if is a durable connection to broker or not. Only applies to consumers.
		 * @param clientAck Flag to specify if the this library sends ack to each message or not (activemq-cpp will do it automatically).
		 * @param maxSizeQueue Maximun size for the internal queue of this consumer.
		 * @param username Username for connections that requires it
		 * @param password Password for connections that requires it
		 * @param clientId name of the client, it will be used for durable connections
		 * @param persistence is a number that specifies the number of messages that will be stored
		 * before delete it (if it is possible)
		 * @param certificate Path to the pem certificate if you want to use SSL protocol.
		 *
		 * @return pointer to connection if was created succesfull, else null.
		 */
		ActiveConnection* saveConnection(	std::string& id,
											std::string& ipBroker,
											int type,
											bool topic,
											std::string& destination,
											bool persistent,
											std::string& selector,
											bool durable,
											bool clientAck,
											int maxSizeQueue,
											std::string& username,
											std::string& password,
											std::string& clientId,
											int persistence,
											std::string& certificate);

		/**
		 * Method that start each service invoking his run method
		 *
		 * @throws ActiveException if something bad happens
		 */
		void startConnections() throw (ActiveException);

		/**
		 * Method that inserts links into links map
		 *
		 * @param linkId link id to save into links map
		 * @param activeLink Pointer to activelink to save into map
		 *
		 * @return true if link was created succesfully, else false.
		 *
		 * @throws ActiveException if something bad happens
		 */
		bool insertInLinksMap (	std::string& linkId, ActiveLink* activeLink) throw (ActiveException);


		////////////////////////////////////////////////////////////////////////////////
		// Callbacks methods
		////////////////////////////////////////////////////////////////////////////////

		/**
		 * Callback that the library will invoke when messages are received for his associated
		 * consumers ids.
		 *
		 * @param activeMessage is the message that the library sends to the user
		 */
		void onMessageCallback (ActiveMessage& activeMessage);

		/**
		 * Callback that the library will invoke when connection with one of his associated
		 * brokers is interrupted.
		 *
		 * @param connectionId is the connection id for the connection that is down
		 */
		void onConnectionInterruptCallback(std::string& connectionId);

		/**
		 * Callback that the library will invoke when connection with one of his associated
		 * brokers is up again.
		 *
		 * @param connectionId is the connection id for the connection that is up again
		 */
		void onConnectionRestoreCallback(std::string& connectionId);

		/**
		 * Callback that the library will invoke when the intern queue is full
		 *
		 * @param activeMessage is the message that the library sends to the user
		 */
		void onQueuePacketDropped(const ActiveMessage& activeMessage);

		/**
		 * Callback that the library will invoke when the intern queue is full
		 *
		 * @param connectionId connection that identify the connection who origin the exception
		 */
		void onException(std::string& connectionId);

		/**
		 * Callback that the library will invoke when the intern queue is ready again
		 *
		 * @param connectionId connection that identify the connection who origin the exception
		 */
		void onQueuePacketReady(std::string& connectionId);

		/**
		 * Class that implements the reader & writer thread safe method to
		 * access to a code block
		 */
		//ReadersWriters readersWriters;

		/**
		 * Destructor of the class
		 */
		virtual ~ActiveManager();

	private:
		/**
		 * Static var use by log4cxx for the logging system
		 */
		static log4cxx::LoggerPtr logger;

		/**
		 * var used to save the state of the singleton
		 */
		static bool instanceFlag;

		/**
		 * pointer to myself for the singleton
		 */
		static ActiveManager* mySelf;

		/**
		 * Pointer to class that manage xml configuration File
		 */
		ActiveXML activeXML;

		/**
		 * Reference to activeInterface for callback
		 */
		ActiveInterface* activeInterfacePtr;

		/**
		 * Data structure that stores all connections (producers and consumers).
		 */
		std::map <std::string,ActiveConnection*> connectionsMap;

		/**
		 * Data structure that represents the relation of different services
		 * with the really link. Links map has a reference from each link to
		 * each connection to send or receive through it.
		 */
		std::map <std::string,ActiveLink*> linksMap;

		/**
		 * Multimap that stores references to links for each service id.
		 * It allow the user to stablish a data structure in which I can send
		 * to a serviceId that is referenced to one or more links that are referenced
		 * to a connection.
		 */
		std::multimap <std::string,ActiveLink*> servicesMMap;

		/**
		 * Mutex that serializes the  access to onMessage function of the user
		 */
		ActiveMutex messageSerializer;

		/**
		 * Variable to serialize messages received or not for each connection
		 */
		bool messageSerializedInConsumption;

		/**
		 * Default constructor, is private for the singleton
		 */
		ActiveManager();

		/**
		 * Method used for initialize memory structures
		 *
		 * @throws ActiveException if something bad happens
		 */
		void initMemStructures() throw (ActiveException);

		/**
		 * Method for initialize ticpp xml library for reading configuration file
		 *
		 * @param configurationFile is the name for the configuration file to initialize library
		 *
		 * @throws ActiveException if something bad happens
		 */
		void initXMLLibrary(const std::string& configurationFile) throw (ActiveException);

		/**
		 * Method that invokes the run method for the given identifer of connection.
		 * This method will launch connection
		 *
		 * @param connectionId connection that is going to be run
		 *
		 * @throws ActiveException if something bad happens
		 */
		void startConnection(std::string& connectionId) throw (ActiveException);

		/**
		 * Method that for type received returns true if is a consumer and false is a producer
		 *
		 * @param type if is even will be consumer else producer
		 *
		 * @return bool if is a consumer else false
		 */
		bool isConsumer (int type);

		/**
		 * Method that is used to know if a connections exists in a list of connections
		 *
		 * @param activeConnection is the connection that we are going to check if it exists.
		 * @param connectionListR list of connections in which we are going to search for.
		 *
		 * @return true if exists, else false.
		 */
		bool existsInConnectionList(	ActiveConnection* activeConnection,
										std::list<ActiveConnection*>& connectionListR);

		/**
		 * Method to log some stringstream that calls to logfile
		 */
		void logIt (std::stringstream& logMessage);
	};
}

#endif /* ACTIVEMANAGER_H_ */
