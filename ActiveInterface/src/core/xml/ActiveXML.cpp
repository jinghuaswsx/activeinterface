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
 * Class that implements the reading of the xml configuration file, if it is used
 * to startup the application. It is a wrapper about ticpp library.
 */

#include "ActiveXML.h"
#include "../wrapper/ActiveProducer.h"
#include "../wrapper/ActiveConsumer.h"
#include "../../utils/exception/ActiveException.h"

using namespace log4cxx;
using namespace log4cxx::helpers;


LoggerPtr ActiveXML::logger(Logger::getLogger("ActiveXML"));


ActiveXML::ActiveXML() {
	doc=NULL;
}

void ActiveXML::init(const std::string& configurationFile) throw (ActiveException){

	std::stringstream logMessage;
	try{
		logMessage << "Reading configuration file "<<configurationFile;
		logIt(logMessage);
		doc=new ticpp::Document (configurationFile);
		doc->LoadFile();
	}catch (ticpp::Exception& ex){
		logMessage << "ERROR: Error trying to read "<< configurationFile << ".";
		throw ActiveException(logMessage.str());
	}catch (...){
		throw ActiveException("ERROR: Unknown exception reading configuration document.");
	}
}

void ActiveXML::loadConnections() throw (ActiveException){

	int iteratorCounter=0;

	std::stringstream logMessage;

	try{
		ticpp::Element* connectionlist = doc->FirstChildElement("connectionslist");
		ticpp::Iterator<ticpp::Element> connectionsIterator;

		for (connectionsIterator = connectionsIterator.begin(connectionlist); connectionsIterator != connectionsIterator.end(); connectionsIterator++){

			//initialize data
			std::string id="";
			std::string ipBroker="";
			int type=0;
			std::string destination="";
			bool persistent=false;
			bool topic=false;
			bool durable=false;
			std::string selector="";
			bool clientAck=false;
			int maxSizeQueue=0;
			std::string username="";
			std::string password="";
			std::string clientId="";
			int persistence=0;
			std::string certificate="";
			///////////////

			try {
				iteratorCounter++;
				ticpp::Element* connection = connectionsIterator.Get();

				logMessage << "-----------CONNECTION -------------------";
				logIt(logMessage);

				//getting attributes
				getString(connection,"id",id);
				getString(connection,"ipbroker",ipBroker);
				getInt(connection,"type",type);
				getString(connection,"destination",destination);
				//persistent is only for producers
				if (isConsumer(type)){
					getBool(connection,"persistent",persistent,false);
				}else{
					getBool(connection,"persistent",persistent);
				}
				getBool(connection,"topic",topic);
				//selector is only for consumers
				getString(connection,"selector",selector,isConsumer(type));
				//durable connection or not if is a consumer and is for topic
				if (isConsumer(type) && !type){
					getBool(connection,"durable",durable);
				}else{
					getBool(connection,"durable",durable,false);
				}
				//managing ack mode
				getBool(connection,"clientack",clientAck,false);
				getInt(connection,"maxsizequeue",maxSizeQueue,false);
				getString(connection,"username",username,false);
				getString(connection,"password",password,false);
				if (isConsumer(type) && topic){
					getString(connection,"clientid",clientId);
				}else{
					getString(connection,"clientid",clientId,false);
				}
				getInt(connection,"persistence",persistence,false);
				getString(connection,"certificate",certificate,false);

				/////////////////////////////////////////////////////////////////////////////////////
				//saving active link to map
				if (ActiveManager::getInstance()->
						saveConnection(	id, ipBroker, type, topic, destination,
										persistent,selector,durable,clientAck,maxSizeQueue,
										username,password,clientId,persistence,certificate)){

					logMessage << "Loaded connection " << id << " OK! ";
					logIt(logMessage);

				}else{
					logMessage << "Loaded connection " << id << " ERROR! ";
					logIt(logMessage);
				}
				//////////////////////////////////////////////////////////////////////////////////////

			}catch (ActiveException& ae){
				logMessage << "ERROR: Something happened initializing connection data " << iteratorCounter << ". Going to next connection.";
				logIt(logMessage);
			}
		}
		logMessage << "---------------------------------------";
		logIt(logMessage);
	}catch( ... ){
		logMessage << "ERROR Loading connections list, check XML configuration file";
		throw ActiveException(logMessage.str());
	}
}

