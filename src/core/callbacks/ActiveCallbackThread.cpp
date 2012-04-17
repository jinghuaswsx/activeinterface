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

#include <apr_general.h>
#include <apr_thread_proc.h>
#include <apr_thread_cond.h>

#include "ActiveCallbackThread.h"
#include "../../utils/exception/ActiveException.h"


using namespace log4cxx;
using namespace log4cxx::helpers;

LoggerPtr ActiveCallbackThread::logger(Logger::getLogger("ActiveCallbackThread"));


ActiveCallbackThread::ActiveCallbackThread() {
	//initializing attributes
	threadRunning=-1;
	rv=-1;
	mp=NULL;
	thd_arr=NULL;
	thd_attr=NULL;
	activeCallbackQueue=NULL;
	thd_main=NULL;
}

void ActiveCallbackThread::init (ActiveCallbackQueue& activeCallbackQueueR){

	try {
		//initializing libraries
		//apr_initialize();
		apr_pool_create(&mp, NULL);
		apr_threadattr_create(&thd_attr, mp);

		//initializing callback queue
		activeCallbackQueue=&activeCallbackQueueR;

		//initializing mutex and condition lock with mutex
	    apr_thread_mutex_create(activeSharedObject.getMutexPtr(), APR_THREAD_MUTEX_UNNESTED, mp);
	    apr_thread_cond_create(activeSharedObject.getCondPtr(), mp);

	}catch (...){
		throw ActiveException ("Failed communication with cms. Callback thread can not send.");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// thread that sends
/////////////////////////////////////////////////////////////////////////////////////////
static void* APR_THREAD_FUNC callbackThread(apr_thread_t *thd, void *data){

	if (data){

		ActiveCallbackThread* activeCallbackThread=((ActiveCallbackThread*)data);
		if (activeCallbackThread){
			ActiveCallbackSharedObject* mySharedObject=((ActiveCallbackThread*)data)->getActiveSharedObject();

			while(true){

				apr_thread_mutex_lock(mySharedObject->getMutex());
				while (mySharedObject->getCallbacksReady() == 0 && !mySharedObject->getEndThread()) {
					apr_thread_cond_wait(mySharedObject->getCond(), mySharedObject->getMutex());
				}
				if (mySharedObject->getEndThread()){
					break;
				}

				apr_thread_mutex_unlock(mySharedObject->getMutex());

				if (mySharedObject->getCallbacksReady()>0){
					//activeCallbackThread->congestionControl(mySharedObject->getCallbacksReady());
					activeCallbackThread->spawnCallback();
				}

			}
			apr_thread_exit(thd, APR_SUCCESS);
			return NULL;
		}else{
			throw ActiveException ("Failed communication with cms. Callback thread can not send.");
		}
	}else{
		throw ActiveException ("Failed communication with cms. Callback thread can not send.");
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

int ActiveCallbackThread::runCallbackThread (){

	threadRunning=apr_thread_create(&thd_main, thd_attr, callbackThread, (void*)this, mp);
	return threadRunning;
}

void ActiveCallbackThread::spawnCallback(){

	std::stringstream logMessage;

	//create the object that the thred will destroy it
	ActiveCallbackObject* activeCallbackObject=new ActiveCallbackObject();
	//dequeue from the queue and stores into callback object
	activeCallbackQueue->dequeue(*activeCallbackObject);
	//decreasing the number of callbacks to send
	newCallback(false);

	//spawning the thread to the user with data
	switch (activeCallbackObject->getType()){
	case ON_PACKET_DROPPED:{
		throwPacketDropped(*activeCallbackObject);
	}
	break;
	case ON_EXCEPTION:{
		throwOnException(*activeCallbackObject);
	}
	break;
	case ON_TRANSPORT_INTERRUPT:{
		throwOnTransportInterrupt(*activeCallbackObject);
	}
	break;
	case ON_TRANSPORT_RESUMED:{
		throwOnTransportResumed(*activeCallbackObject);
	}
	break;
	case ON_QUEUE_READY:{
		throwOnQueueReady(*activeCallbackObject);
	}
	break;
	}
}

void ActiveCallbackThread::newCallback(bool received){

	std::stringstream logMessage;

	if (getActiveSharedObject()){
		apr_thread_mutex_lock(activeSharedObject.getMutex());
		if (getActiveSharedObject()){
			if (received){
				activeSharedObject.newCallback();
			}else{
				activeSharedObject.callbackDone();
			}
		}else{
			//pondre trazas
		}
		apr_thread_cond_signal(activeSharedObject.getCond());
		apr_thread_mutex_unlock(activeSharedObject.getMutex());
	}else{
		logMessage << "ERROR: POSSIBLE DATA LOSS! New callback was not able to increment the count.";
		LOG4CXX_ERROR(logger,logMessage.str().c_str());
		throw ActiveException (logMessage);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////
// PAcket dropped callback
static void* APR_THREAD_FUNC onDroppedPacket(apr_thread_t *thd, void *data){

	//initializing vars
	ActiveCallbackObject* activeCallbackObject=NULL;
	if (data){
		activeCallbackObject=(ActiveCallbackObject*)data;
		if (activeCallbackObject){
			ActiveManager::getInstance()->onQueuePacketDropped(activeCallbackObject->getActiveMessage());
		}
	}
	//deleting object
	delete activeCallbackObject;
	//returning status
	apr_thread_exit(thd, APR_SUCCESS);
	//returning
	return NULL;
}

void ActiveCallbackThread::throwPacketDropped(ActiveCallbackObject& activeCallbackObject){

	std::stringstream logMessage;
	//creating the thread
	if (apr_thread_create(&thd_arr, thd_attr, onDroppedPacket, (void*)&activeCallbackObject, mp)==APR_SUCCESS){
		apr_thread_join(&rv,thd_arr);
	}
	if (rv!=APR_SUCCESS){
		logMessage << "ERROR: Thread couldn't be joined. Perhaps threads are being enqueued.";
		LOG4CXX_ERROR(logger,logMessage.str().c_str());
	}

}
///////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////
// On exception callback
static void* APR_THREAD_FUNC onException(apr_thread_t *thd, void *data){

	//initializing vars
	ActiveCallbackObject* activeCallbackObject=NULL;
	if (data){
		activeCallbackObject=(ActiveCallbackObject*)data;
		if (activeCallbackObject){
			ActiveManager::getInstance()->onException(activeCallbackObject->getConnectionId());
		}
	}
	//deleting object
	delete activeCallbackObject;
	//returning status
	apr_thread_exit(thd, APR_SUCCESS);
	//returning
	return NULL;
}

void ActiveCallbackThread::throwOnException(ActiveCallbackObject& activeCallbackObject){

	std::stringstream logMessage;
	//creating the thread
	if (apr_thread_create(&thd_arr, thd_attr, onException, (void*)&activeCallbackObject, mp)==APR_SUCCESS){
		apr_thread_join(&rv,thd_arr);
	}
	if (rv!=APR_SUCCESS){
		logMessage << "ERROR: Thread couldn't be joined. Perhaps threads are being enqueued.";
		LOG4CXX_ERROR(logger,logMessage.str().c_str());
	}

}
///////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////
// On transport interrupt callback
static void* APR_THREAD_FUNC onTransportInterrupt(apr_thread_t *thd, void *data){

	//initializing vars
	ActiveCallbackObject* activeCallbackObject=NULL;
	if (data){
		activeCallbackObject=(ActiveCallbackObject*)data;
		if (activeCallbackObject){
			ActiveManager::getInstance()->onConnectionInterruptCallback
					(activeCallbackObject->getConnectionId());
		}
	}
	//deleting object
	delete activeCallbackObject;
	//returning status
	apr_thread_exit(thd, APR_SUCCESS);
	//returning
	return NULL;
}

void ActiveCallbackThread::throwOnTransportInterrupt(ActiveCallbackObject& activeCallbackObject){

	std::stringstream logMessage;
	//creating the thread
	if (apr_thread_create(&thd_arr, thd_attr, onTransportInterrupt, (void*)&activeCallbackObject, mp)==APR_SUCCESS){
		apr_thread_join(&rv,thd_arr);
	}
	if (rv!=APR_SUCCESS){
		logMessage << "ERROR: Thread couldn't be joined. Perhaps threads are being enqueued.";
		LOG4CXX_ERROR(logger,logMessage.str().c_str());
	}

}
///////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////
// On transport reconnect callback
static void* APR_THREAD_FUNC onTransportResumed(apr_thread_t *thd, void *data){

	//initializing vars
	ActiveCallbackObject* activeCallbackObject=NULL;
	if (data){
		activeCallbackObject=(ActiveCallbackObject*)data;
		if (activeCallbackObject){
			ActiveManager::getInstance()->onConnectionRestoreCallback
								(activeCallbackObject->getConnectionId());
		}
	}
	//deleting object
	delete activeCallbackObject;
	//returning status
	apr_thread_exit(thd, APR_SUCCESS);
	//returning
	return NULL;
}

void ActiveCallbackThread::throwOnTransportResumed(ActiveCallbackObject& activeCallbackObject){

	std::stringstream logMessage;
	//creating the thread
	if (apr_thread_create(&thd_arr, thd_attr, onTransportResumed, (void*)&activeCallbackObject, mp)==APR_SUCCESS){
		apr_thread_join(&rv,thd_arr);
	}
	if (rv!=APR_SUCCESS){
		logMessage << "ERROR: Thread couldn't be joined. Perhaps threads are being enqueued.";
		LOG4CXX_ERROR(logger,logMessage.str().c_str());
	}

}
///////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////
// On queue is ready again after was full
static void* APR_THREAD_FUNC onQueueReady(apr_thread_t *thd, void *data){

	//initializing vars
	ActiveCallbackObject* activeCallbackObject=NULL;
	if (data){
		activeCallbackObject=(ActiveCallbackObject*)data;
		if (activeCallbackObject){
			ActiveManager::getInstance()->onQueuePacketReady
								(activeCallbackObject->getConnectionId());
		}
	}
	//deleting object
	delete activeCallbackObject;
	//returning status
	apr_thread_exit(thd, APR_SUCCESS);
	//returning
	return NULL;
}

void ActiveCallbackThread::throwOnQueueReady(ActiveCallbackObject& activeCallbackObject){

	std::stringstream logMessage;
	//creating the thread
	if (apr_thread_create(&thd_arr, thd_attr, onQueueReady, (void*)&activeCallbackObject, mp)==APR_SUCCESS){
		apr_thread_join(&rv,thd_arr);
	}
	if (rv!=APR_SUCCESS){
		logMessage << "ERROR: Thread couldn't be joined. Perhaps threads are being enqueued.";
		LOG4CXX_ERROR(logger,logMessage.str().c_str());
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////

void ActiveCallbackThread::logIt (std::stringstream& logMessage){
	LOG4CXX_INFO(logger, logMessage.str().c_str());
	logMessage.str("");
}

void ActiveCallbackThread::endThread(){
	apr_thread_mutex_lock(activeSharedObject.getMutex());
	activeSharedObject.setEndThread();
	apr_thread_cond_signal(activeSharedObject.getCond());
	apr_thread_mutex_unlock(activeSharedObject.getMutex());
}

void ActiveCallbackThread::stop(){
	LOG4CXX_DEBUG (logger,"Stopping callback thread");
	endThread();
	LOG4CXX_DEBUG (logger,"Stopped callback thread succesfully!.");
}

ActiveCallbackThread::~ActiveCallbackThread() {
	if (threadRunning==APR_SUCCESS){
		LOG4CXX_DEBUG (logger,"Exiting callback thread.");
		apr_thread_join(&rv, thd_main);
		LOG4CXX_DEBUG (logger,"Exited callback thread succesfully!.");
	}
}
