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
 * Class that implements the persistence module of this library. Persistence file is
 * implemented using boost-serialization library and is written to a file per
 * connection to broker.
 * Each connection will be persisted in file persistence_file_1 for connection 1
 *
 */

#ifndef ACTIVEPERSISTENCE_H_
#define ACTIVEPERSISTENCE_H_

#include <iostream>
#include <fstream>

#include "../ActiveConnection.h"
#include "../mutex/ActiveMutex.h"
#include "../message/ActiveMessage.h"
#include "ActivePersistenceThread.h"

#include "log4cxx/logger.h"
#include "log4cxx/helpers/exception.h"

namespace ai{

	class ActivePersistence {
	private:

		//connection id referred
		std::stringstream dataFilename;

		//name of the file that controls the last sent
		std::stringstream controlFilename;

		//mutex to access to files
		ActiveMutex persistenceMutex;

		//counter to know which is the last packet sent
		long lastSent;

		//counter to know the last packet that
		//was inserted into the queue
		long lastEnqueue;

		//counter to know which is the last registry
		//serialized into file
		long lastWrote;

		//var to know if we have to use persistence or not
		bool initialized;

		//var to know which is the max size of messages
		//that will be stored before delete persistence file
		long sizePersistence;

		//static var for logger
		static log4cxx::LoggerPtr logger;

		//flag to know if we are in recovery mode
		bool recoveryMode;

		//class that implements the persistence thread
		//that puts new messages from persistence file
		//into the queue
		ActivePersistenceThread activePersistenceThread;

		//pointer to connection
		ActiveConnection* activeConnection;

		/**
		 * position in file to dont have to iterate over
		 * all entries in file
		 */
		long long positionInFile;

		/**
		 * Method that increase the number of messages sent
		 *
		 * @throws ActiveException if something bad happens.
		 */
		void increaseSent() throw (ActiveException);

		/**
		 * Method that sets the next position to send. Use the
		 * lastSent to set the position to it.
		 */
		void setPositionToSend();

		/**
		 * Method that gets the nextMessage to the actual position
		 * of the cursor positionInFile
		 *
		 * @param activeMessageR message in which message read from file is
		 * going to be stored.
		 */
		bool getNextMessage(ActiveMessage& activeMessageR);

	public:

		/**
		 * Default constructor
		 */
		ActivePersistence();

		/**
		 * Method that initializes the persistence
		 *
		 * @param activeConnection parameter to have a reference
		 * to the connection in which the persistence is running
		 */
		void init(	ActiveConnection& activeConnection);

		/**
		 * Method that is invoked when library goes up to know if
		 * we have messages stored in persistence file that was not
		 * sent before.
		 */
		void crashRecovery();

		/**
		 *	Method that gets the last sent from control_file file.
		 *
		 *	@return long The number of the last message sent
		 */
		long getLastSentFromFile();

		/**
		 *	Method that returns the number of messages serialized
		 *
		 *	@return the number of messages that are serialized into the file.
		 */
		long getNumberMessagesSerialized();

		/**
		 * method that increase the number of the last
		 * message sent and increase the number on
		 * the file.
		 *
		 * @param dequeueInRecovery parameter that specifies if the message
		 * was dequeue when the connection was in persitence state or not.
		 */
		void oneMoreSent(bool dequeueInRecovery);

		/**
		 *	Method that is invoked by the thread and enqueue data into the
		 *	connection queue.
		 */
		void enqueue();

		/**
		 *	Method that gets the message stored in file in position lastEnqueued.
		 *
		 *	@param lastEnqueued last message that was enqueued
		 *	@param activeMessageR message in which is stored the message read from file.
		 *
		 *	@return true if message exists in this posision.
		 */
		//bool getMessageAt(long lastEnqueued, ActiveMessage& activeMessageR);

		/**
		 *	Method that increment the conditional lock of the thread or decrement.
		 *
		 *	@param received if is false, message -- else message++
		 */
		void newMessage(bool received);

		/**
		 *	Method that increment the lastEnqueued variable.
		 */
		void oneMoreEnqueued();

		/**
		 *	Method that returns the size of the persistence file.
		 *
		 *	@return size of the persistence file
		 */
		long getSizePersistenceFile();

		/**
		 *	Method that is invoked every sent and that checks if the number of
		 *	messages that are stored in persistence file are more than the specified
		 *	If messagesStored>persistence then the persistence file is truncated.
		 *
		 *	@throws ActiveException if something bad happens
		 */
		void rollFile() throw (ActiveException);

		/**
		 *	Method that serializes the message to the persistence file.
		 *
		 *	@param activeMessage message that is going to be serialized.
		 */
		int serialize(ActiveMessage& activeMessage);

		/**
		 *	Method that deserializes a message from the file
		 *
		 *	@param activeMessage message that is going to be deserialized from file
		 */
		void deserialize (ActiveMessage& activeMessageR);

		/**
		 * Method that checks if the persistence is enabled by configuration
		 */
		bool isEnabled();

		/**
		 * Method that start the persistence mode. It is invoked when a message
		 * is rejected by the queue.
		 */
		int startRecoveryMode();

		/**
		 *	Method that reset control and persistence files.
		 */
		void resetFiles();

		/**
		 * stops the persistence thread
		 */
		void stopThread();

		/**
		 *	Default destructor
		 */
		virtual ~ActivePersistence();

		/////////////////////////////////////////////////////////////////
		//Setters & getters
		void setDataFilename (std::string& dataFilenameR){dataFilename<<dataFilenameR;}
		std::string getDataFilename (){return dataFilename.str();}
		bool getInitialized(){return initialized;}
		void setInitialized(bool initializedR){ initialized=initializedR;}
		long getSizePersistence(){return sizePersistence;}
		void setSizePersistence(long sizePersistenceR){sizePersistence=sizePersistenceR;}
		bool getRecoveryMode (){return recoveryMode;}
		void setRecoveryMode(bool recoveryModeR){recoveryMode=recoveryModeR;}
	};
}

#endif /* ACTIVEFILE_H_ */
