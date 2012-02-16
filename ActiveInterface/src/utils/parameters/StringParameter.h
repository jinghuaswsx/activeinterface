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
 * Class that extends the parameter parent class with strings.
 */

#ifndef STRINGPARAMETER_H_
#define STRINGPARAMETER_H_

#ifdef ACTIVEINTERFACE_DLL
 #ifdef ACTIVEINTERFACE_EXPORTS
  #define ACTIVEINTERFACE_API __declspec( dllexport )
 #else
  #define ACTIVEINTERFACE_API __declspec( dllimport )
 #endif
#else 
 #define ACTIVEINTERFACE_API
#endif

#include "Parameter.h"
#include <string>

#include <boost/serialization/base_object.hpp>

namespace ai{
 namespace utils{

	class ACTIVEINTERFACE_API StringParameter : public Parameter{
	private:
		/**
		 * Value of the parameter
		 */
		std::string value;

		/**
		 * Method that copy the values of the given parameter to this one
		 *
		 * @param stringParameter Pointer to the object that is going to be copied
		 */
		void copy (const StringParameter* stringParameter){ type=stringParameter->getType(); value=stringParameter->getValue();}

	public:
		/**
		 * Default constructor
		 */
		StringParameter(){type=2;}

		/**
		 * Constructor
		 *
		 * @param valueR string that is going to be assigned to value of object
		 */
		StringParameter(const std::string& valueR){ type=2; value=valueR;}

		/**
		 * Copy constructor by reference
		 *
		 * @param stringParameter Reference to the object that is going to be copied to new one
		 */
		StringParameter(const StringParameter& stringParameter){ copy(&stringParameter);}

		/*
		 * Copy constructor by pointer
		 *
		 * @param Pointer to the object that is going to be copied to new one
		 */
		StringParameter(const StringParameter* stringParameter){ copy(stringParameter);}


		/**
		 * Const Method to get value of parameter
		 *
		 * @return the value of the parameter
		 */
		const std::string& getValue() const { return value;}

		/**
		 * Method to get value of parameter
		 *
		 * @return the value of the parameter
		 */
		std::string& getValue() { return value;}

		/**
		 * Method to set the value of parameter
		 *
		 * @param valueR value that is going to set to this object
		 */
		void setValue(std::string& valueR){ value=valueR;}

		/**
		 * Default destructor
		 */
		virtual ~StringParameter(){};

		/**
		 *  Serializing parameters map
		 */
		friend class boost::serialization::access;
		template<class Archive>

		void serialize(Archive & ar, const unsigned int version){
			// serialize base class information
			ar & boost::serialization::base_object<Parameter>(*this);
			ar & value;
			ar & type;
		}
	};
 }
}

#endif /* STRINGPARAMETER_H_ */
