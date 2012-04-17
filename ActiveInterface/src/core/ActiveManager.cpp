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
 * Class that is the core of the library, implements a singleton to provides
 * a unique access pointer and stores all data structures needed for the library
 * to store data.
 *
 */

#include "ActiveManager.h"
#include <activemq/library/ActiveMQCPP.h>
#include "../utils/exception/ActiveException.h"
#include "wrapper/ActiveConsumer.h"
#include "wrapper/ActiveProducer.h"

using namespace log4cxx;
using namespace log4cxx::helpers;

/**
 * Variables used for singleton
 */
bool ActiveManager::instanceFlag=false;
ActiveManager* ActiveManager::mySelf;

LoggerPtr ActiveManager::logger(Logger::getLogger("ActiveManager"));


ActiveManager::ActiveManager() {
	//vars for singleton
	instanceFlag=false;
	//pointer to myself to setup the answer
	mySelf=NULL;
	//reference to activeInterface for callback
	activeInterfacePtr=NULL;
	//by default we are going to serialize messages in consumption
	messageSerializedInConsumption=false;
}

void ActiveManager::init (	const std::string& configurationFile,
							ActiveInterface* activeInterfacePtrR,
							bool messageSerializedInConsumptionR)
	throw (ActiveException){

	std::stringstream logMessage;
	try{
		//initialize pointer for callbacks
	    activeInterfacePtr=activeInterfacePtrR;
		//initializing activemq-cpp library
	    activemq::library::ActiveMQCPP::initializeLibrary();
		//serializing onMessage in user space or not
		messageSerializedInConsumption=messageSerializedInConsumptionR;
		//initializing xml library
		initXMLLibrary(configurationFile);
		//initializing all memory structures extracted from xml
		initMemStructures();

	}catch (ActiveException& e){
		throw e;
	}
}

void ActiveManager::initMemStructures() throw (ActiveException){

	try{
		activeXML.loadConnections();
		activeXML.loadActiveLinks();
		activeXML.loadServices();
	}catch (ActiveException& ae){
		throw ae;
	}
}

void ActiveManager::initXMLLibrary(const std::string& configurationFile)
	throw (ActiveException){

	try {
		activeXML.init(configurationFile);
	}catch (ActiveException& ae){
		throw ae;
	}
}

void ActiveManager::startConnections() throw (ActiveException){
	std::stringstream logMessage;
	try{
		std::map<std::string,ActiveConnection*>::iterator it=connectionsMap.begin();
		std::map<std::string,ActiveConnection*>::iterator itEnd=connectionsMap.end();
		while ( it != itEnd ){
			ActiveConnection* activeConnection=(ActiveConnection*)((*it).second);
			try{
				if (activeConnection){
					activeConnection->run();
				}else{
					logMessage << "ActiveManager::startConnection. Connection was not started.";
					throw ActiveException(logMessage);
				}
				logMessage<<"Started connection id "<<((*it).first) << " succesfully.";
				logIt(logMessage);
			}catch (ActiveException& ae){
				//deleting iterator
				//connectionsMap.erase(it++);
				//deleting activeConnection
				//delete activeConnection;
				//logging it
				logMessage<<ae.getMessage();
				logIt(logMessage);
				throw ae;
			}
			it++;
		}
	}catch (ActiveException& ae){
		throw ae;
	}catch (...){
		throw ActiveException("Unknown exception ocurred starting services");
	}
}

void ActiveManager::startConnection(std::string& connectionId) throw (ActiveException){

	std::stringstream logMessage;
	std::map<std::string,ActiveConnection*>::iterator it;
	try{
		if(connectionsMap.find(connectionId) == connectionsMap.end()){
			logMessage << "ActiveManager::startConnection. Connection id "<< connectionId << "does not exists in the map";
			throw ActiveException(logMessage);
		}else{
			it=connectionsMap.find(connectionId);
			ActiveConnection* activeConnection=(ActiveConnection*)(it->second);
			try{
				logMessage << "ActiveManager::startConnection. Connection id "<< connectionId << " is going to start.";
				LOG4CXX_DEBUG(logger,logMessage.str().c_str());
				if (activeConnection){
					activeConnection->run();
				}else{
					logMessage << "ActiveManager::startConnection. Connection id "<< connectionId << "was not started.";
					throw ActiveException(logMessage);
				}
			}catch (ActiveException& ae){
				//deleting iterator
				connectionsMap.erase(it++);
				//deleting activeConnection
				delete activeConnection;
				//logging it
				logMessage<<ae.getMessage();
				logIt(logMessage);
				throw ae;
			}
		}
	}catch (ActiveException& ae){
		logMessage << "ActiveManager::startConnection. Exception ocurred " << ae.getMessage();
		throw ae;
	}catch (...){
		throw ActiveException("Unknown exception ocurred starting services");
	}
}

