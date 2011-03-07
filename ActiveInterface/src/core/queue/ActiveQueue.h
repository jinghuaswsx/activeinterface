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
 * Class that implements the internal queue used by this library to stores
 * all messages that the user send.
 * ActiveMq-cpp blocks the thread if the user needs to send something but
 * the broker is down. With this queue, the user never is going to be blocked.
 *
 * The size of the queue is user defined by the XML configuration file with the
 * att maxsizequeue=X and with methods to allow inline producer/consumer creation.
 * This queue has a simple control congestion that apply some sleeps to the user
 * depending of the % of capacity of the queue that is filled up.
 */

#ifndef ACTIVEQUEUE_H_
#define ACTIVEQUEUE_H_

#include <queue>

#include "../mutex/ActiveMutex.h"
#include "../message/ActiveMessage.h"

#include "log4cxx/logger.h"
#include "log4cxx/helpers/exception.h"

using namespace ai::message;

namespace ai{

	class ActiveQueue {
	private:
		/**
		 * Concurrent queue used to store messages before to be sent
		 */
		std::queue<ActiveMessage> messageQueue;

		/**
		 * mutex to guard from concurrent access queue
		 */
		ActiveMutex accessQueue;

		/**
		 * congestion control flag
		 */
		bool congestionFlag;

		/**
		 * Maximum size of the queue
		 */
		int maxQueueSize;

		/**
		 * flags to know the state of the queue, if it is
		 * dropping messages or accepting it
		 */
		bool working;

		/**
		 * Static var use by log4cxx for the logging system
		 */
		static log4cxx::LoggerPtr logger;

	public:

		/**
		 * Default constructor
		 */
		ActiveQueue(){}

		/**
		 * Method that initializes the queue
		 *
		 * @param maxQueueSize max size for the queue.
		 */
		void init (int maxQueueSize);

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
		 * @param activeMessage message to be stored into queue
		 * @return position of the message in the queue
		 *
		 * @throws ActiveException if something bad happens.
		 */
		int enqueue(const ActiveMessage& activeMessage) throw (ActiveException);

		/**
		 * Method used to dequeue a message from the queue
		 *
		 * @param activeMessage reference to the message that is filled with the message pop from the queue
		 *
		 * @throws ActiveException if something bad happens.
		 */
		void dequeue(ActiveMessage& activeMessage) throw (ActiveException);

		/**
		 *	method to know if the queue is full or not
		 */
		bool isFull();

		/**
		 * Congestion control system. This method is developed to apply
		 * some sleeps to the thread that producer, to try to balance the
		 * producer/consumer cpu time per request.
		 * Is better to try to not drop packages, to not overload the
		 * queue.
		 */
		void congestionControl ();

		/**
		 * method to know if the queue is refusing messages because is full
		 * or not
		 */
		bool getWorkingState (){ return working;}

		/**
		 * Method to set the working state
		 */
		void setWorkingState (bool state){ working=state;}

		/**
		 * method to get the actual size of the queue
		 */
		long getSizeQueue (){return messageQueue.size();}

		/**
		 * Default destructor
		 */
		virtual ~ActiveQueue();
	};
}

#endif /* ACTIVEQUEUE_H_ */
