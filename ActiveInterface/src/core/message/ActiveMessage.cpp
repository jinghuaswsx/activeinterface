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
 * Class that implements the main message that we are going to send/receive
 * to/from the library.
 * This ActiveMessage gives the user an upper abstracion layer about JMS messages,
 * giving the possibility to send different data types in the same message only using
 * its upper abstraction methods.
 * This message is built like a heterogeneus map, or a map that stores polymorphic types.
 * Every entry of the map of this message has a key but is possible to loop through
 * all data stored in the map.
 */

#include "ActiveMessage.h"
#include "../../utils/exception/ActiveException.h"

using namespace ai::message;

using namespace log4cxx;
using namespace log4cxx::helpers;


LoggerPtr ActiveMessage::logger(Logger::getLogger("ActiveMessage"));


ActiveMessage::ActiveMessage() {

	//flags initialization
	serviceId.clear();
	linkId.clear();
	connectionId.clear();
	timeToLive=0;
	priority=0;
	requestReply=false;
	textMessage=false;

	//packet description
	packetDesc.reserve(MAX_PARAMETERS);
	packetDesc.clear();

	//clearing lists
	clearParameters();
	clearProperties();
}


ActiveMessage::ActiveMessage(const ActiveMessage& activeMessageR) {
	clone(activeMessageR);
}

void ActiveMessage::clone(const ActiveMessage& activeMessageR) throw (ActiveException){
	try {
		////////////////////////////////////////////////////
		//copying all data
		setServiceId(const_cast<std::string&>(activeMessageR.getServiceId()));
		setLinkId(const_cast<std::string&>(activeMessageR.getLinkId()));
		setConnectionId(const_cast<std::string&>(activeMessageR.getConnectionId()));
		setTimeToLive(activeMessageR.getTimeToLive());
		setPriority(activeMessageR.getPriority());
		setRequestReply(activeMessageR.getRequestReply());
		//setting text
		setText(const_cast<std::string&>(activeMessageR.getText()));
		if (activeMessageR.isTextMessage()){
			textMessage=true;
		}else{
			textMessage=false;
		}
		//Setting correlation Id
		setCorrelationId(const_cast<std::string&>(activeMessageR.getCorrelationId()));
		//copying packet description
		packetDesc=activeMessageR.getPacketDesc();
		//cloning lists
		cloneLists(activeMessageR);
		//cloning destination reply to
		activeDestination.clone(activeMessageR.getDestination());

	}catch (...){
		throw ActiveException ("Exception cloning data from queue.");
	}
}

void ActiveMessage::cloneLists (const ActiveMessage& activeMessageR)
	throw (ActiveException){

	std::stringstream logMessage;
	try{
		parameterList.clone(activeMessageR.getParameterList());
		propertiesList.clone(activeMessageR.getPropertiesList());
	}catch (ActiveException& ae){
		logMessage << "Cloning lists error.";
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
		throw ActiveException (logMessage.str().c_str());
	}
}

void ActiveMessage::cloneDestination (const cms::Destination* destinationR){
	activeDestination.clone(destinationR);
}

void ActiveMessage::insertIntParameter(std::string& key, int value){

	std::stringstream logMessage;

	try{
		parameterList.insertIntParameter(key,value);
	}catch (ActiveException& ae){
		logMessage << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}

}

void ActiveMessage::insertRealParameter(std::string& key,float value){
	std::stringstream logMessage;

	try{
		parameterList.insertRealParameter(key,value);
	}catch (ActiveException& ae){
		logMessage << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}
}

void ActiveMessage::insertStringParameter(std::string& key,std::string& value){
	std::stringstream logMessage;

	try{
		parameterList.insertStringParameter(key,value);
	}catch (ActiveException& ae){
		logMessage << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}
}

void ActiveMessage::insertBytesParameter(std::string& key, std::vector<unsigned char>& value){
	std::stringstream logMessage;

	try{
		parameterList.insertBytesParameter(key,value);
	}catch (ActiveException& ae){
		logMessage << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}
}