void ActiveManager::sendData(	std::string& serviceId,
								ActiveMessage& activeMessage) throw (ActiveException){

	std::stringstream logMessage;
	try{
		//test if serviceId exists on multimap
		if(servicesMMap.find(serviceId) == servicesMMap.end()){
			logMessage << "ActiveManager::sendData. Service identifier doesnt exist" << serviceId;
			throw ActiveException(logMessage.str());
		}else{
			std::pair<std::multimap<std::string,ActiveLink*>::iterator, std::multimap<std::string,ActiveLink*>::iterator> iterator;

			iterator = servicesMMap.equal_range(serviceId);

			for(std::multimap<std::string,ActiveLink*>::iterator iteratorAux=iterator.first;
				iteratorAux!=iterator.second;++iteratorAux){

				ActiveLink* activeLink=(ActiveLink*)((*iteratorAux).second);
				if (activeLink->getActiveConnection()){
					if (activeLink->getActiveConnection()->getType()==ACTIVE_PRODUCER ||
						activeLink->getActiveConnection()->getType()==ACTIVE_PRODUCER_RR ){
						int result=activeLink->getActiveConnection()->deliver(activeMessage,*activeLink);
						if (result==-1){
							if (!activeLink->getActiveConnection()->isInRecoveryMode()){
								throw ActiveException("ERROR: Queue is full, or something bad happened. Persistence is not on?");
							}
						}
					}else{
						logMessage << "ActiveManager::send. Error connection is not ready or is not a producer.";
						throw ActiveException(logMessage.str());
					}
				}else{
					logMessage << "ActiveManager::send. This link has no connection to send through.";
					throw ActiveException(logMessage.str());
				}
			}
		}
	}catch (ActiveException e){
		throw e;
	}catch (...){
		logMessage.str("Unknown Exception sending data.");
		throw ActiveException(logMessage.str());
	}
}

void ActiveManager::sendData(	std::string& serviceId,
								ActiveMessage& activeMessage,
								std::list<int>& positionInQueue) throw (ActiveException){

	std::stringstream logMessage;
	try{
		//test if serviceId exists on multimap
		if(servicesMMap.find(serviceId) == servicesMMap.end()){
			logMessage << "ActiveManager::sendData. Service identifier doesnt exist" << serviceId;
			throw ActiveException(logMessage.str());
		}else{
			std::pair<std::multimap<std::string,ActiveLink*>::iterator, std::multimap<std::string,ActiveLink*>::iterator> iterator;

			iterator = servicesMMap.equal_range(serviceId);

			for(std::multimap<std::string,ActiveLink*>::iterator iteratorAux=iterator.first;
				iteratorAux!=iterator.second;++iteratorAux){

				ActiveLink* activeLink=(ActiveLink*)((*iteratorAux).second);
				if (activeLink->getActiveConnection()){
					if (activeLink->getActiveConnection()->getType()==ACTIVE_PRODUCER ||
						activeLink->getActiveConnection()->getType()==ACTIVE_PRODUCER_RR ){

						int result=activeLink->getActiveConnection()->deliver(activeMessage,*activeLink);
						positionInQueue.push_front(result);
						if (result==-1){
							if (!activeLink->getActiveConnection()->isInRecoveryMode()){
								throw ActiveException("ERROR: Queue is full, or something bad happened. Persistence is on?");
							}
						}
					}else{
						logMessage << "ActiveManager::sendData. Error connection is not ready or is not a producer. "<<activeLink->getId();
						throw ActiveException(logMessage.str());
					}
				}else{
					logMessage << "ActiveManager::send. This link has no connection to send through: "<< activeLink->getId();
					throw ActiveException(logMessage.str());
				}
			}
		}
	}catch (ActiveException e){
		throw e;
	}catch (...){
		logMessage.str("Unknown Exception sending data.");
		throw ActiveException(logMessage.str());
	}
}

