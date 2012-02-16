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
 * Class that implements interface with application. Is the entering point to the library, and
 * gives users all methods to interact with the wrapper.
 */

#include "ActiveCallbackQueue.h"
#include "../../utils/exception/ActiveException.h"

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace ai;

//initializing logger
LoggerPtr ActiveCallbackQueue::logger(Logger::getLogger("ActiveCallbackQueue"));


ActiveCallbackQueue::ActiveCallbackQueue() {
}

void ActiveCallbackQueue::init (int maxQueueSizeR){
	//clearing all data of queue
	std::queue<ActiveCallbackObject> callbacksQueueEmpty;
	std::swap( callbacksQueue, callbacksQueueEmpty );
	maxQueueSize=maxQueueSizeR;
}

int ActiveCallbackQueue::enqueue(ActiveCallbackObject& activeCallbackObjectR)
	throw (ActiveException){

	std::stringstream logMessage;
	try {
		//mutex to access the concurrent queue
		accessQueue.lock();
		//if the max value size queue is not defined
		//by the user is unlimited
		if (callbacksQueue.size()<getMaxSizeQueue() ||
			getMaxSizeQueue()==0){

			callbacksQueue.push(activeCallbackObjectR);

			//mutex to access the concurrent queue
			accessQueue.unlock();

			//logMessage << "Enqueued callback in position "<<callbacksQueue.size();
			//LOG4CXX_INFO(logger,logMessage.str().c_str());

			congestionControl();

			return callbacksQueue.size();
		}else{
			//mutex to access the concurrent queue
			accessQueue.unlock();
			return -1;
		}
	}catch (...){
		//mutex to access the concurrent queue
		accessQueue.unlock();
		throw ActiveException ("POSSIBLE DATA LOSS! Error inserting in messages queue.");
	}
	return -1;
}

void ActiveCallbackQueue::dequeue (ActiveCallbackObject& activeCallbackObjectR)
	throw (ActiveException){

	std::string key;
	std::stringstream logMessage;

	try{
		accessQueue.lock();
		if (!callbacksQueue.empty()){
			activeCallbackObjectR.clone(callbacksQueue.front());
			callbacksQueue.pop();
		}
		//logMessage << "Dequeue callback in position "<<callbacksQueue.size();
		//LOG4CXX_INFO(logger,logMessage.str().c_str());
		accessQueue.unlock();
	}catch (...){
		accessQueue.unlock();
		throw ActiveException ("Unknown exception getting message from the queue.");
	}
}

void ActiveCallbackQueue::congestionControl (){

	std::stringstream logMessage;
	if (maxQueueSize!=0){
		int pct=(callbacksQueue.size()*100)/maxQueueSize;
		logMessage << "Aplying congestion control controlling flow producer. "<< pct;
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
		if (pct<LEVEL1_PERCENT_MESSAGES_READY){
			apr_sleep (0);

		} else if (	pct>=LEVEL1_PERCENT_MESSAGES_READY  &&
					pct<LEVEL2_PERCENT_MESSAGES_READY){
			apr_sleep (1000);

		} else if (	pct>=LEVEL2_PERCENT_MESSAGES_READY  &&
				pct<LEVEL3_PERCENT_MESSAGES_READY){
			apr_sleep (5000);

		} else if (	pct>=LEVEL3_PERCENT_MESSAGES_READY  &&
					pct<LEVEL4_PERCENT_MESSAGES_READY){
			apr_sleep (10000);

		} else if (pct>=LEVEL4_PERCENT_MESSAGES_READY){
			apr_sleep (100000);

		} else{
			apr_sleep (0);
		}
	}
}

ActiveCallbackQueue::~ActiveCallbackQueue() {
}
