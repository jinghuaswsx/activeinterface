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
 * Parent class of all types of parameter.
 */

#ifndef PARAMETER_H_
#define PARAMETER_H_

#ifdef ACTIVEINTERFACE_DLL
 #ifdef ACTIVEINTERFACE_EXPORTS
  #define ACTIVEINTERFACE_API __declspec( dllexport )
 #else
  #define ACTIVEINTERFACE_API __declspec( dllimport )
 #endif
#else 
 #define ACTIVEINTERFACE_API
#endif

#include <boost/serialization/base_object.hpp>
#include <map>
#include <string>

namespace ai{
 namespace utils{

	class ParameterList;

	class ACTIVEINTERFACE_API Parameter {

	protected:
		/**
		 * Type of parameter
		 * 0 - integer
		 * 1 - real
		 * 2 - string
		 * 3 - bytes
		 */
		int type;

		/**
		 * params of params?
		 */
		//ParameterList* parameterList;

	public:
		/**
		 * Default constructor
		 */
		Parameter();

		/**
		 * Copy constructor with a reference
		 *
		 * @param parameterR Reference of the object that is going to be copied to
		 * the new one
		 */
		Parameter(const Parameter& parameterR);

		/**
		 * Copy constructor with a pointer
		 *
		 * @param parameterR Pointer of the object that is going to be copied to
		 * the new one
		 */
		Parameter(const Parameter* parameterR);

		/**
		 * Method that gets the type of parameter
		 *
		 * @return Integer to know which type is the parameter
		 */
		int getType () const { return type;}

		/**
		 * Method to get the reference to the param of param
		 */
		//ParameterList* getParameterList (){ return parameterList;}

		/**
		 * Method to set the reference to the parameterList
		 */
		//void setParameterList(ParameterList* parameterListR){ parameterList=parameterListR;}

		/**
		 * Default destructor
		 */
		virtual ~Parameter(){}

		/**
		 *  Serializing parameters map
		 */
		friend class boost::serialization::access;
		template<class Archive>

		void serialize(Archive & ar, const unsigned int version){}
	};
 }
}

#endif /* PARAMETER_H_ */