void ActiveManager::sendResponse (std::string& connectionId, ActiveMessage& activeMessage) throw (ActiveException){

	std::stringstream logMessage;
	try{
		//test if connection exists on map
		if(connectionsMap.find(connectionId) == connectionsMap.end()){
			logMessage << "ActiveManager::sendResponse. This connection id does not exist " << connectionId;
			throw ActiveException(logMessage.str());
		}else{
			std::map <std::string,ActiveConnection*>::iterator it;
			it=connectionsMap.find(connectionId);
			ActiveConnection* activeConnection=(ActiveConnection*)((*it).second);

			if (activeConnection && activeConnection->getType()==ACTIVE_CONSUMER_RR ){
				if (activeConnection->deliver(activeMessage)==-1){
					if (activeConnection->getState()!=CONNECTION_IN_PERSISTENCE){
						logMessage << "ActiveManager::sendResponse. Imposible to send response to connection " << connectionId;
						throw ActiveException(logMessage.str());
					}
				}else{
					logMessage << "ActiveManager::sendResponse. Sent response to connection " << connectionId;
					LOG4CXX_DEBUG(logger, logMessage.str().c_str());
				}
			}else{
				logMessage << "ActiveManager::sendResponse. Error connection is not with RR. ";
				throw ActiveException(logMessage.str());
			}
		}
	}catch (ActiveException& ae){
		throw ae;
	}catch (...){
		logMessage.str("Unknown Exception sending data.");
		throw ActiveException(logMessage.str());
	}
}

ActiveConnection* ActiveManager::newConnection (std::string& id,
												std::string& ipBroker,
												int type,
												std::string& destination,
												bool topic,
												bool persistent,
												const std::string& selector,
												bool durable,
												bool clientAck,
												int maxSizeQueue,
												const std::string& username,
												const std::string& password,
												const std::string& clientId,
												int persistence,
												const std::string& certificate) throw (ActiveException){

	std::stringstream logMessage;
	try{
		logMessage << "ActiveManager::newConnection. new connection to  " << ipBroker <<
				" destination "<< destination;
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());

		std::string selectorNC=selector;
		std::string usernameNC=username;
		std::string passwordNC=password;
		std::string clientIdNC=clientId;
		std::string certificateNC=certificate;

		ActiveConnection* connectionPtr=saveConnection(	id, ipBroker, type, topic, destination,
														persistent,selectorNC,durable,clientAck,maxSizeQueue,
														usernameNC,passwordNC,clientIdNC,persistence,certificateNC);
		if (connectionPtr){
			//startConnection(id);
			return connectionPtr;
		}else{
			logMessage.str("Connection couldn't be stored in map. is there another one with the same id?");
			LOG4CXX_DEBUG(logger,logMessage.str().c_str());
			throw ActiveException (logMessage.str());
		}
	}catch (ActiveException& ae){
		LOG4CXX_DEBUG(logger,ae.getMessage().c_str());
		throw ae;
	}
	return false;
}

bool ActiveManager::newLink(std::string& serviceId,
							std::string& linkId,
							std::string& name,
							std::string& connectionId,
							ParameterList& parameterList) throw (ActiveException){

	std::string key;
	std::stringstream logMessage;
	try{

		logMessage << "ActiveManager::newLink. new link to  " << serviceId <<
				" link id "<< linkId;
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());

		//initializing activelink
		ActiveLink* activeLink=NULL;
		//getting connection from map
		ActiveConnection* activeConnection=getConnection(connectionId);
		//creatin link
		activeLink=new ActiveLink(linkId,name,activeConnection);
		//setting the linkId to the connection
		if (activeConnection){
			activeConnection->setLinkId(linkId);
		}else{
			logMessage.str("");
			logMessage << "Creating Link. Link " << linkId;
			logMessage << ". Error, connection does not exist " << connectionId;
			LOG4CXX_DEBUG (logger,logMessage.str().c_str());
			logMessage.str("");
		}

		//loading properties
		for (unsigned int it=0; it<parameterList.size();it++){
			Parameter* property=parameterList.get(it,key);
			switch (property->getType()){
			case ACTIVE_INT_PARAMETER:{
				IntParameter* intParameter=(IntParameter*)property;
				activeLink->insertIntProperty(key,intParameter->getValue());
			}
			break;
			case ACTIVE_REAL_PARAMETER:{
				RealParameter* realParameter=(RealParameter*)property;
				activeLink->insertRealProperty(key,realParameter->getValue());
			}
			break;
			case ACTIVE_STRING_PARAMETER:{
				StringParameter* stringParameter=(StringParameter*)property;
				activeLink->insertStringProperty(key,stringParameter->getValue());
			}
			break;
			}
		}
		//inserting in map
		if (insertInLinksMap(linkId,activeLink)){
			insertInMMap(serviceId,linkId);
		}

		return true;
	}catch (ActiveException& ae){
		LOG4CXX_DEBUG(logger,ae.getMessage().c_str());
		throw ae;
	}
	return false;
}

