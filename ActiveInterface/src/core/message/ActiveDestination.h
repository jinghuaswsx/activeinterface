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
 * Class that encapsulate the cms::destination class
 *
 */

#ifndef ACTIVEDESTINATION_H_
#define ACTIVEDESTINATION_H_

#ifdef ACTIVEINTERFACE_DLL
 #ifdef ACTIVEINTERFACE_EXPORTS
  #define ACTIVEINTERFACE_API __declspec( dllexport )
 #else
  #define ACTIVEINTERFACE_API __declspec( dllimport )
 #endif
#else 
 #define ACTIVEINTERFACE_API
#endif

#include <cms/Session.h>

namespace ai {

	class ACTIVEINTERFACE_API ActiveDestination {
	private:
		const cms::Destination* replyTo;

	public:
		/**
		 * Default constructor
		 */
		ActiveDestination();

		/**
		 * Method that gets the pointer to replyTo
		 *
		 * @returns destination* pointer to destination.
		 */
		const cms::Destination* getReplyTo ()const { return replyTo;}

		/**
		 *	Method that sets the destination to where we need to
		 *	response.
		 *
		 * @param replyToR destination to respond.
		 */
		void setReplyTo(const cms::Destination* replyToR);

		/**
		 *	Method that clone the destination object. If you are saving this
		 *	destination on your own spce, you need to clone it.
		 *
		 *	@param activeDestinationR destination to clone.
		 */
		void clone(const ActiveDestination& activeDestinationR);

		/**
		 *	Method that is used internally to clone the activemq-cpp pointer
		 *	to destination
		 *
		 *	@param replyToR pointer to clone.
		 */
		void clone (const cms::Destination* replyToR);

		/**
		 *	Method that clear the destination.
		 */
		void clear (){ replyTo=NULL;}

		/**
		 *	Copy constructor
		 */
		void copy(const ActiveDestination& activeDestinationR);

		/**
		 *	Default destructor
		 */
		virtual ~ActiveDestination();
	};
}

#endif /* ACTIVEDESTINATION_H_ */
