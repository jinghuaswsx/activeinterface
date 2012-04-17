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
 * Class that implements the association between a real connection to the broker
 * (for us ActiveConnection) and the services.
 * Link is an abstract entity that allows the user to do:
 *      - Abstraction about sending the same message to different connections. We have
 *      services that are compossed by one or more links, that allow the user to configure
 *      that all messages that are going to be sent by this service id, are  going to be
 *      distributed to more than one link (more than one connection).
 *      - Abstracion about default properties. Links allows the users to specify different
 *      properties for each one. For example, we could want to send to a service id
 *      that sends two messages (service has two links) and each links has its owns properties.
 *      It is very useful for selector and to configure default properties that the developer or
 *      doesn't need to be care about it because are going to be send automatically in all
 *      messages sent by this link.
 */

#ifndef ACTIVELINK_H_
#define ACTIVELINK_H_

#include "ActiveConnection.h"
#include "../utils/parameters/ParameterList.h"

#include "log4cxx/logger.h"
#include "log4cxx/helpers/exception.h"

namespace ai{

	class ActiveLink{
	private:
		/**
		 * Static var use by log4cxx for the logging system
		 */
		static log4cxx::LoggerPtr logger;

		/**
		 * String that identifies each link
		 */
		std::string id;

		/**
		 * name for this link
		 */
		std::string name;

		/**
		 * Pointer to active connection, this pointer allows a link to send
		 * through a connection
		 */
		ActiveConnection* activeConnection;

		/**
		 * Data structure to save all properties for each proxylink
		 * and for each service. See parameterList class.
		 */
		ParameterList propertiesList;


		/**
		 * method to log some stringstream that calls to log4cxx
		 *
		 * @param logMessage stringstream that stores the message to log.
		 */
		void logIt (std::stringstream& logMessage);

	public:

		/**
		 * Method that returns the number of properties associated with this link
		 *
		 * @return size of properties list
		 */
		int getPropertySize() { return propertiesList.size();}

		/**
		 * Method used to get a property using key string name from properties list
		 *
		 * @return parameter stored with this key
		 */
		Parameter* getProperty (std::string& key){ return propertiesList.get(key);}

		/**
		 * Method used to loop into a property list getting the property stored in index position
		 *
		 * @param index is the position that is going to return from the properties list
		 * @param key is a reference that i have to pass to method and is filled with the key for the property
		 * stored in this position
		 * @return parameter stored in this index position
		 */
		Parameter* getProperty (int index, std::string& key){ return propertiesList.get(index,key);}

		/**
		 * Methods that insert integer parameter in message with a key associated
		 *
		 * @param key string to save value into map
		 * @param value int value to save into map
		 */
		void insertIntProperty(	std::string& key,
									int value);

		/**
		 * Methods that insert real parameter in message with a key associated
		 *
		 * @param key string to save value into map
		 * @param value float value to save into map
		 */
		void insertRealProperty(	std::string& key,
									float value);

		/**
		 * Methods that insert string parameter in message with a key associated
		 *
		 * @param key string to save value into map
		 * @param value string value to save into map
		 */
		void insertStringProperty(	std::string& key,
									std::string& value);

		/**
		 * Methods that insert bytes parameter in message with a key associated
		 *
		 * @param key string to save value into map
		 * @param value bytes value to save into map
		 */
		void insertBytesProperty(	std::string& key,
									std::vector<unsigned char>& value);


		/**
		 * Method that clears all properties from this link
		 */
		void clearProperties();

		/**
		 * Default constructor
		 *
		 * @param idR Is the identifier for this link
		 * @param nameR is the name for this link
		 * @param activeConnectionR is the connection to this link is going to be associated
		 */
		ActiveLink(std::string& idR, std::string& nameR, ActiveConnection* activeConnectionR);


		/////////////////////////////////////////////////////////
		//setters & getters
		////////////////////////////////////////////////////////
		void setId (int idR){ id=idR;}
		void setName (std::string& nameR){ name=nameR;}
		void setConnection (ActiveConnection* activeConnectionR){ activeConnection=activeConnectionR;}

		std::string& getId (){return id;}
		std::string& getName (){return name;}
		ActiveConnection* getActiveConnection (){return activeConnection;}
		void removeConnBinding(){ activeConnection=NULL;}
		/////////////////////////////////////////////////////////////////////////////////////////////

		/**
		 * Default destructor
		 */
		virtual ~ActiveLink();
	};
}

#endif /* ACTIVELINK_H_ */
