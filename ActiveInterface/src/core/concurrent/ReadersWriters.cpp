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
 * Class that gives an api to resolve readers writers concurrence problem
 *
 */

#include "ReadersWriters.h"

using namespace ai;

void ReadersWriters::readerLock(){
	semReaders.lock();
	counterReaders++;
	if (counterReaders==1){
		writerLock();
	}
	semReaders.unlock();
}

void ReadersWriters::readerUnlock(){
	semReaders.lock();
	counterReaders--;
	if (counterReaders==0){
		writerUnlock();
	}
	semReaders.unlock();
}

void ReadersWriters::writerLock(){
	semWriters.lock();
}

void ReadersWriters::writerUnlock(){
	semWriters.unlock();
}
