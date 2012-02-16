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
 * Class that implements the main message that we are going to send/receive
 * to/from the library.
 * This ActiveMessage gives the user an upper abstracion layer about JMS messages,
 * giving the possibility to send different data types in the same message only using
 * its upper abstraction methods.
 * This message is built like a heterogeneus map, or a map that stores polymorphic types.
 * Every entry of the map of this message has a key but is possible to loop through
 * all data stored in the map.
 */

#ifndef ACTIVEMESSAGE_H_
#define ACTIVEMESSAGE_H_

#ifdef ACTIVEINTERFACE_DLL
 #ifdef ACTIVEINTERFACE_EXPORTS
  #define ACTIVEINTERFACE_API __declspec( dllexport )
 #else
  #define ACTIVEINTERFACE_API __declspec( dllimport )
 #endif
#else 
 #define ACTIVEINTERFACE_API
#endif

#include "ActiveDestination.h"
#include "../../utils/parameters/ParameterList.h"
#include "../../utils/exception/ActiveException.h"

#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

#include "log4cxx/logger.h"
#include "log4cxx/helpers/exception.h"

using namespace ai::utils;

namespace ai{
 namespace message{

	class ACTIVEINTERFACE_API ActiveMessage{

	private:

		/**
		 * Service Id that is going to be used to know to
		 * which service is going to send the message
		 */
		std::string serviceId;

		/**
		 * Identification to know link id from message comes
		 */
		std::string linkId;

		/**
		 * connection id used to know the connection from message comes
		 */
		std::string connectionId;

		/**
		 * Defines the message time to live. For more documentation see JMS timeToLive
		 */
		long long timeToLive;

		/**
		 * defines the message priority. For more documentation see JMS priorities
		 */
		int priority;

		/**
		 * Flag sets by the library to show the users if the message needs to be respond
		 */
		bool requestReply;

		/**
		 * This string is set by the user. When the user sends a message that needs a respond
		 * this correlation id should be saved by the user.
		 * When a message is received the user will be informed with this correlation id to
		 * know exactly which message is acknowledged.
		 */
		std::string correlationId;

		/**
		 * Text that is going to be added to the message only if is defined as a
		 * text message
		 */
		std::string text;

		/**
		 * Boolean var that specifiy if the message is going to be send as a text
		 * message. If it is not defined message will be sent in ActiveInterface
		 * format (streamMessage).
		 */
		bool textMessage;

		/**
		 * This array of bytes is used to describe the packet
		 * when you write an int this array will have 0 and the same
		 * for other types. This array will have one byte for each
		 * parameter and property included in message.
		 *
		 * This will be used in the retrieve to know how many parameters
		 * and properties will have and its type.
		 * Is transparent for the users.
		 */
		std::vector<unsigned char> packetDesc;

		/**
		 * When a message needs to be respond, the consumer that consumes this message
		 * needs to know where to respond. This att store requestReply destination.
		 * The user when receives a message with requestReply flag activated needs to clone
		 * this destination and set to the message that wants to send back to the
		 * connection who origin the message.
		 */
		ActiveDestination activeDestination;

		/**
		 * Is where the message is going to store all parameters that
		 * the user wants to send.
		 */
		ParameterList parameterList;

		/**
		 * Is where the message is going to store all properties that
		 * the user wants to send.
		 */
		ParameterList propertiesList;

		/**
		 * Method to log some stringstream that calls to log4cxx
		 *
		 * @param logMessage stringstream to be logged
		 */
		void logIt (std::stringstream& logMessage);

		/**
		 * Method that copy all data included in parameterList and
		 * propertiesList from the message received to the actualMessage
		 *
		 * @param activeMessageR message from we are going to clone parameters
		 * and properties to the new message
		 */
		void cloneLists (const ActiveMessage& activeMessageR) throw (ActiveException);

		/**
		 * Static var use by log4cxx for the logging system
		 */
		static log4cxx::LoggerPtr logger;


	public:

		/**
		 * method that clears the package description
		 */
		void clearPacketDesc (){packetDesc.clear();}


		/**
		 * method used internally to fill the packet description
		 *
		 * @param paramDesc is the description of the param that is going to be inserted
		 * 0 - for integers parameters
		 * 1 - for reals parameters
		 * 2 - for strings parameters
		 * 3-  for bytes parameters
		 * 10- for integers properties
		 * 11- for reals properties
		 * 12- for string properties
		 *
		 * used internally only.
		 */
		void pushInPacketDesc (int paramDesc){packetDesc.push_back(paramDesc);}

		/**
		 * Method that returns the packet description
		 *
		 * @return vector with the package description
		 */
		const std::vector<unsigned char>& getPacketDesc () const {return packetDesc;}

