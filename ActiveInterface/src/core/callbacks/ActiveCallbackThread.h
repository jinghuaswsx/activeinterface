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
 * @section Class that implements all methods related with the thread that
 * makes the callbacks to the user part.
 *
 */

#ifndef ACTIVECALLBACKTHREAD_H_
#define ACTIVECALLBACKTHREAD_H_

#include "log4cxx/logger.h"

#include "../ActiveManager.h"
#include "ActiveCallbackSharedObject.h"
#include "../queue/ActiveCallbackQueue.h"

namespace ai{

	class ActiveCallbackThread {
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
		 * APR pointer to pass atts to the thread, used for the
		 * main thread
		 */
		apr_thread_t *thd_main;

		/**
		 * Static var use by log4cxx for the logging system
		 */
		static log4cxx::LoggerPtr logger;

		/**
		 * Flag to know if the thread started to run
		 */
		int threadRunning;

		/**
		 * reference to the queue
		 */
		ActiveCallbackQueue* activeCallbackQueue;

		/**
		 * the shared object to implements
		 * producer - consumer callback blocking
		 */
		ActiveCallbackSharedObject activeSharedObject;

		/**
		 *	Method that throws the callback to packetDropped function in the
		 *	user part.
		 *
		 * @param activeCallbackObject object that stores all information about
		 * callback that is going to be spawned to the user.
		 */
		void throwPacketDropped(ActiveCallbackObject& activeCallbackObject);

		/**
		 *	Method that throws the callback to onException function in the
		 *	user part.
		 *
		 * @param activeCallbackObject object that stores all information about
		 * callback that is going to be spawned to the user.
		 */
		void throwOnException(ActiveCallbackObject& activeCallbackObject);

		/**
		 *	Method that throws the callback to OnTransportInterrupt function in the
		 *	user part.
		 *
		 * @param activeCallbackObject object that stores all information about
		 * callback that is going to be spawned to the user.
		 */
		void throwOnTransportInterrupt(ActiveCallbackObject& activeCallbackObject);

		/**
		 *	Method that throws the callback to wOnTransportResumed function in the
		 *	user part.
		 *
		 * @param activeCallbackObject object that stores all information about
		 * callback that is going to be spawned to the user.
		 */
		void throwOnTransportResumed(ActiveCallbackObject& activeCallbackObject);

		/**
		 *	Method that throws the callback to OnQueueReady function in the
		 *	user part.
		 *
		 * @param activeCallbackObject object that stores all information about
		 * callback that is going to be spawned to the user.
		 */
		void throwOnQueueReady(ActiveCallbackObject& activeCallbackObject);

	public:

		/**
		 * Default constructor that initializes all APR symbols
		 */
		ActiveCallbackThread();

		/**
		 * Method that initializes pointers and structures that are going
		 * to be used by the producer thread.
		 *
		 * @param activeCallbackQueueR  reference to the queue of callback
		 * messages.
		 */
		void init (ActiveCallbackQueue& activeCallbackQueueR);

		/**
		 * Method that returns the shared object that the thread uses to
		 * be synchronized using the semaphore implemented.
		 *
		 * @return Shared object used by both threads (reader/writers)
		 */
		ActiveCallbackSharedObject* getActiveSharedObject(){ return &activeSharedObject;}

		/**
		 * Method that starts the thread
		 *
		 * @return 0 if the thread spawn went fine. See more documentation at APR returns values of creating threads
		 */
		int runCallbackThread ();

		/**
		 * This method is used to increment/decrement the number of messages
		 * that we have waiting to send
		 *
		 * @param received If is true, message counter is going to be incremented
		 * else decremented
		 */
		void newCallback(bool received);

		/**
		 * Method that spawn the callback to the user
		 */
		void spawnCallback();

		/**
		 * Method that applys congestin control to the producer thread
		 *
		 * @param messagesReady Number of messages ready to send
		 */
		int congestionControl(long long messagesReady);

		/**
		 * Method to log a message with INFO level
		 *
		 * @param logMessage message to log
		 */
		void logIt (std::stringstream& logMessage);

		/**
		 * Method to end the thread
		 */
		void endThread();

		/**
		 * Method to stop the thread
		 */
		void stop();

		/**
		 * Default destructor
		 */
		virtual ~ActiveCallbackThread();
	};
}

#endif /* ACTIVECALLBACKTHREAD_H_ */
