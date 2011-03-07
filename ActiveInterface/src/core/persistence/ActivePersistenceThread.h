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
 */

#ifndef ACTIVEPERSISTENCETHREAD_H_
#define ACTIVEPERSISTENCETHREAD_H_

#include <apr_general.h>
#include <apr_thread_proc.h>
#include <apr_thread_cond.h>

#include "log4cxx/logger.h"
#include "log4cxx/helpers/exception.h"

#include "../wrapper/ActiveSharedObject.h"


namespace ai{

	class ActivePersistence;

	class ActivePersistenceThread {
	private:
		/**
		 * APR flag that saves the status of the thread
		 */
		apr_status_t rv;

		/**
		 * APR pool to manage threads
		 */
		apr_pool_t *mp;

		/**
		 * APR pointer to the real thread
		 */
		apr_thread_t *thd_arr;

		/**
		 * APR pointer to pass atts to the thread
		 */
		apr_threadattr_t *thd_attr;

		/**
		 * Static var use by log4cxx for the logging system
		 */
		static log4cxx::LoggerPtr logger;

		/**
		 * APR mutex used to provides mutual exclusion accesing to the queue
		 */
		ActiveSharedObject activeSharedObject;

		/**
		 * Flag to know if the thread started to run
		 */
		int threadRunning;

		/**
		 * Pointer to active persistence to invoke the method that enqueues.
		 */
		ActivePersistence* activePersistence;

		/**
		 * Method to log a message with INFO level
		 */
		void logIt (std::stringstream& logMessage);

	public:

		/**
		 * Default constructor
		 */
		ActivePersistenceThread();

		/**
		 * Method that initializes pointers and structures that are going
		 * to be used by the producer thread.
		 *
		 * @param activeConnectionR Reference to the real connection that spawn thread
		 */
		void init (ActivePersistence& activePersistence);

		/**
		 * Method that starts the thread
		 *
		 * @return 0 if the thread spawn went fine. See more documentation at APR returns values of creating threads
		 */
		int runPersistenceThread ();

		/**
		 * Method that returns the active connection associated
		 *
		 * @return active connection to communicate thread with the real connection
		 */
		ActivePersistence* getActivePersistence(){ return activePersistence;}

		/**
		 * Method that returns the shared object
		 *
		 * @return Shared object used by both threads (reader/writers)
		 */
		ActiveSharedObject* getActiveSharedObject(){ return &activeSharedObject;}

		/**
		 * Set the active connection associated with thread
		 *
		 * @param activeConnectionR Pointer to the connection that thread uses to send messages
		 */
		void setActivePersistence(ActivePersistence* activePersistenceR){ activePersistence=activePersistenceR;}

		/**
		 * This method is used to increment/decrement the number of messages that
		 * is still pending to send by the thread. This methods unlock the writer thread
		 * when a new message comes.
		 *
		 * @param receive Default value is true, that says to the semaphore that a new message
		 * is in the queue ready to be send. False value is used to decrement the number
		 * of messages that are still pending to send.
		 */
		void newMessage(bool receive);

		/**
		 *	Method used to reset the messages that are going to be spawned.
		 */
		void resetMessages();

		/**
		 *	Default destructor
		 */
		virtual ~ActivePersistenceThread();
	};
}

#endif /* ACTIVEPERSISTENCETHREAD_H_ */
