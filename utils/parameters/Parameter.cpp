/*
 * Parameter.cpp
 *
 *  Created on: 15/07/2010
 *      Author: opernas
 */

#include "Parameter.h"
#include "ParameterList.h"

using namespace ai::utils;

Parameter::Parameter(){
	type=0;
	//parameterList=NULL;
}

Parameter::Parameter(const Parameter& parameterR){
	type=parameterR.getType();
}

Parameter::Parameter(const Parameter* parameterR){
	type=parameterR->getType();
}