bool ActiveManager::destroyConnection(std::string& connectionId) throw (ActiveException) {

	std::stringstream logMessage;
	try{
		if(connectionsMap.find(connectionId) == connectionsMap.end()){
			logMessage << "ActiveManager::destroyConnection. Connection id "<< connectionId
					<< "does not exists in the map";
			return false;
		}else{
			logMessage << "ActiveManager::destroyConnection. Connection id "<< connectionId <<
					"is going to be destroyed";
			LOG4CXX_DEBUG(logger,logMessage.str().c_str());

			//closing connection
			ActiveConnection* activeConnection=(ActiveConnection*)(connectionsMap.find(connectionId)->second);
			if (activeConnection){
				connectionsMap.erase(connectionId);
				//we are going to set the connection from link to connection to null
				ActiveLink* al=getLink(activeConnection->getLinkId());
				al->removeConnBinding();
				//closing connection
				delete activeConnection;
			}else{
				throw ActiveException("Connection does not exists in map");
			}
			//returning
			return true;
		}
	}catch (ActiveException& ae){
		logMessage << "ActiveManager::destroyConnection. Exception ocurred " << ae.getMessage();
		throw ActiveException(logMessage);
	}catch(...){
		throw ActiveException ("Unknown exception.");
	}
	return false;
}

bool ActiveManager::destroyLink(std::string& linkId) throw (ActiveException){

	std::stringstream logMessage;
	try{
		//we have to delete all references from service to this link
		//that is going to be deleted
		std::list<ActiveLink*> auxLinks;
		ActiveLink* al=getLink(linkId);
		if (al){
			auxLinks.push_front(al);
		}
		destroyServiceLink(auxLinks);
		//later we are going to destroy the activelink
		std::map<std::string,ActiveLink*>::iterator it=linksMap.find(linkId);
		ActiveLink* activeLink=(*it).second;
		if (activeLink){
			delete activeLink;
		}
		linksMap.erase(linkId);
		return true;
	}catch (ActiveException e){
		LOG4CXX_ERROR(logger, e.getMessage().c_str());
		throw e;
	}
	return false;
}

bool ActiveManager::destroyService(std::string& serviceId) throw (ActiveException) {

	std::stringstream logMessage;
	try{
		servicesMMap.erase(serviceId);
		return true;
	}catch (ActiveException& e){
		LOG4CXX_ERROR(logger, e.getMessage().c_str());
		throw e;
	}
	return false;
}

bool ActiveManager::destroyLinkConnection(std::string& linkId) throw (ActiveException) {
	
	std::stringstream logMessage;
	try{
		//test if serviceId exists on multimap
		if(linksMap.find(linkId) == linksMap.end()){
			logMessage << "ActiveManager::destroyLinkConnection. couldn't find linkId id in linksMap " << linkId;
			return false;
		}else{
			std::map<std::string,ActiveLink*>::iterator iterator;
			iterator=linksMap.find(linkId);

			ActiveLink* activeLink=(ActiveLink*)((*iterator).second);
			activeLink->removeConnBinding();
		}
		return true;
	}catch (ActiveException e){
		LOG4CXX_ERROR(logger, e.getMessage().c_str());
		throw e;
	}
}

bool ActiveManager::setLinkConnection(	std::string& linkId,
										std::string& connectionId) throw (ActiveException) {

	std::stringstream logMessage;
	try{
		ActiveConnection* activeConnection=getConnection(connectionId);
		if (activeConnection){
			ActiveLink*  activeLink=getLink(linkId);
			if (activeLink){
				activeLink->setConnection(activeConnection);
				return true;
			}
		}
		return false;
	}catch (ActiveException e){
		LOG4CXX_ERROR(logger, e.getMessage().c_str());
		throw e;
	}
}

bool ActiveManager::removeLinkBindingTo (std::string& connectionId)  throw (ActiveException) {

	std::stringstream logMessage;

	try{
		for(std::map<std::string,ActiveLink*>::iterator iteratorAux=linksMap.begin();
				iteratorAux!=linksMap.end();++iteratorAux){

			ActiveLink* activeLink=(ActiveLink*)((*iteratorAux).second);
			if (activeLink){
				ActiveConnection* activeConnection=activeLink->getActiveConnection();
				if (activeConnection && activeConnection->getId()==connectionId){
					activeLink->removeConnBinding();
				}
			}
		}
		return true;
	}catch (ActiveException& e){
		LOG4CXX_ERROR(logger, e.getMessage().c_str());
		throw e;
	}
}

bool ActiveManager::destroyServiceLink (std::list<ActiveLink*>& linkList) throw (ActiveException){

	std::stringstream logMessage;
	bool returnValue=false;

	try{
		for (std::multimap <std::string,ActiveLink*>::iterator it = servicesMMap.begin();
				it != servicesMMap.end();){

			ActiveLink* activeLink=(ActiveLink*)((*it).second);

			for (std::list<ActiveLink*>::iterator iteratorLinks=linkList.begin();
					iteratorLinks!= linkList.end(); iteratorLinks++){

				ActiveLink* activeLinkToDelete=(ActiveLink*)(*iteratorLinks);
				
				if ((activeLink && activeLinkToDelete) &&
					(activeLink->getId()==activeLinkToDelete->getId())){
					servicesMMap.erase(it++);
					returnValue=true;
				}else{
					++it;
				}
			}
		}
	}catch (ActiveException& ae){
		LOG4CXX_ERROR(logger, ae.getMessage().c_str());
		throw ae;
	}
	return returnValue;
}