		/**
		 * Method that clone the destination
		 */
		void cloneDestination (const cms::Destination* destinationR);

		////////////////////////////////////////////////////
		// setters and getters
		////////////////////////////////////////////////////
		const std::string& getServiceId() const {return serviceId;}
		const std::string& getLinkId() const {return linkId;}
		const std::string& getConnectionId() const {return connectionId;}
		long long getTimeToLive() const {return timeToLive;}
		int getPriority() const { return priority;}
		bool getRequestReply() const {return requestReply;}
		//method to know if i have to reply, to be more readable
		bool isWithRequestReply() const { return requestReply;}
		const std::string& getCorrelationId()const {return correlationId;}
		const std::string& getText() const {return text;}
		const ActiveDestination& getDestination() const { return activeDestination;}

		void setServiceId(std::string& serviceIdR){ serviceId=serviceIdR;}
		void setLinkId(std::string& linkIdR){linkId=linkIdR;}
		void setConnectionId(std::string& connectionIdR){connectionId=connectionIdR;}
		void setTimeToLive(long long timeToLiveR){ timeToLive=timeToLiveR;}
		void setPriority (int priorityR) { priority=priorityR;}
		void setRequestReply (bool requestReplyR) { requestReply=requestReplyR;}
		void setCorrelationId(std::string& correlationIdR){ correlationId=correlationIdR;}
		void setDestination (const ActiveDestination& activeDestinationR){ activeDestination.clone(activeDestinationR);}
		void setText(std::string& textR){ text=textR; textMessage=true;}

		////////////////////////////////////////////////////////////////////////////////////////////


		/////////////////////////////////////////////////////////////////////////////////////////
		// Methods to manage parameters
		/////////////////////////////////////////////////////////////////////////////////////////
		/**
		 * Method that gets the reference of the parameter list of this message
		 *
		 * @return parameter list reference.
		 */
		const ParameterList& getParameterList() const { return parameterList;}

		/**
		 * Method that returns the size of parameters
		 *
		 * @return numbers of parameters
		 */
		int getParametersSize() const { return parameterList.size();}

		/**
		 * method that allow the user to get parameter by key
		 *
		 * @param key key to be found
		 * @return parameter if key exists or null
		 */
		Parameter* getParameter (std::string& key) const { return parameterList.get(key);}

		/**
		 * Method to get an int parameter directly
		 *
		 * @param key key to find.
		 * @return pointer to int parameter
		 * @throw ActiveException if parameter is not int
		 */
		IntParameter* getIntParameter(std::string& key) const
				throw (ActiveException);
		/**
		 * Method to get an real parameter directly
		 *
		 * @param key key to find.
		 * @return pointer to real parameter
		 * @throw ActiveException if parameter is not real
		 */
		RealParameter* getRealParameter(std::string& key) const
				throw (ActiveException);

		/**
		 * Method to get an string parameter directly
		 *
		 * @param key key to find.
		 * @return pointer to string parameter
		 * @throw ActiveException if parameter is not string
		 */
		StringParameter* getStringParameter(std::string& key) const
				throw (ActiveException);

		/**
		 * Method to get a bytes parameter directly
		 *
		 * @param key key to find.
		 * @return pointer to bytes parameter
		 * @throw ActiveException if parameter is not bytes
		 */
		BytesParameter* getBytesParameter(std::string& key) const
				throw (ActiveException);

		/**
		 * Method that get parameter by its position in parameter list. Used to loop around
		 * all parameters
		 *
		 * @param index position to retrieve from parameterList
		 * @param key reference to a string that will be fill up with key of the
		 * parameter store at this index position
		 * @return parameter or NULL
		 */
		Parameter* getParameter (int index, std::string& key) const { return parameterList.get(index,key);}

		/**
		 * Methods that insert integer parameter into parameter list
		 *
		 * @param key key associated with value
		 * @param value value that is going to be stored
		 */
		void insertIntParameter(	std::string& key,
									int value);

		/**
		 * Methods that insert real parameter into parameter list
		 *
		 * @param key key associated with value
		 * @param value value that is going to be stored
		 */
		void insertRealParameter(	std::string& key,
									float value);

		/**
		 * Methods that insert string parameter into parameter list
		 *
		 * @param key key associated with value
		 * @param value value that is going to be stored
		 */
		void insertStringParameter(	std::string& key,
									std::string& value);

		/**
		 * Methods that insert bytes parameter into parameter list
		 *
		 * @param key key associated with value
		 * @param value value that is going to be stored
		 */
		void insertBytesParameter(	std::string& key,
									std::vector<unsigned char>& value);

