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
 * Class that extends the parameter parent class with float numbers.
 */

#ifndef REALPARAMETER_H_
#define REALPARAMETER_H_

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

	class ACTIVEINTERFACE_API RealParameter: public Parameter {
	private:
		/**
		 * Value of the parameter
		 */
		float value;

		/**
		 * Method that copy the values of the given parameter to this one
		 *
		 * @param realParameter Pointer to the object that is going to be copied
		 */
		void copy (const RealParameter* realParameter){ type=realParameter->getType(); value=realParameter->getValue();}

	public:
		/**
		 * Default constructor
		 */
		RealParameter(){type=1;}

		/**
		 * Constructor
		 *
		 * @param valueR real that is going to be assigned to value of object
		 */
		RealParameter(float valueR){ type=1; value=valueR;}

		/**
		 * Copy constructor by reference
		 *
		 * @param realParameter Reference to the object that is going to be copied to new one
		 */
		RealParameter(const RealParameter& realParameter){ copy(&realParameter);}

		/*
		 * Copy constructor by pointer
		 *
		 * @param Pointer to the object that is going to be copied to new one
		 */
		RealParameter(const RealParameter* realParameter){ copy(realParameter);}

		/**
		 * Method to get value of parameter
		 *
		 * @return the value of the parameter
		 */
		float getValue() const { return value;}

		/**
		 * Method to set the value of parameter
		 *
		 * @param valueR value that is going to set to this object
		 */
		void setValue(float valueR){ value=valueR;}

		/**
		 * Default destructor
		 */
		virtual ~RealParameter(){};

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

#endif /* REALPARAMETER_H_ */