bool ActiveManager::destroyServiceLink (std::string& linkId,
										std::string& serviceId) throw (ActiveException){

	std::stringstream logMessage;
	try{
		//test if serviceId exists on multimap
		if(servicesMMap.find(serviceId) == servicesMMap.end()){
			logMessage << "ActiveManager::destroyServiceLink. serviceId does not exist " << serviceId;
			throw ActiveException(logMessage.str());
		}else{
			std::pair<std::multimap<std::string,ActiveLink*>::iterator, std::multimap<std::string,ActiveLink*>::iterator> iterator;

			iterator = servicesMMap.equal_range(serviceId);

			for(std::multimap<std::string,ActiveLink*>::iterator iteratorAux=iterator.first;
				iteratorAux!=iterator.second;){

				ActiveLink* activeLink=(ActiveLink*)((*iteratorAux).second);
				if (activeLink->getId()==linkId){
					servicesMMap.erase(iteratorAux++);
				}else{
					++iteratorAux;
				}
			}
		}
	}catch (ActiveException& ae){
		LOG4CXX_ERROR(logger, ae.getMessage().c_str());
		throw ae;
	}
	return true;
}

void ActiveManager::getLinksByConn(	std::string& connectionId,
									std::list<ActiveLink*>& linkList) throw (ActiveException){

	std::stringstream logMessage;
	//clearing list
	linkList.clear();
	try{
		for(std::map<std::string,ActiveLink*>::iterator iteratorAux=linksMap.begin();
				iteratorAux!=linksMap.end();++iteratorAux){

			ActiveLink* activeLink=(ActiveLink*)((*iteratorAux).second);
			if (activeLink){
				ActiveConnection* activeConnection=activeLink->getActiveConnection();
				if (activeConnection && activeConnection->getId()==connectionId){
					linkList.push_front(activeLink);
				}
			}
		}
	}catch (ActiveException& e){
		LOG4CXX_ERROR(logger, e.getMessage().c_str());
		throw e;
	}
}

void ActiveManager::getLinksByService(	std::string& serviceId,
										std::list<ActiveLink*>& linkList) throw (ActiveException){

	std::stringstream logMessage;
	//clearing list
	linkList.clear();
	try{
		//test if serviceId exists on multimap
		if(servicesMMap.find(serviceId) == servicesMMap.end()){
			logMessage << "ActiveManager::getLinksByService. serviceId does not exist " << serviceId;
			throw ActiveException(logMessage.str());
		}else{
			std::pair<std::multimap<std::string,ActiveLink*>::iterator, std::multimap<std::string,ActiveLink*>::iterator> iterator;

			iterator = servicesMMap.equal_range(serviceId);

			for(std::multimap<std::string,ActiveLink*>::iterator iteratorAux=iterator.first;
				iteratorAux!=iterator.second;++iteratorAux){

				ActiveLink* activeLink=(ActiveLink*)((*iteratorAux).second);
				if (activeLink){
					linkList.push_front(activeLink);
				}else{
					LOG4CXX_DEBUG(logger,"There are entrys in links map with null objects");
				}
			}
		}
	}catch (ActiveException e){
		LOG4CXX_ERROR(logger, e.getMessage().c_str());
		throw e;
	}

}

void ActiveManager::getServicesByLink(	std::string& linkId,
										std::list<std::string>& linkList) throw (ActiveException){
	std::stringstream logMessage;
	//clearing list
	linkList.clear();

	try{
		for(std::multimap<std::string,ActiveLink*>::iterator iteratorAux=servicesMMap.begin();
				iteratorAux!=servicesMMap.end();++iteratorAux){

			ActiveLink* activeLink=(ActiveLink*)((*iteratorAux).second);
			if (activeLink){
				if (activeLink->getId()==linkId){
					linkList.push_front((*iteratorAux).first);
				}
			}else{
				LOG4CXX_DEBUG(logger,"There are entrys in links map with null objects");
			}
		}
	}catch (ActiveException e){
		LOG4CXX_ERROR(logger, e.getMessage().c_str());
		throw e;
	}
}

