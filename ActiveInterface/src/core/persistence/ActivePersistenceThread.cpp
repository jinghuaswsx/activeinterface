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
 */

#include "ActivePersistence.h"
#include "ActivePersistenceThread.h"
#include "../../utils/exception/ActiveException.h"

using namespace log4cxx;
using namespace log4cxx::helpers;

LoggerPtr ActivePersistenceThread::logger(Logger::getLogger("ActivePersistenceThread"));

ActivePersistenceThread::ActivePersistenceThread() {
	//initializing attributes
	rv=-1;
	mp=NULL;
	thd_arr=NULL;
	thd_attr=NULL;

	activePersistence=NULL;
	threadRunning=-1;
}

void ActivePersistenceThread::init (ActivePersistence& activePersistenceR){

	try {
		//initializing libraries
		//checking if the pointer to objects are ok
		activePersistence=&activePersistenceR;

		//apr_initialize();
		apr_pool_create(&mp, NULL);
		apr_threadattr_create(&thd_attr, mp);

		//initializing mutex and condition lock with mutex
	    apr_thread_mutex_create(activeSharedObject.getMutexPtr(), APR_THREAD_MUTEX_UNNESTED, mp);
	    apr_thread_cond_create(activeSharedObject.getCondPtr(), mp);

	}catch (...){
		throw ActiveException ("Failed communication with cms. Producer thread can not send.");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//// thread that sends
///////////////////////////////////////////////////////////////////////////////////////////
static void* APR_THREAD_FUNC persistenceThread(apr_thread_t *thd, void *data){

	if (data){
		ActivePersistence* myActivePersistence=((ActivePersistenceThread*)data)->getActivePersistence();
		ActiveSharedObject* mySharedObject=((ActivePersistenceThread*)data)->getActiveSharedObject();
		mySharedObject->setRunningThread();

		while(true){
			apr_thread_mutex_lock(mySharedObject->getMutex());
			while (mySharedObject->getMessagesReady() == 0 && !mySharedObject->getEndThread()) {
				apr_thread_cond_wait(mySharedObject->getCond(), mySharedObject->getMutex());
			}
			if (mySharedObject->getEndThread()){
				apr_thread_mutex_unlock(mySharedObject->getMutex());
				break;
			}

			apr_thread_mutex_unlock(mySharedObject->getMutex());

			if (mySharedObject->getMessagesReady()>0){
				//std::cout << "antes del enqueue persistence" << std::endl;
				myActivePersistence->enqueue();
				//std::cout << "despues del enqueue persistence" << std::endl;
			}

		}
		//std::cout << "Exited persistence thread!!!!!!!!" << std::endl;
		apr_thread_exit(thd, APR_SUCCESS);
		return NULL;
	}else{
		std::cout << "Failed communication with cms. Producer thread can not send." << std::endl;
		return NULL;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

int ActivePersistenceThread::runPersistenceThread (){

	threadRunning=apr_thread_create(&thd_arr, thd_attr, persistenceThread, (void*)this, mp);
	return 0;
}

void ActivePersistenceThread::newMessage(bool received){

	try{
		apr_thread_mutex_lock(activeSharedObject.getMutex());
		if (received){
			activeSharedObject.newMessage();
			//std::cout << "sumamos uno estos mensajes " << activeSharedObject.getMessagesReady() << std::endl;
		}else{
			activeSharedObject.messageSent();
			//std::cout << "restamos estos mensajes " << activeSharedObject.getMessagesReady() << std::endl;
		}
		apr_thread_cond_signal(activeSharedObject.getCond());
		apr_thread_mutex_unlock(activeSharedObject.getMutex());
	}catch (...){
		throw ActiveException ("Unknown exception with semaphore in persistence queue.");
	}
}

void ActivePersistenceThread::endThread(){
	apr_thread_mutex_lock(activeSharedObject.getMutex());
	activeSharedObject.setEndThread();
	apr_thread_cond_signal(activeSharedObject.getCond());
	apr_thread_mutex_unlock(activeSharedObject.getMutex());
}

void ActivePersistenceThread::logIt (std::stringstream& logMessage){
	LOG4CXX_INFO(logger, logMessage.str().c_str());
	logMessage.str("");
}

void ActivePersistenceThread::stop(){
	LOG4CXX_DEBUG (logger,"Stopping persistence thread");
	if (threadRunning==APR_SUCCESS){
		endThread();
	}
	LOG4CXX_DEBUG (logger,"Stopped persistence thread succesfully!.");
}

ActivePersistenceThread::~ActivePersistenceThread() {
	LOG4CXX_DEBUG (logger,"Exiting persistence thread.");
	if (threadRunning==APR_SUCCESS){
		endThread();
		apr_thread_join(&rv, thd_arr);
	}
	LOG4CXX_DEBUG (logger,"Exited persistence thread succesfully!.");
}
