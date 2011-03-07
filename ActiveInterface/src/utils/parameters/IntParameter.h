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
 * Class that extends the parameter parent class with integer.
 */

#ifndef INTPARAMETER_H_
#define INTPARAMETER_H_

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

#include <boost/serialization/base_object.hpp>

namespace ai{
 namespace utils{

	class ACTIVEINTERFACE_API IntParameter : public Parameter{
	private:
		/**
		 * Value of the parameter
		 */
		int value;

		/**
		 * Method that copy the values of the given parameter to this one
		 *
		 * @param intParameter Pointer to the object that is going to be copied
		 */
		void copy (const IntParameter* intParameter){ type=intParameter->getType(); value=intParameter->getValue();}

	public:

		/**
		 * Default constructor
		 */
		IntParameter(){type=0;}

		/**
		 * Constructor
		 *
		 * @param valueR integer that is going to be assigned to value of object
		 */
		IntParameter(int valueR){ type=0; value=valueR;}

		/**
		 * Copy constructor by reference
		 *
		 * @param intParameter Reference to the object that is going to be copied to new one
		 */
		IntParameter(const IntParameter& intParameter){ copy(&intParameter);}

		/*
		 * Copy constructor by pointer
		 *
		 * @param Pointer to the object that is going to be copied to new one
		 */
		IntParameter(const IntParameter* intParameter){ copy(intParameter);}

		/**
		 * Method to get value of parameter
		 *
		 * @return the value of the parameter
		 */
		int getValue() const { return value;}

		/**
		 * Method to set the value of parameter
		 *
		 * @param valueR value that is going to set to this object
		 */
		void setValue(int valueR){ value=valueR;}

		/**
		 * Default destructor
		 */
		virtual ~IntParameter(){};

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

#endif /* INTPARAMETER_H_ */
