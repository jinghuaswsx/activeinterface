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
 * Class that is the parent class that abstracts from the real producer/consumer
 * wrappers. This class stores all information about this connection to the brok
 */

#include "ActiveConnection.h"

using namespace ai;
using namespace log4cxx;
using namespace log4cxx::helpers;

LoggerPtr ActiveConnection::logger(Logger::getLogger("ActiveConnection"));


ActiveConnection::ActiveConnection() {
	id.clear();
	brokerURI="";
	destinationURI="";
    topic=false;
    selector="";
	requestReply=false;
	durable=false;
	type=1;
	persistent=false;
	clientAck=false;
	maxSizeQueue=10000;
	username="";
	password="";
	linkId.clear();
	sizePersistence=0;
	certificate="";
	consumerThreadFlag=false;
}

void ActiveConnection::initSSLSupport(){
	//if we find ssl:// string we can know that the connection is going to be
	//stablished by ssl protocol.
	std::stringstream logMessage;
	std::string sslHeader="ssl://";
	size_t found=getIpBroker().find(sslHeader);
	if (found!=std::string::npos){
		#ifdef WITH_SSL
			logMessage << "Connection to "<< getIpBroker() << "is using SSL with certificate "<<getCertificate();
			LOG4CXX_DEBUG(logger,logMessage.str().c_str());
			decaf::lang::System::setProperty( "decaf.net.ssl.trustStore", getCertificate() );
		#else
			logMessage << "ERROR: To use SSL Protocol you must compile with it!";
			LOG4CXX_DEBUG(logger,logMessage.str().c_str());
		#endif
	}
}

void ActiveConnection::loadPacketDesc(ActiveMessage& activeMessage){

	//clearing packet description
	activeMessage.clearPacketDesc();
	//Loading parameters and properties
	loadPacketDescParameters(activeMessage);
	loadPacketDescProperties(activeMessage);
}

void ActiveConnection::loadPacketDescParameters(ActiveMessage& activeMessage){

	std::string key;

	for (int it=0; it<activeMessage.getParametersSize();it++){
		Parameter* parameter=activeMessage.getParameter(it,key);
		switch (parameter->getType()){
		case ACTIVE_INT_PARAMETER:{
			activeMessage.pushInPacketDesc(ACTIVE_INT_PARAMETER);
		}
		break;
		case ACTIVE_REAL_PARAMETER:{
			activeMessage.pushInPacketDesc(ACTIVE_REAL_PARAMETER);
		}
		break;
		case ACTIVE_STRING_PARAMETER:{
			activeMessage.pushInPacketDesc(ACTIVE_STRING_PARAMETER);
		}
		break;
		case ACTIVE_BYTES_PARAMETER:{
			BytesParameter* bytesParameter=(BytesParameter*)parameter;
			activeMessage.pushInPacketDesc(ACTIVE_BYTES_PARAMETER);
			activeMessage.pushInPacketDesc(bytesParameter->getValue().size());
		}
		break;
		}
	}
}

void ActiveConnection::loadPacketDescProperties(ActiveMessage& activeMessage){

	std::string key;

	for (int it=0; it<activeMessage.getPropertiesSize();it++){
		Parameter* property=activeMessage.getProperty(it,key);
		switch (property->getType()){
		case ACTIVE_INT_PARAMETER:{
			activeMessage.pushInPacketDesc(ACTIVE_INT_PROPERTY);
		}
		break;
		case ACTIVE_REAL_PARAMETER:{
			activeMessage.pushInPacketDesc(ACTIVE_REAL_PROPERTY);
		}
		break;
		case ACTIVE_STRING_PARAMETER:{
			activeMessage.pushInPacketDesc(ACTIVE_STRING_PROPERTY);
		}
		break;
		}
	}
}

std::string ActiveConnection::getStringClientId(){
	std::stringstream strClientId;

	strClientId << getClientId();
	return strClientId.str();
}

void ActiveConnection::logIt (std::stringstream& logMessage){
	LOG4CXX_INFO(logger, logMessage.str().c_str());
	logMessage.str("");
}