ActiveConnection* ActiveManager::getConnByLink(std::string& linkId) throw (ActiveException) {

	std::stringstream logMessage;

	try{
		//test if serviceId exists on multimap
		if(linksMap.find(linkId) == linksMap.end()){
			logMessage << "ActiveManager::getConnByLink. couldn't find linkId id in linksMap " << linkId;
			throw ActiveException(logMessage.str());
		}else{
			std::map<std::string,ActiveLink*>::iterator iterator;
			iterator=linksMap.find(linkId);

			ActiveLink* activeLink=(ActiveLink*)((*iterator).second);
			if (activeLink){
				ActiveConnection* activeConnection=activeLink->getActiveConnection();
				if (activeConnection){
					return activeConnection;
				}
			}
		}
		return NULL;
	}catch (ActiveException e){
		LOG4CXX_ERROR(logger, e.getMessage().c_str());
		throw e;
	}
}

void ActiveManager::getConnsByService (	std::string& serviceId,
										std::list<ActiveConnection*>& connectionListR)
	throw (ActiveException){

	//clearing list
	connectionListR.clear();
	try{
		std::list<ActiveLink*> linksList;
		getLinksByService(serviceId,linksList);

		std::list<ActiveLink*>::iterator iterator;
		for(iterator=linksList.begin(); iterator != linksList.end(); ++iterator){
			ActiveLink* activeLink=(ActiveLink*)(*iterator);
			if (activeLink){
				ActiveConnection* activeConnection=getConnByLink(activeLink->getId());
				if (activeConnection && !existsInConnectionList(activeConnection, connectionListR)){
					connectionListR.push_front(getConnByLink(activeLink->getId()));
				}
			}
		}
	}catch (ActiveException& ae){
		LOG4CXX_ERROR(logger, ae.getMessage().c_str());
		throw ae;
	}
}

bool ActiveManager::existsInConnectionList(	ActiveConnection* activeConnection,
											std::list<ActiveConnection*>& connectionListR){
	bool returnValue=false;
	std::list<ActiveConnection*>::iterator iterator;
	for(iterator=connectionListR.begin(); iterator != connectionListR.end(); ++iterator){
		ActiveConnection* ac=(ActiveConnection*)(*iterator);
		if (ac && (ac->getId() == activeConnection->getId())){
			returnValue=true;
		}
	}
	return returnValue;
}

void ActiveManager::getConnsByDestination (	std::string& destination,
											std::list<ActiveConnection*>& connectionListR)
	throw (ActiveException){

	//clearing list
	connectionListR.clear();
	try{
		for( std::map <std::string,ActiveConnection*>::iterator ii=connectionsMap.begin();
			ii!=connectionsMap.end(); ++ii){
			ActiveConnection* activeConnection=(*ii).second;
			if (activeConnection && activeConnection->getDestination()==destination){
				connectionListR.push_front(activeConnection);
			}
		}
	}catch (ActiveException& ae){
		LOG4CXX_ERROR(logger, ae.getMessage().c_str());
		throw ae;
	}
}

void ActiveManager::getConnections (std::list<ActiveConnection*>& connectionListR)
	throw (ActiveException){

	//clearing list
	connectionListR.clear();
	try{
		for( std::map <std::string,ActiveConnection*>::iterator ii=connectionsMap.begin();ii!=connectionsMap.end(); ++ii){
			ActiveConnection* activeConnection=(*ii).second;
			if (activeConnection){
				connectionListR.push_front(activeConnection);
			}
		}
	}catch (ActiveException& ae){
		LOG4CXX_ERROR(logger, ae.getMessage().c_str());
		throw ae;
	}
}

void ActiveManager::getServices (std::list<std::string>& servicesList)
	throw (ActiveException){

	//clearing list
	servicesList.clear();
	try{
		for(std::multimap<std::string,ActiveLink*>::iterator iteratorAux=servicesMMap.begin();
			iteratorAux!=servicesMMap.end();++iteratorAux){

			std::string serviceId=(std::string)((*iteratorAux).first);
			servicesList.push_front(serviceId);
		}
	}catch (ActiveException& ae){
		LOG4CXX_ERROR(logger, ae.getMessage().c_str());
		throw ae;
	}catch (...){
		throw ActiveException ("Unknown exception getting services.");
	}

}


bool ActiveManager::insertInMMap(	std::string& serviceId,
									std::string& linkId){

	std::stringstream logMessage;

	if(linksMap.find(linkId) == linksMap.end()){
		logMessage << "ERROR: ActiveXML::insertInMMap. couldn't find link id "<<linkId;
		LOG4CXX_ERROR(logger, logMessage.str().c_str());
		return false;
	}else{
		ActiveLink* activeLink=(ActiveLink*)linksMap.find(linkId)->second;
		if (activeLink != NULL){
			std::multimap<std::string,ActiveLink*>::iterator ret;
			ret=servicesMMap.insert( std::multimap <std::string, ActiveLink*>::value_type(serviceId,activeLink));
			if (!ret->second){
				logMessage << "ActiveXML::insertInMMap. Error saving link to services mmap, link already exists with linkId "
						<<linkId;
				LOG4CXX_DEBUG(logger, logMessage.str().c_str());
				return false;
			}
		}else{
			logMessage << "Link is null, going to next link";
			return false;
		}
	}
	return true;
}