void ActiveMessage::deleteParameter(std::string& key){
	std::stringstream logMessage;
	try{
		parameterList.deleteParameter(key);
	}catch (ActiveException& ae){
		logMessage << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}
}

void ActiveMessage::insertIntProperty(std::string& key, int value){

	std::stringstream logMessage;

	try{
		propertiesList.insertIntParameter(key,value);
	}catch (ActiveException& ae){
		logMessage << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}

}

void ActiveMessage::insertRealProperty(std::string& key,float value){
	std::stringstream logMessage;

	try{
		propertiesList.insertRealParameter(key,value);
	}catch (ActiveException& ae){
		logMessage << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}
}

void ActiveMessage::insertStringProperty(std::string& key,const std::string& value){
	std::stringstream logMessage;

	try{
		propertiesList.insertStringParameter(key,value);
	}catch (ActiveException& ae){
		logMessage << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}
}

void ActiveMessage::deleteProperty(std::string& key){
	std::stringstream logMessage;
	try{
		propertiesList.deleteParameter(key);
	}catch (ActiveException& ae){
		logMessage << ae.getMessage();
		LOG4CXX_DEBUG(logger,logMessage.str().c_str());
	}
}

IntParameter* ActiveMessage::getIntParameter (std::string& key) const
	throw (ActiveException){

	IntParameter* intParameter=parameterList.getInt(key);
	if (intParameter){
		return intParameter;
	}else{
		throw ActiveException("Parameter is not int");
	}
}

RealParameter* ActiveMessage::getRealParameter(std::string& key)const
	throw (ActiveException){

	RealParameter* realParameter=parameterList.getReal(key);
	if (realParameter){
		return realParameter;
	}else{
		throw ActiveException("Parameter is not real");
	}

}

StringParameter* ActiveMessage::getStringParameter(std::string& key) const
	throw (ActiveException){

	StringParameter* stringParameter=parameterList.getString(key);
	if (stringParameter){
		return stringParameter;
	}else{
		throw ActiveException("Parameter is not string");
	}
}

BytesParameter* ActiveMessage::getBytesParameter(std::string& key) const
	throw (ActiveException){

	BytesParameter* bytesParameter=parameterList.getBytes(key);
	if (bytesParameter){
		return bytesParameter;
	}else{
		throw ActiveException("Parameter is not bytes");
	}
}

IntParameter* ActiveMessage::getIntProperty(std::string& key) const
	throw (ActiveException){

	IntParameter* intProperty=propertiesList.getInt(key);
	if (intProperty){
		return intProperty;
	}else{
		throw ActiveException("Property is not int");
	}
}

RealParameter* ActiveMessage::getRealProperty(std::string& key) const
	throw (ActiveException){

	RealParameter* realProperty=propertiesList.getReal(key);
	if (realProperty){
		return realProperty;
	}else{
		throw ActiveException("Property is not real");
	}
}

StringParameter* ActiveMessage::getStringProperty(std::string& key)const
	throw (ActiveException){

	StringParameter* stringProperty=propertiesList.getString(key);
	if (stringProperty){
		return stringProperty;
	}else{
		throw ActiveException("Property is not string");
	}
}

void ActiveMessage::clear(){

	//flags initialization
	serviceId.clear();
	linkId.clear();
	connectionId.clear();
	timeToLive=0;
	priority=0;
	requestReply=false;

	clearParameters();
	clearProperties();
	packetDesc.clear();

	activeDestination.clear();
}

void ActiveMessage::clearParameters(){
	parameterList.clear();
}

void ActiveMessage::clearProperties(){
	propertiesList.clear();
}

void ActiveMessage::logIt (std::stringstream& logMessage){
	LOG4CXX_INFO(logger, logMessage.str().c_str());
	logMessage.str("");
}

ActiveMessage::~ActiveMessage() {
}
