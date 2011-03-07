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
 * Class that gives an api to resolve readers writers concurrence problem
 *
 */


#ifndef READERSWRITERS_H_
#define READERSWRITERS_H_

#include "../mutex/ActiveMutex.h"

namespace ai{

	class ReadersWriters {
	private:
		/**
		 * Mutex to control the access to connections Map to read
		 */
		ActiveMutex semReaders;

		/**
		 * Mutex to control the access to connections Map to write
		 */
		ActiveMutex semWriters;

		/**
		 * Number to control the number of readers to services
		 */
		long counterReaders;

	public:
		/**
		 * Default constructor
		 */
		ReadersWriters(){counterReaders=0;}

		/**
		 * Locks the reader
		 */
		void readerLock();

		/**
		 * Unlocks the reader
		 */
		void readerUnlock();

		/**
		 * Locks the writer
		 */
		void writerLock();

		/**
		 * Unlock the writer
		 */
		void writerUnlock();

		/**
		 * Destructor by default
		 */
		virtual ~ReadersWriters(){};
	};
}

#endif /* READERSWRITERS_H_ */
