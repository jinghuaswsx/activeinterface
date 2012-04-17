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

#include "ActiveQueue.h"
#include "../../utils/exception/ActiveException.h"

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace ai;

//initializing logger
LoggerPtr ActiveQueue::logger(Logger::getLogger("ActiveQueue"));

void ActiveQueue::init (int maxQueueSizeR){
	//clearing all data of queue
	std::queue<ActiveMessage> messageQueueEmpty;
	std::swap( messageQueue, messageQueueEmpty );
	maxQueueSize=maxQueueSizeR;
	//setting the state to accepting
	working=true;
}

int ActiveQueue::enqueue(const ActiveMessage& activeMessage) throw (ActiveException){

	std::stringstream logMessage;
	try {
		//mutex to access the concurrent queue
		accessQueue.lock();
		//if the max value size queue is not defined
		//by the user is unlimited
		if (messageQueue.size()<getMaxSizeQueue() ||
			getMaxSizeQueue()==0){

			//inserting the message into the queue
			messageQueue.push(activeMessage);

			//unlocking the queue
			accessQueue.unlock();

			logMessage << "Enqueued message in position "<<messageQueue.size();
			LOG4CXX_DEBUG(logger,logMessage.str().c_str());

			congestionControl();

			//returning size
			return messageQueue.size();
		}else{
			//unlocking the queue
			accessQueue.unlock();
			return -1;
		}
	}catch (...){
		//unlocking the queue
		accessQueue.unlock();
		throw ActiveException ("POSSIBLE DATA LOSS! Error inserting in messages queue.");
	}
	return -1;
}

void ActiveQueue::dequeue (ActiveMessage& activeMessage) throw (ActiveException) {

	std::string key;
	std::stringstream logMessage;

	try{
		accessQueue.lock();
		if (!messageQueue.empty()){
			activeMessage.clone(messageQueue.front());
			messageQueue.pop();
		}
		//logMessage << "Dequeue message in position "<<messageQueue.size();
		//LOG4CXX_DEBUG(logger,logMessage.str().c_str());
		accessQueue.unlock();
	}catch (...){
		accessQueue.unlock();
		throw ActiveException ("Unknown exception getting message from the queue.");
	}

}

bool ActiveQueue::isFull(){
	if (getSizeQueue()==getMaxSizeQueue()){
		return true;
	}
	return false;
}

void ActiveQueue::congestionControl (){

	std::stringstream logMessage;
	if (maxQueueSize!=0){
		int pct=(messageQueue.size()*100)/maxQueueSize;
		if (pct<LEVEL1_PERCENT_MESSAGES_READY){
			logMessage << "Aplying congestion control controlling flow producer. "<< pct;
			apr_sleep (1);

		} else if (	pct>=LEVEL1_PERCENT_MESSAGES_READY  &&
					pct<LEVEL2_PERCENT_MESSAGES_READY){
			logMessage << "Aplying congestion control controlling flow producer. "<< pct;
			apr_sleep (1000);

		} else if (	pct>=LEVEL2_PERCENT_MESSAGES_READY  &&
					pct<LEVEL3_PERCENT_MESSAGES_READY){
			logMessage << "Aplying congestion control controlling flow producer. "<< pct;
			apr_sleep (5000);

		} else if (	pct>=LEVEL3_PERCENT_MESSAGES_READY  &&
					pct<LEVEL4_PERCENT_MESSAGES_READY){
			logMessage << "Aplying congestion control controlling flow producer. "<< pct;
			apr_sleep (10000);

		} else if (pct>=LEVEL4_PERCENT_MESSAGES_READY){
			logMessage << "Aplying congestion control controlling flow producer. "<< pct;
			apr_sleep (100000);

		} else{
			apr_sleep (0);
		}
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}
}

ActiveQueue::~ActiveQueue() {
}
