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
 * Class that encapsulate the cms::destination class
 *
 */

#include "ActiveDestination.h"

using namespace ai;

ActiveDestination::ActiveDestination(){
	replyTo=NULL;
}

void ActiveDestination::clone(const ActiveDestination& activeDestinationR){
	if (activeDestinationR.getReplyTo()){
		setReplyTo(activeDestinationR.getReplyTo()->clone());
	}else{
		setReplyTo(NULL);
	}
}

void ActiveDestination::clone (const cms::Destination* replyToR){
	if (replyToR){
		replyTo=replyToR->clone();
	}else{
		replyTo=NULL;
	}
}

void ActiveDestination::setReplyTo(const cms::Destination* replyToR){
	replyTo=replyToR;
}

void ActiveDestination::copy(const ActiveDestination& activeDestinationR){
	setReplyTo(activeDestinationR.getReplyTo());
}

ActiveDestination::~ActiveDestination(){
	if (replyTo){
		delete replyTo;
	}else{
		replyTo=NULL;
	}
}
