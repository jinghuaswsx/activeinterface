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

#include "ActiveConsumerThread.h"

using namespace ai;

using namespace log4cxx;
using namespace log4cxx::helpers;

LoggerPtr ActiveConsumerThread::logger(Logger::getLogger("ActiveConsumerThread"));


ActiveConsumerThread::ActiveConsumerThread() {
	//initializing attributes
	threadRunning=-1;
	rv=-1;
	mp=NULL;
	thd_arr=NULL;
	thd_attr=NULL;
	activeConnection=NULL;
}

void ActiveConsumerThread::init ( ActiveConnection* activeConnectionR ){

	try {
		//initializing libraries
		//apr_initialize();
		apr_pool_create(&mp, NULL);
		apr_threadattr_create(&thd_attr, mp);

		//checking if the pointer to objects are ok
		if ((activeConnectionR!=NULL)){
			activeConnection=activeConnectionR;
		}else{
			throw ActiveException ("ERROR: No consumer associated with consuming thread.");
		}

	}catch (...){
		throw ActiveException ("Failed communication with connection. Consumer thread can not start.");
	}
}

static void* APR_THREAD_FUNC onMessageThread(apr_thread_t *thd, void *data){

	if (data){
		ActiveConnection* myActiveConsumer=((ActiveConsumerThread*)data)->getActiveConnection();
		//Starting to consume messages
		myActiveConsumer->onReceive();
		//exiting of thread
		apr_thread_exit(thd, APR_SUCCESS);
	}
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////


int ActiveConsumerThread::runThread (){

	threadRunning=apr_thread_create(&thd_arr, thd_attr, onMessageThread, (void*)this, mp);
	if (threadRunning!=0){
		throw ActiveException ("Error: Consumer Thread was not initialized well.");
	}
	return threadRunning;

}

ActiveConsumerThread::~ActiveConsumerThread() {
	if (threadRunning==APR_SUCCESS){
		LOG4CXX_DEBUG (logger,"Exiting consumer thread.");
		apr_thread_join(&rv, thd_arr);
		LOG4CXX_DEBUG (logger,"Consumer thread exited!.")
	}
}
