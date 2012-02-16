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
 * Class that provides a list of parameter datatype in which parameters are
 * polymorphic.
 */

#include "ParameterList.h"
#include "../exception/ActiveException.h"

using namespace ai::utils;
using namespace log4cxx;
using namespace log4cxx::helpers;


LoggerPtr ParameterList::logger(Logger::getLogger("ParameterList"));

//clone a parameterList with new objects
void ParameterList::clone(const ParameterList& parameterListR){

	std::string key;

	std::string idAux=parameterListR.getId();
	setId(idAux);

	for (unsigned int it=0; it<parameterListR.size();it++){
		Parameter* parameter=parameterListR.get(it,key);
		switch (parameter->getType()){
		case ACTIVE_INT_PARAMETER:{
			IntParameter* intParameter=
					new IntParameter(((IntParameter*)parameter)->getValue());
			insertToMap(key,intParameter);
		}
		break;
		case ACTIVE_REAL_PARAMETER:{
			RealParameter* realParameter=
					new RealParameter(((RealParameter*)parameter)->getValue());
			insertToMap(key,realParameter);
		}
		break;
		case ACTIVE_STRING_PARAMETER:{
			StringParameter* stringParameter=
					new StringParameter(((StringParameter*)parameter)->getValue());
			insertToMap(key,stringParameter);
		}
		break;
		case ACTIVE_BYTES_PARAMETER:{
			BytesParameter* bytesParameter=
					new BytesParameter(((BytesParameter*)parameter)->getValue());
			insertToMap(key,bytesParameter);
		}
		break;
		}
	}
}

Parameter* ParameterList::get(std::string& key)const{

	std::stringstream logMessage;

	try{
		if(parametersMap.find(key) == parametersMap.end()){
			logMessage << "ParameterList::get I couldn't find my service id in parameters Map " << "property name: "<< key;
			throw ActiveException(logMessage);
		}else{
			return (parametersMap.find(key)->second);
		}
		return NULL;
	}catch (...){
		logMessage << "ParameterList::get. Something wrong happened getting value property "<<key;
		throw ActiveException(logMessage);
	}
}

IntParameter* ParameterList::getInt(std::string& key) const{
	std::stringstream logMessage;
	try{
		Parameter* parameter=get(key);
		if (parameter->getType()==ACTIVE_INT_PARAMETER){
			return (IntParameter*)parameter;
		}else{
			return NULL;
		}
	}catch (...){
		logMessage << "ParameterList::get. Something wrong happened getting value property "<<key;
		return NULL;
	}
}

RealParameter* ParameterList::getReal(std::string& key) const{
	std::stringstream logMessage;
	try{
		Parameter* parameter=get(key);
		if (parameter->getType()==ACTIVE_REAL_PARAMETER){
			return (RealParameter*)parameter;
		}else{
			return NULL;
		}
	}catch (...){
		logMessage << "ParameterList::get. Something wrong happened getting value property "<<key;
		return NULL;
	}
}

StringParameter* ParameterList::getString(std::string& key) const{
	std::stringstream logMessage;
	try{
		Parameter* parameter=get(key);
		if (parameter->getType()==ACTIVE_STRING_PARAMETER){
			return (StringParameter*)parameter;
		}else{
			return NULL;
		}
	}catch (...){
		logMessage << "ParameterList::get. Something wrong happened getting value property "<<key;
		return NULL;
	}
}

BytesParameter* ParameterList::getBytes(std::string& key) const{
	std::stringstream logMessage;
	try{
		Parameter* parameter=get(key);
		if (parameter->getType()==ACTIVE_BYTES_PARAMETER){
			return (BytesParameter*)parameter;
		}else{
			return NULL;
		}
	}catch (...){
		logMessage << "ParameterList::get. Something wrong happened getting value property "<<key;
		return NULL;
	}
}

Parameter* ParameterList::get(int index, std::string& key) const{

    int i = 0;
	std::stringstream logMessage;

	std::map <std::string,Parameter*>::const_iterator mapIterator;

	for(	mapIterator  = parametersMap.begin();
			mapIterator != parametersMap.end() && i!= index;
			mapIterator++, i++);

	if(mapIterator!=parametersMap.end()){
		key=(*mapIterator).first;
		return (*mapIterator).second;
	}else{
		return NULL;
	}
}

void ParameterList::insertToMap(std::string& name, Parameter* parameter){

	std::stringstream logMessage;
	try{
		std::pair<std::map<std::string,Parameter*>::iterator,bool> ret;
		ret=parametersMap.insert(std::map<std::string,Parameter*>::value_type(name,parameter));
		if (!ret.second){
			throw ActiveException(logMessage);
		}
	}catch (...){
			logMessage<<"ERROR inserting property. This property is not going to be used. Key: "<<name;
			throw ActiveException(logMessage);
	}
}


void ParameterList::insertIntParameter(std::string& key, int value){

	std::stringstream logMessage;

	try{
		IntParameter* intParameter=new IntParameter(value);
		insertToMap(key,intParameter);
	}catch (ActiveException& ae){
		logMessage << ae.getMessage();
		logIt(logMessage);
	}

}

void ParameterList::insertRealParameter(std::string& key,float value){
	std::stringstream logMessage;

	try{
		RealParameter* realParameter=new RealParameter(value);
		insertToMap(key,realParameter);
	}catch (ActiveException& ae){
		logMessage << ae.getMessage();
		logIt(logMessage);
	}
}

void ParameterList::insertStringParameter(std::string& key,const std::string& value){
	std::stringstream logMessage;

	try{
		StringParameter* stringParameter=new StringParameter(value);
		insertToMap(key,stringParameter);
	}catch (ActiveException& ae){
		logMessage << ae.getMessage();
		logIt(logMessage);
	}
}

void ParameterList::insertBytesParameter(std::string& key, std::vector<unsigned char>& value){
	std::stringstream logMessage;

	try{
		BytesParameter* bytesParameter=new BytesParameter(value);
		insertToMap(key,bytesParameter);
	}catch (ActiveException& ae){
		logMessage << ae.getMessage();
		logIt(logMessage);
	}
}

void ParameterList::insertParameter(	std::string& key,
										Parameter* parameter){
	std::stringstream logMessage;

	try{
		if (parameter){
			insertToMap(key,parameter);
		}else{
			throw ActiveException ("Parameter was null, Can not insert to map.");
		}
	}catch (ActiveException& ae){
		logMessage << ae.getMessage();
		logIt(logMessage);
	}
}

void ParameterList::deleteParameter (std::string& key){

	std::stringstream logMessage;
	try{
		std::map <std::string,Parameter*>::iterator it=parametersMap.find(key);
		if (it!=parametersMap.end()){
			Parameter* parameter=(*it).second;
			if (parameter!=NULL){
				delete (*it).second;
			}
			parametersMap.erase(key);
		}else{
			throw ActiveException ("Parameter was null, Can not delete from map.");
		}
	}catch (ActiveException& ae){
		logMessage << ae.getMessage();
		logIt(logMessage);
	}
}

void ParameterList::logIt (std::stringstream& logMessage){
	LOG4CXX_INFO(logger, logMessage.str().c_str());
	logMessage.str("");
}


ParameterList::~ParameterList() {
	clear();
}

void ParameterList::clear(){
	//deleting all parameters
	for( std::map <std::string,Parameter*>::iterator ii=parametersMap.begin();
		ii!=parametersMap.end(); ++ii){
		delete (*ii).second;
	}
	parametersMap.clear();

}