		/**
		 * Method that allows user to delete a parameter included
		 * in message using the key
		 *
		 * @param key key of the map that is going to be erased
		 */
		void deleteParameter(std::string& key);

		/**
		 * Method that clears all parameters
		 */
		void clearParameters ();

		/////////////////////////////////////////////////////////////////////////////
		// methods to manage properties
		///////////////////////////////////////////////////////////////////////////////

		/**
		 * Method that gets the reference of the property list of this message
		 *
		 * @return property list reference.
		 */
		const ParameterList& getPropertiesList() const { return propertiesList;}

		/**
		 * Method that returns the  properties size
		 *
		 * @return numbers of properties
		 */
		int getPropertiesSize() const { return propertiesList.size();}

		/**
		 * method that allow the user to get property by key
		 *
		 * @param key key to be found
		 * @return property if key exists or null
		 */
		Parameter* getProperty (std::string& key) const { return propertiesList.get(key);}

		/**
		 * Method that get property by its position in parameter list. Used to loop around
		 * all parameters
		 *
		 * @param index position to retrieve from parameterList
		 * @param key reference to a string that will be fill up with key of the
		 * property store at this index position
		 * @return property or NULL
		 */
		Parameter* getProperty (int index, std::string& key) const { return propertiesList.get(index,key);}

		/**
		 * Method to get an int property directly
		 *
		 * @param key key to find.
		 * @return pointer to int parameter
		 * @throw ActiveException if parameter is not int
		 */
		IntParameter* getIntProperty(std::string& key) const
				throw (ActiveException);

		/**
		 * Method to get an real property directly
		 *
		 * @param key key to find.
		 * @return pointer to real parameter
		 * @throw ActiveException if parameter is not real
		 */
		RealParameter* getRealProperty(std::string& key) const
				throw (ActiveException);

		/**
		 * Method to get an string property directly
		 *
		 * @param key key to find.
		 * @return pointer to string parameter
		 * @throw ActiveException if parameter is not string
		 */
		StringParameter* getStringProperty(std::string& key) const
				throw (ActiveException);

		/**
		 * Methods that insert integer property into parameter list
		 *
		 * @param key key associated with value
		 * @param value value that is going to be stored
		 */
		void insertIntProperty(	std::string& key,
									int value);

		/**
		 * Methods that insert real property into parameter list
		 *
		 * @param key key associated with value
		 * @param value value that is going to be stored
		 */
		void insertRealProperty(	std::string& key,
									float value);

		/**
		 * Methods that insert string property into parameter list
		 *
		 * @param key key associated with value
		 * @param value value that is going to be stored
		 */
		void insertStringProperty(	std::string& key,
									const std::string& value);


		/**
		 * Method that allows user to delete a property included
		 * in message using the key
		 *
		 * @param key key of the map that is going to be erased
		 */
		void deleteProperty(std::string& key);

		/**
		 * Method that clears all properties
		 */
		void clearProperties();

		//////////////////////////////////////////////////////////////////////////////////

		/**
		 * method that returns if actual message is a text message
		 *
		 * return True if message is text message, else false.
		 */
		bool isTextMessage ()const{return textMessage;}

		/**
		 * Method that sets message as a textMessage, if this method
		 * is set to a message, this message is going to be send
		 * as a text message, only sending parameter added by
		 * setText.
		 */
		void setMessageAsText (){textMessage=true;}

		/**
		 * Method that clear all data of the message
		 */
		void clear();

		/**
		 * Methods to clone message received into new message
		 *
		 * @param activeMessageR message that is going to be cloned.
		 *
		 * @throws ActiveException if something bad happens
		 */
		void clone(const ActiveMessage& activeMessageR) throw (ActiveException);

		/**
		 * Default copy constructor
		 *
		 * @param activeMessage message that is going to be cloned
		 */
		ActiveMessage(const ActiveMessage &activeMessage);

		/**
		 * Default constructor
		 */
		ActiveMessage();

		/**
		 * Default destructor
		 */
		virtual ~ActiveMessage();

		//making activemessage as a serializable class
		friend class boost::serialization::access;
		template<class Archive>
	
		/**
		 * Method that describes atts that are going to be serialized
		 */
		void serialize(Archive & ar, const unsigned int version){
			ar & serviceId;
			ar & linkId;
			ar & connectionId;
			ar & timeToLive;
			ar & priority;
			ar & requestReply;
			ar & correlationId;
			ar & text;
			ar & textMessage;
			ar & packetDesc;
			ar & parameterList;
			ar & propertiesList;
		}
	};
 }
}

#endif /* ACTIVEMESSAGE_H_ */
