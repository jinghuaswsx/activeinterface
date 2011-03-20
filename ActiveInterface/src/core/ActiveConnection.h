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
 * Class that is the parent class that abstracts from the real producer/consumer
 * wrappers. This class stores all information about this connection to the broker
 *
 */

#ifndef ACTIVECONNECTION_H_
#define ACTIVECONNECTION_H_

#include "message/ActiveMessage.h"

#include <decaf/lang/System.h>

#include "log4cxx/logger.h"
#include "log4cxx/helpers/exception.h"

using namespace ai;
using namespace ai::message;

namespace ai{

	class ActiveLink;

	class ActiveConnection {
	private:
		/**
		 * Static var use by log4cxx for the logging system
		 */
		static log4cxx::LoggerPtr logger;

		/**
		 * Flag to identify connection
		 */
		std::string id;

		/**
		 * Broker address string
		 */
		std::string brokerURI;

		/**
		 * Name of queue or topic
		 */
		std::string destinationURI;

		/**
		 * Topic(0) or queue(1)
		 */
		bool topic;

		/**
		 * String for saving message selector
		 */
		std::string selector;

		/**
		 * flag to know if i have to send response
		 */
		bool requestReply;

		/**
		 * Flag to know if is a durable message or not
		 */
		bool durable;

		/**
		 * Flag that specify the connection type
		 */
		int type;

		/**
		 * Flag to specify if a message is persistent
		 */
		bool persistent;

		/**
		 * flag to specify the ack mode
		 */
		bool clientAck;

		/**
		 * Flag that specifies the maximun size for the internal queue
		 */
		int maxSizeQueue;

		/**
		 * Username for connections who need it
		 */
		std::string username;

		/**
		 * Password for connections who need it
		 */
		std::string password;

		/**
		 * Client id to identificate the client
		 */
		std::string clientId;

		/**
		 *  link identificator that is associated with this connection
		 */
		std::string linkId;

		/**
		 * Flag that identifies the size of the persistence file
		 * If 0, no persistence
		 */
		long sizePersistence;

		/**
		 * flag to set the state of this connection
		 * 0-no initated 1-running  2-persistence 3-closed
		 */
		int state;

		/**
		 * path to the certificate to use SSL
		 */
		std::string certificate;

		/**
		 * var used to terminate consuming thread
		 */
		bool consumerThreadFlag;

	protected:

		//////////////////////////////////////////////////////////////////
		// setters are private
		void setId (std::string& idR){ id=idR;}
		void setClientId (std::string& clientIdR){clientId=clientIdR;}
		void setIpBroker (std::string& brokerURIR){ brokerURI=brokerURIR;}
		void setDestination (std::string& destinationR) {destinationURI=destinationR;}
		void setTopic (bool topicR) {topic=topicR;}
		void setSelector (std::string selectorR){selector=selectorR;}
		void setDurable (bool durableR){durable=durableR;}
		void setType (int typeR) { type=typeR;}
		void setPersistent (bool persistentR) {persistent=persistentR;}
		void setRequestReply (bool requestReplyR){ requestReply=requestReplyR;}
		void setClientAck (bool clientAckR) { clientAck=clientAckR;}
		void setMaxSizeQueue(int maxSizeQueueR){maxSizeQueue=maxSizeQueueR;}
		void setUsername(std::string& usernameR){username=usernameR;}
		void setPassword(std::string& passwordR){password=passwordR;}
		void setSizePersistence(long sizePersistenceR){sizePersistence=sizePersistenceR;}
		void setCertificate(std::string& certificateR){certificate=certificateR;}
		void endConsumerThread (){ consumerThreadFlag=true;}
		////////////////////////////////////////////////////////////////////

		/**
		 * Loading the packet description into activeMessage composed throug parameter and
		 * properties from activeMessage
		 *
		 * @param activeMessage message that contains properties and parameters that should be
		 * loaded into packet description
		 */
		void loadPacketDesc(ActiveMessage& activeMessage);

		/**
		 * Method that load parameters into packet desc
		 *
		 * @param activeMessage message from all parameters are extracted to be inserted
		 * into packet description header
		 */
		void loadPacketDescParameters(ActiveMessage& activeMessage);

		/**
		 * Method that load properties into packet desc
		 *
		 * @param activeMessage message from all properties are extracted to be inserted
		 * into packet description header
		 */
		void loadPacketDescProperties(ActiveMessage& activeMessage);

		/**
		 * Method to log some stringstream that calls to log4cxx
		 */
		void logIt (std::stringstream& logMessage);

	public:

		/**
		 * Default constructor
		 */
		ActiveConnection();

		/**
		 * Virtual method that is used to run specific producers/consumer that inherits from
		 * this class (parent class).
		 */
		virtual void run() abstract;

		/**
		 * Method invoked when we want to send something through a connection id
		 * It should be implemented in its derived classes
		 */
		virtual int send() abstract;

		/**
		 * virtual method to deliver a message into the intern queue
		 *
		 * @param activeMessage is the message that is going to be store in the queue
		 * @param activeLink is the link associated. Used to copy default properties read from
		 * xml configuration file to activeMessage.
		 *
		 * @return int Returning the position in the queue in which is stored this message
		 */
		virtual int deliver (ActiveMessage& activeMessage, ActiveLink& activeLink) abstract;

		/**
		 * virtual method to deliver a message into the intern queue. This method is used
		 * for request reply answers because this answers does not have default properties to add
		 *
		 * @param activeMessage is the message that is going to be store in the queue
		 *
		 * @return int Returning the position in the queue in which is stored this message
		 */
		virtual int deliver (ActiveMessage& activeMessage) abstract;

		/**
		 * Method that receives the messages synchronously
		 */
		virtual int onReceive() abstract;

		//////////////////////////////////////////////////////////
		// getters
		std::string& getId() { return id;}
		std::string& getLinkId() { return linkId;}
		std::string& getClientId(){return clientId;}
		std::string getStringClientId();
		std::string& getIpBroker (){ return brokerURI;}
		std::string& getDestination(){ return destinationURI;}
		bool getTopic (){ return topic;}
		int getType (){ return type;}
		//Im going to return the negative persistent because
		//for cms persistent is 0 and 1 non persistent
		//but for the users is prefer to say the negative
		bool getPersistent () {return !persistent;}
		std::string getSelector(){return selector;}
		int isDurable(){return durable;}
		bool getRequestReply () { return requestReply;}
		int getClientAck (){ return clientAck;}
		int getMaxSizeQueue(){return maxSizeQueue;}
		std::string& getUsername() {return username;}
		std::string& getPassword() {return password;}
		long getSizePersistence() {return sizePersistence;}
		int getState (){ return state;}
		std::string& getCertificate(){return certificate;}
		bool getEndConsumerThread (){ return consumerThreadFlag;}
		////////////////////////////////////////////////////////////////
		void setLinkId (std::string& linkIdR){ linkId=linkIdR;}
		void setState (int stateR){state=stateR;}

		/**
		 * virtual method that says if the connection is in recovery mode
		 * or not
		 */
		virtual bool isInRecoveryMode() abstract;

		/**
		 * Method that initialices the SSL Support.
		 */
		void initSSLSupport();

		/**
		 * Method that cleans up the connection, close connection and free resources
		 */
		virtual void close() abstract;

		/**
		 * Destructor by default
		 */
		virtual ~ActiveConnection(){}
	};
}

#endif /* ACTIVECONNECTION_H_ */
