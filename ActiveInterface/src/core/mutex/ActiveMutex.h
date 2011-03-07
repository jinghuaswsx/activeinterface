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
 * @section Class used to abstract the user the use of mutex with APR library.
 * If you have a variable of this type, you could apply lock, unlock primitives
 * in a easy way.
 */

#ifndef ACTIVEMUTEX_H_
#define ACTIVEMUTEX_H_

#include <apr_general.h>
#include <apr_thread_proc.h>

namespace ai{

	class ActiveMutex {
	private:
		/**
		 * APR mutex used to provides mutual exclusion accesing to the queue
		 */
		apr_thread_mutex_t* activeMutex;

		/**
		 * APR pool to manage threads
		 */
		apr_pool_t *mp;

	public:
		/**
		 *  Constructor that initializes apr library, create pool for managing threads and create thread
		 */
		ActiveMutex(){	apr_pool_create(&mp, NULL);
						apr_thread_mutex_create(&activeMutex,APR_THREAD_MUTEX_UNNESTED,mp);
		}

		/**
		 * Method that locks the access to concurrent active queue
		 */
		void lock(){apr_thread_mutex_lock(activeMutex);}

		/**
		 * Method that unlocks the access to concurrent active queue
		 */
		void unlock(){apr_thread_mutex_unlock(activeMutex);}

		/**
		 * Default destructor that terminate apr threads and free resources
		 */
		virtual ~ActiveMutex(){}
	};
}

#endif /* ACTIVEMUTEX_H_ */
