/*
 * MyActiveInterface.cpp
 *
 *  Created on: 29/06/2010
 *      Author: opernas
 */

#include "MyActiveInterface.h"
#include "core/message/ActiveMessage.h"
#include "utils/parameters/ParameterList.h"
#include "utils/parameters/Parameter.h"
#include "utils/parameters/IntParameter.h"
#include "utils/parameters/StringParameter.h"
#include "utils/parameters/RealParameter.h"
#include "utils/parameters/BytesParameter.h"
#include "utils/exception/ActiveException.h"
#include <sstream>

static void* APR_THREAD_FUNC startupThread(apr_thread_t *thd, void *data){

	if (data){
		try{
			MyActiveInterface* myActiveInterface=(MyActiveInterface*)data;
			//Starting to consume messages
			myActiveInterface->startup();
			//exiting of thread
			apr_thread_exit(thd, APR_SUCCESS);
			std::cout << "fuera del thread de startup"<<std::endl;
		}catch(ActiveException& ae){
			std::cout << ae.getMessage()<<std::endl;
		}
	}
	return NULL;
}

static void* APR_THREAD_FUNC sendThread(apr_thread_t *thd, void *data){

	if (data){
		MyActiveInterface* myActiveInterface=(MyActiveInterface*)data;
		apr_sleep(1000000);
		std::cout << "starting to send" <<std::endl;
		//Starting to consume messages
		myActiveInterface->sendMessages();
		//exiting of thread
		apr_thread_exit(thd, APR_SUCCESS);
	}
	return NULL;
}


MyActiveInterface::MyActiveInterface() {

	messagesSent=0;
	messagesReceived=0;
	queueFull=false;
	rrMessagesReceived=0;
	exitThread=false;

	try{

		//initializing libraries
		apr_pool_create(&mp, NULL);
		apr_threadattr_create(&thd_attr, mp);
		apr_thread_create(&thd_arr, thd_attr, startupThread, (void*)this, mp);
		apr_sleep(50000);
		apr_thread_create(&thd_arr, thd_attr, sendThread, (void*)this, mp);
		apr_thread_create(&thd_arr, thd_attr, sendThread, (void*)this, mp);
		apr_thread_create(&thd_arr, thd_attr, sendThread, (void*)this, mp);
		apr_thread_create(&thd_arr, thd_attr, sendThread, (void*)this, mp);

		std::cout << "Press 'q' to quit" << std::endl;
		while( std::cin.get() != 'q') {}
		exitThread=true;
		while( std::cin.get() != 'q') {}
		shutdown();
	}catch (ActiveException& ae){
		std::cout << ae.getMessage();
	}
}

void MyActiveInterface::sendMessages(){

	int messagesNumber=5000;

	for (int it=0; it<messagesNumber; it++){

	try{
			//if (getState()!=CLOSING || getState()!=CLOSED){

				ActiveMessage activeMessage;

				std::stringstream correlation;
				correlation<< it;
				std::string aux=correlation.str();
				activeMessage.setCorrelationId(aux);

				activeMessage.setTimeToLive(0);
				activeMessage.setPriority(0);

//				std::string parametro1="prueba";
//				activeMessage.insertIntParameter(parametro1,it);
//
//				std::vector<unsigned char> parametroBytes;
//				parametroBytes.push_back(4);
//				parametroBytes.push_back(70);
//
//				std::string parametro4="prueba4";
//				activeMessage.insertBytesParameter(parametro4,parametroBytes);
//
//				std::string parametro2="prueba2";
//				activeMessage.insertRealParameter(parametro2,3.0);
//
//				std::string parametro3="prueba3";
//				activeMessage.insertStringParameter(parametro3,parametro3);
//
//				std::string propiedad1="propiedadprueba1";
//				activeMessage.insertStringProperty(propiedad1,parametro3);

				std::stringstream xmlMessage;
				xmlMessage << "Sent:"<<it<<std::endl;
				std::string auxXML=xmlMessage.str();

				activeMessage.setText(auxXML);
				activeMessage.setMessageAsText();

				std::list<int> positionInQueue;

				std::string serviceId="1";
				send(serviceId,activeMessage,positionInQueue);
				//sendData(2,activeMessage,positionInQueue);

				if (exitThread){
					std::cout << "saliendo del thread" << std::endl;
					return;
				}
				//if (it % 97==0){
				//	sleep(20);
				//}
				messagesSent++;
			//}
		}catch (ActiveException& ae ){
			std::cout << std::endl;
		}
	}
	std::cout << "saliendo del thread de envio "<< std::endl;
}

