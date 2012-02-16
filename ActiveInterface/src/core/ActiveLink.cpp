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
 * Class that implements the association between a real connection to the broker
 * (for us ActiveConnection) and the services.
 * Link is an abstract entity that allows the user to do:
 *      - Abstraction about sending the same message to different connections. We have
 *      services that are compossed by one or more links, that allow the user to configure
 *      that all messages that are going to be sent by this service id, are  going to be
 *      distributed to more than one link (more than one connection).
 *      - Abstracion about default properties. Links allows the users to specify different
 *      properties for each one. For example, we could want to send to a service id
 *      that sends two messages (service has two links) and each links has its owns properties.
 *      It is very useful for selector and to configure default properties that the developer or
 *      doesn't need to be care about it because are going to be send automatically in all
 *      messages sent by this link.
 */

#include "ActiveLink.h"
#include "../utils/exception/ActiveException.h"

using namespace log4cxx;
using namespace log4cxx::helpers;

LoggerPtr ActiveLink::logger(Logger::getLogger("ActiveLink"));


ActiveLink::ActiveLink(std::string& idR, std::string& nameR, ActiveConnection* activeConnectionR){
	id=idR;
	name=nameR;
	activeConnection=activeConnectionR;
	propertiesList.clear();
}

void ActiveLink::insertIntProperty(std::string& key, int value){

	std::stringstream logMessage;

	try{
		propertiesList.insertIntParameter(key,value);
	}catch (ActiveException& ae){
		logMessage << ae.getMessage();
		logIt(logMessage);
	}
}

void ActiveLink::insertRealProperty(std::string& key,float value){
	std::stringstream logMessage;

	try{
		propertiesList.insertRealParameter(key,value);
	}catch (ActiveException& ae){
		logMessage << ae.getMessage();
		logIt(logMessage);
	}
}

void ActiveLink::insertStringProperty(std::string& key,std::string& value){
	std::stringstream logMessage;

	try{
		propertiesList.insertStringParameter(key,value);
	}catch (ActiveException& ae){
		logMessage << ae.getMessage();
		logIt(logMessage);
	}
}

void ActiveLink::clearProperties(){
	propertiesList.clear();
}

void ActiveLink::logIt (std::stringstream& logMessage){
	LOG4CXX_INFO(logger, logMessage.str().c_str());
	logMessage.str("");
}

ActiveLink::~ActiveLink() {
}
