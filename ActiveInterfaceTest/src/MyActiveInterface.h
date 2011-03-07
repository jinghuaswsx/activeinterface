/*
 * MyActiveInterface.h
 *
 *  Created on: 29/06/2010
 *      Author: opernas
 */

#ifndef MYACTIVEINTERFACE_H_
#define MYACTIVEINTERFACE_H_

#include "ActiveInterface.h"
#include <apr_general.h>
#include <apr_thread_proc.h>
#include <apr_thread_cond.h>

class MyActiveInterface: public ActiveInterface{
public:
	int messagesSent;
	int messagesReceived;
	int rrMessagesReceived;
	bool queueFull;
	bool exitThread;

	MyActiveInterface();
	virtual ~MyActiveInterface();
	void onMessage(const ActiveMessage& activeMessage);
	void onConnectionInterrupted(std::string& connectionId);
	void onConnectionRestore(std::string& connectionId);
	void onQueuePacketDropped(const ActiveMessage& activeMessage);
	void onQueuePacketReady(std::string& connectionId);
	void onException(std::string& connectionId);

	void showActiveLinkList(std::list<ActiveLink*>& linkList);
	void showActiveConnectionList(std::list<ActiveConnection*>& connectionList);
	void showIntList(std::list<int>& intList);

	void sendMessages();
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

};

#endif /* MYACTIVEINTERFACE_H_ */
