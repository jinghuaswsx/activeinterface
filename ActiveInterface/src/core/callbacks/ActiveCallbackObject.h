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
 * Object that stores all attributes and data that we need for the callbacks.
 * Defines the types of the callback and its contents.
 */


#ifndef ACTIVECALLBACKOBJECT_H_
#define ACTIVECALLBACKOBJECT_H_

#include "../message/ActiveMessage.h"

using namespace ai::message;

namespace ai{

	class ActiveCallbackObject {
	private:

		/**
		 * type of callback (defines.h)
		 */
		int type;

		/**
		 * message to pass, if we need to
		 */
		std::string message;

		/**
		 * message that is going to be in the queue
		 */
		ActiveMessage activeMessage;

		/**
		 * connection id who origin the message
		 */
		std::string connectionId;

	public:

		/**
		 * Empty constructor
		 */
		ActiveCallbackObject(){
			type=-1;
			connectionId.clear();
		}

		/**
		 * Default constructor
		 *
		 * @param typeR type of the callback.
		 * @param connectionIdR identifier of the connection who origins the callback
		 * @param activeMessageR message that is going to be pass to the user block,
		 * this param is used for drop messages.
		 * @param messageR Is a message to gives more information to the user about
		 * the callback
		 */
		ActiveCallbackObject(	int typeR,
								std::string& connectionIdR,
								ActiveMessage& activeMessageR,
								const std::string& messageR="New Message"){

			activeMessage.clone(activeMessageR);
			type=typeR;
			message=messageR;
			connectionId=connectionIdR;
		}

		/**
		 * Default constructor
		 *
		 * @param typeR type of the callback.
		 * @param connectionIdR identifier of the connection who origins the callback
		 * @param messageR Is a message to gives more information to the user about
		 * the callback
		 */
		ActiveCallbackObject(	int typeR,
								std::string&  connectionIdR,
								const std::string& messageR="New Message"){
			type=typeR;
			message=messageR;
			connectionId=connectionIdR;
		}

		/**
		 * copy constructor
		 *
		 * @param activeCallbackObjectR reference to a class to make the copy.
		 */
		ActiveCallbackObject(const ActiveCallbackObject& activeCallbackObjectR){
			clone(const_cast<ActiveCallbackObject&>(activeCallbackObjectR));
		}

		/**
		 * clone method
		 *
		 * @param activeCallbackObjectR reference to a class to make the clone.
		 */
		void clone (ActiveCallbackObject& activeCallbackObjectR){
			setType(activeCallbackObjectR.getType());
			setActiveMessage(activeCallbackObjectR.getActiveMessage());
			setMessage(activeCallbackObjectR.getMessage());
			setConnectionId(const_cast<std::string&>(activeCallbackObjectR.getConnectionId()));
		}

		/**
		 * Setter of activeMessage
		 *
		 * @param activeMessageR active message to set (is going to clone it).
		 */
		void setActiveMessage(ActiveMessage& activeMessageR){ activeMessage.clone(activeMessageR);}

		/**
		 * Setter type
		 *
		 * @param typeR type of the callback message
		 */
		void setType(int typeR){ type=typeR;}

		/**
		 * Settter of message
		 *
		 * @param messageR message to set.
		 */
		void setMessage (const std::string& messageR){ message=messageR; }

		/**
		 * setter of connection id
		 *
		 * @param connectionIdR connection identifier
		 */
		void setConnectionId (std::string& connectionIdR){ connectionId=connectionIdR;}

		/**
		 * Getting the message stored
		 *
		 * @return The message stored in this object.
		 */
		ActiveMessage& getActiveMessage () {return activeMessage;}

		/**
		 * get type
		 *
		 * @return the type of the callback message.
		 */
		int getType () { return type;}

		/**
		 * get connection id
		 *
		 * @return the connection id that created the callback
		 */
		std::string& getConnectionId () { return connectionId;}

		/**
		 * Getting message
		 *
		 * @return string of the message stored
		 */
		std::string& getMessage() {return message;}

		/**
		 * Default destructor
		 */
		virtual ~ActiveCallbackObject(){}
	};
}

#endif /* ACTIVECALLBACKOBJECT_H_ */
