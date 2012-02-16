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
 */

#include "ActiveProducerThread.h"
#include "../../utils/exception/ActiveException.h"

using namespace log4cxx;
using namespace log4cxx::helpers;

LoggerPtr ActiveProducerThread::logger(Logger::getLogger("ActiveProducerThread"));


ActiveProducerThread::ActiveProducerThread() {
	//initializing attributes
	threadRunning=-1;
	rv=-1;
	mp=NULL;
	thd_arr=NULL;
	thd_attr=NULL;
	activeConnection=NULL;
}

void ActiveProducerThread::init (	ActiveConnection* activeConnectionR ){

	try {
		//initializing libraries
		//apr_initialize();
		apr_pool_create(&mp, NULL);
		apr_threadattr_create(&thd_attr, mp);

		//checking if the pointer to objects are ok
		if ((activeConnectionR!=NULL)){
			activeConnection=activeConnectionR;
		}else{
			throw ActiveException ();
		}

		//initializing mutex and condition lock with mutex
	    apr_thread_mutex_create(activeSharedObject.getMutexPtr(), APR_THREAD_MUTEX_UNNESTED, mp);
	    apr_thread_cond_create(activeSharedObject.getCondPtr(), mp);

	}catch (...){
		throw ActiveException ("Failed communication with cms. Producer thread can not send.");
	}
}


/////////////////////////////////////////////////////////////////////////////////////////
// thread that sends
/////////////////////////////////////////////////////////////////////////////////////////
static void* APR_THREAD_FUNC sendThread(apr_thread_t *thd, void *data){

	if (data){

		ActiveConnection* myActiveProducer=((ActiveProducerThread*)data)->getActiveConnection();
		ActiveSharedObject* mySharedObject=((ActiveProducerThread*)data)->getActiveSharedObject();
		mySharedObject->setRunningThread();

		//while (!((ActiveProducerThread*)data)->getKillThread()){
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
				//myProducerThread->congestionControl(mySharedObject->getMessagesReady());
				myActiveProducer->send();
			}else{
				std::stringstream aux;
				aux << "Unblocked sending data, but no messages ready... what?" << std::endl;
				((ActiveProducerThread*)data)->logIt(aux);
			}

		}

		std::stringstream aux;
		aux << "Getting out of sending thread..." << std::endl;
		((ActiveProducerThread*)data)->logIt(aux);

		apr_thread_exit(thd, APR_SUCCESS);
		return NULL;
	}else{
		std::stringstream aux;
		aux << "Failed communication with cms. Producer thread can not send." << std::endl;
		((ActiveProducerThread*)data)->logIt(aux);
		return NULL;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////


int ActiveProducerThread::runSendThread (){

	threadRunning=apr_thread_create(&thd_arr, thd_attr, sendThread, (void*)this, mp);
	return threadRunning;
}

void ActiveProducerThread::newMessage(bool received){
	//std::cout << "antes del lock"<< std::endl;
	std::stringstream aux;
	apr_thread_mutex_lock(activeSharedObject.getMutex());
	if (received){
		activeSharedObject.newMessage();
		aux << "New message added to sent "<< activeSharedObject.getMessagesReady() << std::endl;
		logIt(aux);
	}else{
		activeSharedObject.messageSent();
		aux << "A message was substract "<< activeSharedObject.getMessagesReady() << std::endl;
		logIt(aux);
	}
	apr_thread_cond_signal(activeSharedObject.getCond());
	//std::cout << "despues y antes del unlock"<<std::endl;
	apr_thread_mutex_unlock(activeSharedObject.getMutex());

}

void ActiveProducerThread::endThread(){
	apr_thread_mutex_lock(activeSharedObject.getMutex());
	activeSharedObject.setEndThread();
	apr_thread_cond_signal(activeSharedObject.getCond());
	apr_thread_mutex_unlock(activeSharedObject.getMutex());
}

void ActiveProducerThread::logIt (std::stringstream& logMessage){
	LOG4CXX_DEBUG(logger, logMessage.str().c_str());
	logMessage.str("");
}

void ActiveProducerThread::stop(){
	LOG4CXX_DEBUG (logger,"Stopping producer thread");
	if (threadRunning==APR_SUCCESS){
		endThread();
	}
	LOG4CXX_DEBUG (logger,"Stopped producer thread succesfully!.");
}

ActiveProducerThread::~ActiveProducerThread() {
	LOG4CXX_DEBUG (logger,"Exiting producer thread.");
	if (threadRunning==APR_SUCCESS){
		endThread();
		apr_thread_join(&rv, thd_arr);
	}
	LOG4CXX_DEBUG (logger,"Exited producer thread succesfully!.");
}