void ActiveXML::loadActiveLinks() throw (ActiveException){

	int iteratorCounter=0;
	std::stringstream logMessage;

	//initializing activelink
	ActiveLink* activeLink=NULL;
	ActiveConnection* activeConnection=NULL;

	try{
		ticpp::Element* links = doc->FirstChildElement("linkslist");
		ticpp::Iterator<ticpp::Element> linksIterator;

		for (linksIterator = linksIterator.begin(links); linksIterator != linksIterator.end(); linksIterator++){

			//initialize
			std::string name="";
			std::string connectionId="";
			std::string linkId="";
			///////////////

			try {
				iteratorCounter++;
				ticpp::Element* link = linksIterator.Get();

				logMessage << "-----------LINK -------------------";
				logIt(logMessage);

				getString (link,"id",linkId);
				getString(link,"name",name);
				getString (link,"connectionid",connectionId);

				activeConnection=ActiveManager::getInstance()->getConnection(connectionId);

				activeLink=new ActiveLink(linkId,name,activeConnection);

				loadProperties(activeLink, link);

				ActiveManager::getInstance()->insertInLinksMap(linkId,activeLink);

				//we set the link id associated with this connection
				if (activeConnection){
					activeConnection->setLinkId(linkId);
				}else{
					logMessage.str("");
					logMessage << "ERROR Reading XML File. Link " << linkId;
					logMessage << " error, connection does not exists " << connectionId;
					LOG4CXX_DEBUG (logger,logMessage.str().c_str());
					logMessage.str("");
				}

			}catch (ActiveException& ae){
				logMessage << ae.getMessage();
				logIt(logMessage);
			}
		}
		logMessage << "------------------------------";
		logIt(logMessage);
	}catch (ticpp::Exception& ex){
		throw ActiveException(ex.what());
	}catch (ActiveException& ae){
		throw ae;
	}catch (...){
		logMessage << "ERROR Loading services list. Check XML Configuration file.";
		logIt(logMessage);
	}
}

void ActiveXML::loadProperties(ActiveLink* activeLink, ticpp::Element*  link){

	int iteratorCounter=0;
	std::stringstream logMessage;

	logMessage << "Loading properties for each link from configuration file.";
	logIt(logMessage);

	std::string name;
	int type;
	int intValue;
	float floatValue;
	std::string stringValue;

	try{
		ticpp::Element* properties = link->FirstChildElement("properties");
		ticpp::Iterator<ticpp::Element> propertiesIterator;

		for (	propertiesIterator = propertiesIterator.begin(properties);
				propertiesIterator != propertiesIterator.end();
				propertiesIterator++){

			try {
				iteratorCounter++;
				ticpp::Element* property = propertiesIterator.Get();

				////////////////////////////////////////////////////////////////////////
				// getting name and type
				getString(property,"name",name);
				getInt(property,"type",type);

				/////////////////////////////////////////////////////////////////////////
				// depending of type parameter we need to add in each type
				switch (type){
				case ACTIVE_INT_PARAMETER:{
					getInt(property,"value",intValue);
					activeLink->insertIntProperty(name,intValue);
				}
				break;
				case ACTIVE_REAL_PARAMETER:{
					getReal(property,"value",floatValue);
					activeLink->insertRealProperty(name,floatValue);
				}
				break;
				case ACTIVE_STRING_PARAMETER:{
					getString(property,"value",stringValue);
					activeLink->insertStringProperty(name,stringValue);
				}
				break;
				}
			}catch (ActiveException& ae){
				logMessage << "ERROR: Something happened loading property link.";
				logIt(logMessage);
			}
		}
	}catch (...){
		logMessage << "ERROR: Something happened loading links properties check XML Configuration file ";
		logIt(logMessage);
	}


}