ActiveConnection* ActiveManager::saveConnection(std::string& id,
												std::string& ipBroker,
												int type,
												bool topic,
												std::string& destination,
												bool persistent,
												std::string& selector,
												bool durable,
												bool clientAck,
												int maxSizeQueue,
												std::string& username,
												std::string& password,
												std::string& clientId,
												int persistence,
												std::string& certificate){

	std::stringstream logMessage;
	bool withRequestReply=false;

	//if is even is a producer, else consumer
	if (isConsumer(type)){

		///////////////////////////////////////////////////////////////////////////////////////
		/// CONSUMER : Saving a consumer
		//////////////////////////////////////////////////////////////////////////////////////
		//getting if is a consumer with response
		if (type==ACTIVE_CONSUMER_RR){
			withRequestReply=true;
		}
		//is a consumer, instantiate it with parameters of consumer
		ActiveConsumer* activeConsumer=new ActiveConsumer(	id, ipBroker,destination,selector,
															maxSizeQueue,username,password,clientId,
															certificate,topic,clientAck,
															withRequestReply,durable);

		//saving proxylink to map
		if (activeConsumer!=NULL){
			std::pair<std::map<std::string,ActiveConnection*>::iterator,bool> ret;
			ret=connectionsMap.insert(std::map<std::string,ActiveConnection*>::value_type(id,activeConsumer));
			if (!ret.second){
				logMessage << " Error Inserting connection " << id<< " check XML configuration file.";
				logIt(logMessage);
				delete activeConsumer;
				return NULL;
			}
		}else{
			logMessage << " Problem instantiating ActiveConsumer, is null." << id;
			logIt(logMessage);
			delete activeConsumer;
			return NULL;
		}
		logMessage << "Saving connection " << id << " to map... Done";
		logIt(logMessage);

		return activeConsumer;

	}else{

		///////////////////////////////////////////////////////////////////////////////////////
		/// Producer : Saving a producer
		//////////////////////////////////////////////////////////////////////////////////////
		//getting if is a producer with response
		if (type==ACTIVE_PRODUCER_RR){
			withRequestReply=true;
		}
		//is a producer because type is even
		ActiveProducer* activeProducer=new ActiveProducer(	id,ipBroker,destination,
															maxSizeQueue,username,password,clientId,
															certificate, topic, withRequestReply,clientAck,
															persistent,persistence);

		//and inserting the new object (producer/consumer)
		//saving proxylink to map
		if (activeProducer!=NULL){
			std::pair<std::map<std::string,ActiveConnection*>::iterator,bool> ret;
			ret=connectionsMap.insert(std::map<std::string,ActiveConnection*>::value_type(id,activeProducer));
			if (!ret.second){
				logMessage << "Error saving connection... " << id;
				logIt(logMessage);
				delete activeProducer;
				return NULL;
			}
		}else{
			logMessage << "Problem instantiating activeProducer, is null." << id;
			logIt(logMessage);
			delete activeProducer;
			return NULL;
		}

		logMessage << "Saving connection " <<id << " to map... Done";
		logIt(logMessage);

		return activeProducer;
	}
}

ActiveConnection* ActiveManager::getConnection (std::string& id) throw (ActiveException){

	std::stringstream logMessage;

	if(connectionsMap.find(id) == connectionsMap.end()){
		logMessage << "ActiveManager::getConnection. I couldn't find connectionId "<< id;
	}else{
		ActiveConnection* activeConnection=(ActiveConnection*)connectionsMap.find(id)->second;
		if (activeConnection != NULL){
			return activeConnection;
		}else{
			logMessage << "ActiveLink with "<< "service id: "<< id << "was null";
			throw ActiveException(logMessage);
		}
	}
	return NULL;

}

bool ActiveManager::insertInLinksMap (	std::string& linkId,
										ActiveLink* activeLink) throw (ActiveException){
	std::stringstream logMessage;
	bool result=true;

	if (activeLink!=NULL){
		std::pair<std::map<std::string,ActiveLink*>::iterator,bool> ret;
		ret=linksMap.insert(std::map<std::string,ActiveLink*>::value_type(linkId,activeLink));
		if (!ret.second){
			logMessage << "Error saving link... " << linkId;
			result=false;
			delete activeLink;
			throw ActiveException (logMessage.str());
		}
	}else{
		result=false;
		logMessage << "Problem instantiating link, is null." << linkId;
		throw ActiveException (logMessage.str());
	}
	logMessage << "Saving link " <<linkId << " to map... Done";
	logIt(logMessage);
	return result;
}

