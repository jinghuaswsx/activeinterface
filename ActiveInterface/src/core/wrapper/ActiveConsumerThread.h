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

#ifndef ACTIVECONSUMERTHREAD_H_
#define ACTIVECONSUMERTHREAD_H_

#include <apr_general.h>
#include <apr_thread_proc.h>
#include <apr_thread_cond.h>

#include "../ActiveConnection.h"

namespace ai{

	class ActiveConsumerThread {
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
		 * Flag to know if the thread started to run
		 */
		int threadRunning;

		/**
		 * Pointer to the producer that spawn this thread
		 */
		ActiveConnection* activeConnection;

	public:
		/**
		 * Default constructor
		 */
		ActiveConsumerThread();

		/**
		 * Method that returns the active connection associated
		 *
		 * @return active connection to communicate thread with the real connection
		 */
		ActiveConnection* getActiveConnection(){ return activeConnection;}

		/**
		 * Method that initializes pointers and structures that are going
		 * to be used by the consumer thread.
		 *
		 * @param activeConnectionR Reference to the real connection that spawn thread
		 */
		void init (	ActiveConnection* activeConnectionR);

		/**
		 * Method that starts the thread
		 *
		 * @return 0 if the thread spawn went fine. See more documentation at APR returns values of creating threads
		 */
		int runThread ();

		/**
		 * Default destructor
		 */
		virtual ~ActiveConsumerThread();
	};
}

#endif /* ACTIVECONSUMERTHREAD_H_ */