void ActiveXML::loadServices() throw (ActiveException){

	int iteratorCounter=0;
	std::string linkId;
	std::string serviceId;
	std::stringstream logMessage;

	try{
		ticpp::Element* servicesList = doc->FirstChildElement("serviceslist");
		ticpp::Iterator<ticpp::Element> servicesIterator;

		for (servicesIterator = servicesIterator.begin(servicesList); servicesIterator != servicesIterator.end(); servicesIterator++){
			try {
				iteratorCounter++;
				ticpp::Element* service = servicesIterator.Get();

				logMessage << "-----------SERVICES -------------------";
				logIt(logMessage);

				//getting attributes
				getString(service,"id",serviceId);

				ticpp::Iterator<ticpp::Element> linksIterator;

				for (linksIterator = linksIterator.begin(service); linksIterator != linksIterator.end(); linksIterator++){
					ticpp::Element* link = linksIterator.Get();
					getString(link,"id",linkId);
					ActiveManager::getInstance()->insertInMMap(serviceId,linkId);
				}

			}catch (ActiveException& ae){
				logMessage << "ERROR: Something happened initializing service data " << iteratorCounter << ". Going to next service.";
				logIt(logMessage);
			}
		}
		logMessage << "----------------------------------";
		logIt(logMessage);
	}catch( ... ){
		logMessage << "ERROR Loading services list, check XML configuration file";
		throw ActiveException(logMessage.str());
	}
}

void ActiveXML::getString(ticpp::Element*  link, std::string name, std::string& result, bool forcedParameter)
	throw (ActiveException){

	std::string value;
	std::stringstream logMessage;

	try{
		link->GetAttribute( name, &value );
		logMessage << "Getting string attribute "<< name << " with value "<<value;
		logIt(logMessage);
		result=value;
	}catch (ticpp::Exception& ex){
		if (forcedParameter){
			LOG4CXX_DEBUG(logger, "ERROR."<< ex.what());
			throw ActiveException();
		}
	}
}

void ActiveXML::getInt(ticpp::Element*  link, std::string name, int& result, bool forcedParameter)
	throw (ActiveException){

	int value;
	std::stringstream logMessage;

	try{
		link->GetAttribute( name, &value );
		logMessage << "Getting int attribute "<< name << " with value " << value;
		logIt(logMessage);
		result=value;
	}catch (ticpp::Exception& ex){
		if (forcedParameter) {
			LOG4CXX_INFO(logger, "ERROR."<< ex.what());
			throw ActiveException();
		}
	}
}

void ActiveXML::getReal(ticpp::Element*  link, std::string name, float& result, bool forcedParameter)
	throw (ActiveException){

	float value;
	std::stringstream logMessage;

	try{
		link->GetAttribute( name, &value );
		logMessage << "Getting real attribute "<< name << " with value " << value;
		logIt(logMessage);
		result=value;
	}catch (ticpp::Exception& ex){
		if (forcedParameter) {
			LOG4CXX_DEBUG(logger, "ERROR."<< ex.what());
			throw ActiveException();
		}
	}
}

void ActiveXML::getBool(ticpp::Element*  link, std::string name, bool& result, bool forcedParameter)
	throw (ActiveException){

	bool value;
	std::stringstream logMessage;

	try{
		link->GetAttribute( name, &value );
		logMessage << "Getting boolean attribute "<< name << " with value " << value;
		logIt(logMessage);
		result=value;
	}catch (ticpp::Exception& ex){
		if (forcedParameter){
			LOG4CXX_DEBUG(logger, "ERROR."<< ex.what());
			throw ActiveException();
		}
	}
}

bool ActiveXML::isConsumer (int type){
	if (type%2 == 0){
		return true;
	}
	return false;
}

void ActiveXML::logIt (std::stringstream& logMessage){
	LOG4CXX_DEBUG(logger, logMessage.str().c_str());
	logMessage.str("");
}

ActiveXML::~ActiveXML() {
	delete doc;
}
