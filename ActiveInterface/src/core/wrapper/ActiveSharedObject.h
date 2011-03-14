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
 * Class that implements interface with application. Is the entering point to the library, and
 * gives users all methods to interact with the wrapper.
 */

#ifndef ACTIVESHAREDOBJECT_H_
#define ACTIVESHAREDOBJECT_H_

#include <apr_general.h>
#include <apr_thread_proc.h>
#include <apr_thread_cond.h>

namespace ai{

	class ActiveSharedObject {
	private:
		/**
		 * APR mutex used to provides mutual exclusion accesing to the queue
		 */
		apr_thread_mutex_t* mutex;

		/**
		 * APR mutex conditional. See more documentation at APR threads.
		 */
		apr_thread_cond_t* cond;

		/**
		 * Shared content that stores the number of messages ready to send
		 */
		long long messagesReady;

		/**
		 * flag to end thread
		 */
		bool endThread;

	public:
		/**
		 * Default constructor
		 */
		ActiveSharedObject(){ 	mutex=NULL;
								cond=NULL;
								messagesReady=0;
								endThread=false;
		}

		/**
		 * Method that returns the condition
		 *
		 * @return Pointer to thread condition
		 */
		apr_thread_cond_t* getCond(){ return cond;};

		/**
		 * Method that returns the mutex
		 *
		 * @return Pointer to mutex of readers/writers semaphore
		 */
		apr_thread_mutex_t* getMutex(){ return mutex;}

		/**
		 * Method that returns a pointer to pointer to the mutex
		 *
		 * @returns Pointer of pointer to mutex
		 */
		apr_thread_mutex_t** getMutexPtr (){return &mutex;}

		/**
		 * Method that returns a pointer to pointer to the condition
		 *
		 * @return pointer to pointer to condition
		 */
		apr_thread_cond_t** getCondPtr(){ return &cond;}

		/**
		 * Returns the number of messages that are ready to send
		 *
		 * @return Number of messages pending to send in the queue.
		 */
		long long getMessagesReady () {return messagesReady;}

		/**
		 * sets the number of messages that are ready to send
		 *
		 */
		void setMessagesReady (long long messagesReadyR) {messagesReady=messagesReadyR;}

		/**
		 * Method that increments the number of message ready to send
		 */
		void newMessage(){ messagesReady++;}

		/**
		 * Method that decrements the number of messages ready to send
		 */
		void messageSent(){ messagesReady--;}

		/**
		 * method to end the thread
		 */
		void setEndThread(){ endThread=true;}

		/**
		 * method to know when the thread needs to dead
		 */
		bool getEndThread(){ return endThread;}

		/**
		 * Default destructor
		 */
		virtual ~ActiveSharedObject(){}
	};
}

#endif /* ACTIVESHAREDOBJECT_H_ */
