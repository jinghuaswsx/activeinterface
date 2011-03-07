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
 * Class that implements the intern queue for callbacks messages to the user.
 * It has the same properties and methods that the ActiveQueue used to stores
 * all messages that are producer to the broker.
 */

#ifndef ACTIVECALLBACKQUEUE_H_
#define ACTIVECALLBACKQUEUE_H_

#include <queue>

#include "../mutex/ActiveMutex.h"
#include "../callbacks/ActiveCallbackObject.h"

#include "log4cxx/logger.h"
#include "log4cxx/helpers/exception.h"

namespace ai{

	class ActiveCallbackQueue {
	private:
		/**
		 * Concurrent queue used to store messages before to be sent
		 */
		std::queue<ActiveCallbackObject> callbacksQueue;

		/**
		 * mutex to guard from concurrent access queue
		 */
		ActiveMutex accessQueue;

		/**
		 * Maximum size of the queue
		 */
		int maxQueueSize;

		/**
		 * Static var use by log4cxx for the logging system
		 */
		static log4cxx::LoggerPtr logger;

	public:

		/**
		 * Default constructor
		 */
		ActiveCallbackQueue();

		/**
		 * Method that initializes the queue
		 *
		 * @param maxQueueSize max size for the queue.
		 */
		void init (int maxQueueSize=0);

		/**
		 * Sets the max size for the queue
		 *
		 * @param maxQueueSizeR max size for the queue
		 */
		void setMaxSizeQueue(int maxQueueSizeR){ maxQueueSize=maxQueueSizeR;}

		/**
		 * Return the max size for the queue
		 *
		 * @return The maximun number of packets that the queue
		 * is going to store at the same time.
		 */
		unsigned int getMaxSizeQueue(){return maxQueueSize;}

		/**
		 * Method used to enqueue messages into the queue
		 *
		 * @param activeCallbackObjectR message to be stored into queue
		 * @return position of the message in the queue
		 *
		 * @throw ActiveException if something bad happens
		 */
		int enqueue(ActiveCallbackObject& activeCallbackObjectR) throw (ActiveException);

		/**
		 * Method used to dequeue a message from the queue
		 *
		 * @param activeCallbackObjectR reference to the message that is filled with the message pop from the queue
		 * @throw ActiveException if something bad happens.
		 */
		void dequeue(ActiveCallbackObject& activeCallbackObjectR) throw (ActiveException);

		/**
		 * Congestion control system. This method is developed to apply
		 * some sleeps to the thread that producer, to try to balance the
		 * producer/consumer cpu time per request.
		 * Is better to try to not drop packages, to not overload the
		 * queue.
		 */
		void congestionControl ();

		/**
		 * Default destructor
		 */
		virtual ~ActiveCallbackQueue();
	};
}

#endif /* ACTIVECALLBACKQUEUE_H_ */