ActiveLink* ActiveManager::getLink(std::string& id) throw (ActiveException){

	std::stringstream logMessage;

	if(linksMap.find(id) == linksMap.end()){
		logMessage << "ActiveManager::getLink. I couldn't find linkid "<< id;
	}else{
		ActiveLink* activeLink=(ActiveLink*)linksMap.find(id)->second;
		if (activeLink != NULL){
			return activeLink;
		}else{
			logMessage << "ActiveLink with "<< "service id: "<< id << "was null";
			throw ActiveException(logMessage);
		}
	}
	return NULL;
}

bool ActiveManager::isConsumer (int type){
	if (type%2 == 0){
		return true;
	}
	return false;
}

//method to return callback when i receive message
void ActiveManager::onMessageCallback (ActiveMessage& activeMessage){

	std::stringstream logMessage;

	//disable locking if user says that messages are not serialized
	if (messageSerializedInConsumption){
		messageSerializer.lock();
	}

	if (activeInterfacePtr!=NULL){
		try{
			activeInterfacePtr->onMessage(activeMessage);
		}catch(...){
			//protecting user error
			LOG4CXX_DEBUG(logger,"ERROR handling the message by the user, protecting it!");
			if (messageSerializedInConsumption){
				messageSerializer.unlock();
			}
		}
	}else{
		logMessage << "ActiveManager::onMessageCallback. Callback is null";
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());
	}

	if (messageSerializedInConsumption){
		messageSerializer.unlock();
	}
}

//mehtod that returns connection interrupt callback
void ActiveManager::onConnectionInterruptCallback(std::string& connectionId){

	std::stringstream logMessage;

	messageSerializer.lock();
	if (activeInterfacePtr!=NULL){
		activeInterfacePtr->onConnectionInterrupted(connectionId);
	}else{
		logMessage << "ActiveManager::onConnectionInterruptCallback. Callback is null";
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());
	}
	messageSerializer.unlock();
}

//mehtod that returns connection interrupt callback
void ActiveManager::onConnectionRestoreCallback(std::string& connectionId){

	std::stringstream logMessage;

	messageSerializer.lock();
	if (activeInterfacePtr!=NULL){
		activeInterfacePtr->onConnectionRestore(connectionId);
	}else{
		logMessage << "ActiveManager::onConnectionRestoreCallback. Callback is null";
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());
	}
	messageSerializer.unlock();
}

//mehtod that is invoked when a packet is dropped by the queue
void ActiveManager::onQueuePacketDropped(const ActiveMessage& activeMessage){

	std::stringstream logMessage;

	messageSerializer.lock();
	if (activeInterfacePtr!=NULL){
		activeInterfacePtr->onQueuePacketDropped(activeMessage);
	}else{
		logMessage << "ActiveManager::onQueuePacketDropped. Callback is null";
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());
	}
	messageSerializer.unlock();
}

//mehtod that is invoked when a packet is dropped by the queue
void ActiveManager::onQueuePacketReady(std::string& connectionId){

	std::stringstream logMessage;

	messageSerializer.lock();
	if (activeInterfacePtr!=NULL){
		activeInterfacePtr->onQueuePacketReady(connectionId);
	}else{
		logMessage << "ActiveManager::onConnectionDropPackage. Callback is null";
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());
	}
	messageSerializer.unlock();
}

void ActiveManager::onException(std::string& connectionId){
	std::stringstream logMessage;

	messageSerializer.lock();
	if (activeInterfacePtr!=NULL){
		activeInterfacePtr->onException(connectionId);
	}else{
		logMessage << "ActiveManager::onConnectionDropPackage. Callback is null";
		LOG4CXX_DEBUG(logger, logMessage.str().c_str());
	}
	messageSerializer.unlock();
}

ActiveManager::~ActiveManager() {

	//deleting all connections
	for( std::map <std::string,ActiveConnection*>::iterator ii=connectionsMap.begin();
		ii!=connectionsMap.end(); ++ii){
		delete (*ii).second;
	}

	//deleting all links
	for( std::map <std::string,ActiveLink*>::iterator ii=linksMap.begin();
		ii!=linksMap.end(); ++ii){
		delete (*ii).second;
	}

	ActiveManager::instanceFlag=false;
	ActiveManager::mySelf=NULL;

	//shuttind down activemqcpp library
    activemq::library::ActiveMQCPP::shutdownLibrary();
}


//method to log
void ActiveManager::logIt (std::stringstream& logMessage){
	LOG4CXX_INFO(logger, logMessage.str().c_str());
	logMessage.str("");
}