void MyActiveInterface::showActiveLinkList(std::list<ActiveLink*>& linkList){

	std::list<ActiveLink*>::iterator i;
	for(i=linkList.begin(); i != linkList.end(); ++i){
		ActiveLink* activeLink=(*i);
		std::cout << activeLink->getName() << std::endl;
	}
}

void MyActiveInterface::showActiveConnectionList(std::list<ActiveConnection*>& connectionList){

	std::list<ActiveConnection*>::iterator i;
	for(i=connectionList.begin(); i != connectionList.end(); ++i){
		ActiveConnection* activeConnection=(*i);
		std::cout << activeConnection->getClientId() << std::endl;
	}
}

void MyActiveInterface::showIntList(std::list<int>& intList){
	std::list<int>::iterator i;
	for(i=intList.begin(); i != intList.end(); ++i){
		std::cout << "Int: " << (*i) << std::endl;
	}
}


void MyActiveInterface::onMessage(const ActiveMessage& activeMessage){

	messagesReceived++;
	if (activeMessage.isWithRequestReply()){


		std::string key;
		std::stringstream resultText;
		resultText <<"Mensaje recibido: "<< activeMessage.getParametersSize() << " from connection "<<activeMessage.getConnectionId();

		if (activeMessage.isTextMessage()){
			resultText << activeMessage.getText();

			std::cout << resultText.str();

			rrMessagesReceived++;
			ActiveMessage activeMessageNew;
			activeMessageNew.setMessageAsText();
			std::stringstream aux;
			aux << "holaaaa en RR con num mensaje "<<messagesReceived << " " <<rrMessagesReceived;
			std::string auxString=aux.str();
			activeMessageNew.setText(auxString);
			activeMessageNew.setDestination(activeMessage.getDestination());

			std::string connectionToResponse=activeMessage.getConnectionId();
			sendResponse(connectionToResponse,activeMessageNew);

			std::cout << "Mensaje con RR reenviado" << std::endl;
		}else{

			//getting all parameters
			for (int it=0; it<activeMessage.getParametersSize();it++){
				Parameter* parameter=activeMessage.getParameter(it,key);
				switch (parameter->getType()){
				case ACTIVE_INT_PARAMETER:{
					IntParameter* intParameter=(IntParameter*)parameter;
					resultText<<intParameter->getValue();
				}
				break;
				case ACTIVE_REAL_PARAMETER:{
					RealParameter* realParameter=(RealParameter*)parameter;
					resultText<< " " <<realParameter->getValue();
				}
				break;
				case ACTIVE_STRING_PARAMETER:{
					StringParameter* stringParameter=(StringParameter*)parameter;
					resultText<< " " <<stringParameter->getValue();
				}
				break;
				case ACTIVE_BYTES_PARAMETER:{
					BytesParameter* bytesParameter=(BytesParameter*)parameter;
					resultText<< " " << bytesParameter->getValue().at(1);
				}
				break;
				}
			}
			/////////////////////////////////////////////////////////////////////
			//getting all properties
			for (int it=0; it<activeMessage.getPropertiesSize();it++){
				Parameter* property=activeMessage.getProperty(it,key);
				switch (property->getType()){
				case ACTIVE_INT_PARAMETER:{
					IntParameter* intParameter=(IntParameter*)property;
					resultText<<intParameter->getValue();
				}
				break;
				case ACTIVE_REAL_PARAMETER:{
					RealParameter* realParameter=(RealParameter*)property;
					resultText<< " " <<realParameter->getValue();
				}
				break;
				case ACTIVE_STRING_PARAMETER:{
					StringParameter* stringParameter=(StringParameter*)property;
					resultText<< " " <<stringParameter->getValue();
				}
				break;
				case ACTIVE_BYTES_PARAMETER:{
					BytesParameter* bytesParameter=(BytesParameter*)property;
					resultText<< " " << bytesParameter->getValue().at(1);
				}
				break;
				}
			}
			std::cout << resultText.str() << std::endl;

			rrMessagesReceived++;

			//limpio y como es request reply haré un response
			ActiveMessage activeMessageNew;
			std::string parametro1="prueba";
			activeMessageNew.insertIntParameter(parametro1,rrMessagesReceived);

			std::vector<unsigned char> parametroBytes;
			parametroBytes.push_back(4);
			parametroBytes.push_back(41);

			std::string parametro4="prueba4";
			activeMessageNew.insertBytesParameter(parametro4,parametroBytes);

			//setting the point to response
			activeMessageNew.setDestination(activeMessage.getDestination());

			std::string connectionToResponse=activeMessage.getConnectionId();
			sendResponse(connectionToResponse,activeMessageNew);


			std::cout << "Mensaje con RR reenviado" << std::endl;
		}
	}else{

		std::string key;
		std::stringstream resultText;
		resultText <<"Mensaje recibido: "<< activeMessage.getParametersSize() << " from connection "<<activeMessage.getConnectionId();

		if (activeMessage.isTextMessage()){
				resultText << activeMessage.getText() << std::endl;
				rrMessagesReceived++;
				/////////////////////////////////////////////////////////////////////
				//getting all properties
				for (int it=0; it<activeMessage.getPropertiesSize();it++){
					Parameter* property=activeMessage.getProperty(it,key);
					switch (property->getType()){
					case ACTIVE_INT_PARAMETER:{
						IntParameter* intParameter=(IntParameter*)property;
						resultText<< key << " " << intParameter->getValue() << std::endl;
					}
					break;
					case ACTIVE_REAL_PARAMETER:{
						RealParameter* realParameter=(RealParameter*)property;
						resultText<< " " <<realParameter->getValue();
					}
					break;
					case ACTIVE_STRING_PARAMETER:{
						StringParameter* stringParameter=(StringParameter*)property;
						resultText<< key << " " <<stringParameter->getValue();
					}
					break;
					case ACTIVE_BYTES_PROPERTY:{
						BytesParameter* bytesParameter=(BytesParameter*)property;
						resultText<< key << " " << bytesParameter->getValue().at(1);
					}
					break;
					}
				}
		}else{
			for (int it=0; it<activeMessage.getParametersSize();it++){
				Parameter* parameter=activeMessage.getParameter(it,key);
				switch (parameter->getType()){
				case ACTIVE_INT_PARAMETER:{
					IntParameter* intParameter=(IntParameter*)parameter;
					resultText<<intParameter->getValue();
				}
				break;
				case ACTIVE_REAL_PARAMETER:{
					RealParameter* realParameter=(RealParameter*)parameter;
					resultText<< " " <<realParameter->getValue();
				}
				break;
				case ACTIVE_STRING_PARAMETER:{
					StringParameter* stringParameter=(StringParameter*)parameter;
					resultText<< " " <<stringParameter->getValue();
				}
				break;
				case ACTIVE_BYTES_PARAMETER:{
					BytesParameter* bytesParameter=(BytesParameter*)parameter;
					resultText<< " " << bytesParameter->getValue().at(1);
				}
				break;
				}
			}
			/////////////////////////////////////////////////////////////////////
			//getting all properties
			for (int it=0; it<activeMessage.getPropertiesSize();it++){
				Parameter* property=activeMessage.getProperty(it,key);
				switch (property->getType()){
				case ACTIVE_INT_PARAMETER:{
					IntParameter* intParameter=(IntParameter*)property;
					resultText<<intParameter->getValue();
				}
				break;
				case ACTIVE_REAL_PARAMETER:{
					RealParameter* realParameter=(RealParameter*)property;
					resultText<< " " <<realParameter->getValue();
				}
				break;
				case ACTIVE_STRING_PARAMETER:{
					StringParameter* stringParameter=(StringParameter*)property;
					resultText<< " " <<stringParameter->getValue();
				}
				break;
				case ACTIVE_BYTES_PROPERTY:{
					BytesParameter* bytesParameter=(BytesParameter*)property;
					resultText<< " " << bytesParameter->getValue().at(1);
				}
				break;
				}
			}
		}
		std::cout << resultText.str() << std::endl;
	}
}

void MyActiveInterface::onConnectionInterrupted(std::string& connectionId){
	std::cout << "ERROR: Received on onConnectionInterrupted from " << connectionId << std::endl;
}

void MyActiveInterface::onConnectionRestore(std::string& connectionId){
	std::cout << "ERROR: Received on onConnectionRestore from " << connectionId << std::endl;
}

void MyActiveInterface::onQueuePacketDropped(const ActiveMessage& activeMessage){
	std::string key;
	std::stringstream resultText;
	resultText <<"MENSAJE DROPPED: "<< activeMessage.getParametersSize();
}

void MyActiveInterface::onQueuePacketReady(std::string& connectionId){
	std::cout << "MyActiveInterface. Queue is ok" << std::endl;
	queueFull=false;
}

void MyActiveInterface::onException(std::string& connectionId){
}

MyActiveInterface::~MyActiveInterface() {
	std::cout << "We've sent "<< messagesSent << " messages";
	std::cout << "We've received "<< messagesReceived << " messages";
	std::cout << std::endl;
}
