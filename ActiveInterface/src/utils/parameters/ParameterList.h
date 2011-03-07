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
 * Class that provides a list of parameter datatype in which parameters are
 * polymorphic.
 */


#ifndef PARAMETERLIST_H_
#define PARAMETERLIST_H_

#ifdef ACTIVEINTERFACE_DLL
 #ifdef ACTIVEINTERFACE_EXPORTS
  #define ACTIVEINTERFACE_API __declspec( dllexport )
 #else
  #define ACTIVEINTERFACE_API __declspec( dllimport )
 #endif
#else 
 #define ACTIVEINTERFACE_API
#endif

#include "log4cxx/logger.h"
#include "log4cxx/helpers/exception.h"

#include <boost/serialization/map.hpp>

#include <map>
#include "Parameter.h"
#include "IntParameter.h"
#include "RealParameter.h"
#include "StringParameter.h"
#include "BytesParameter.h"
#include "../../utils/defines.h"

namespace ai{
 namespace utils{

	class ACTIVEINTERFACE_API ParameterList {
	private:

		/**
		 * Maps for saving polimorphic parameters with a key associated
		 */
		std::map <std::string,Parameter*> parametersMap;

		/**
		 * static var for logger
		 */
		static log4cxx::LoggerPtr logger;

		/**
		 * method to log some stringstream that calls to log4cxx
		 *
		 * @param logMessage Message to log.
		 */
		void logIt (std::stringstream& logMessage);

		/**
		 * Method that insert a parameter reference into the map.
		 *
		 * @param name Name of the property to add.
		 * @param parameter Pointer to the parameter that is going to be added
		 */
		void insertToMap(std::string& name, Parameter* parameter);

	public:

		/**
		 * Methods that returns the size of the parameters list
		 *
		 * @return Number of parameters stored.
		 */
		unsigned int size ()const { return parametersMap.size();}

		/**
		 * Method that insert a integer parameter into parameter list.
		 *
		 * @param key Key that will be associated with the param
		 * @param value integer value that will be added
		 */
		void insertIntParameter(	std::string& key,
									int value);

		/**
		 * Method that insert a real parameter into parameter list.
		 *
		 * @param key Key that will be associated with the param
		 * @param value real value that will be added
		 */
		void insertRealParameter(	std::string& key,
									float value);

		/**
		 * Method that insert a string parameter into parameter list.
		 *
		 * @param key Key that will be associated with the param
		 * @param value string value that will be added
		 */
		void insertStringParameter(	std::string& key,
									const std::string& value);

		/**
		 * Method that insert a bytes parameter into parameter list.
		 *
		 * @param key Key that will be associated with the param
		 * @param value bytes value that will be added
		 */
		void insertBytesParameter(	std::string& key,
									std::vector<unsigned char>& value);

		/**
		 * Method that insert a parameter pointer into parameter list.
		 *
		 * @param key Key that will be associated with the param
		 * @param parameter Parameter pointer that will be added.
		 */
		void insertParameter(	std::string& key,
								Parameter* parameter);

		/**
		 * Method that delete a  parameter from parameter list.
		 *
		 * @param key Key to find parameter into the map
		 */
		void deleteParameter (std::string& key);

		/**
		 * Method that returns a parameter pointer from the given key name
		 *
		 * @param name key to find into map
		 * @return Parameter reference of the parameter associated with name
		 */
		Parameter* get(std::string& key) const;

		/**
		 * Method that returns the direct object of the aproppiate type
		 *
		 * @returns parameter casted
		 * @throw ActiveException if parameter is not the appropiate
		 */
		IntParameter* getInt(std::string& key) const;

		/**
		 * Method that returns the direct object of the aproppiate type
		 *º
		 * @returns parameter casted
		 * @throw ActiveException if parameter is not the appropiate
		 */
		RealParameter* getReal(std::string& key) const;

		/**
		 * Method that returns the direct object of the aproppiate type
		 *
		 * @returns parameter casted
		 * @throw ActiveException if parameter is not the appropiate
		 */
		StringParameter* getString(std::string& key) const;

		/**
		 * Method that returns the direct object of the aproppiate type
		 *
		 * @returns parameter casted
		 * @throw ActiveException if parameter is not the appropiate
		 */
		BytesParameter* getBytes(std::string& key) const;

		/**
		 * Methods that returns the parameter placed in the index position
		 * into the map.
		 *
		 * @param index Is the position number that is going to be returned
		 * from the map.
		 * @param key  Reference that the is filled up whit the key of the parameter
		 * placed in index position.
		 * @return Parameter reference of the parameter placed in index position of the map
		 */
		Parameter* get(int index, std::string& key) const;

		/**
		 * Clear all parameters stored in the parameter list.
		 */
		void clear();

		/**
		 * Clone a parameterList into another parameter list.
		 *
		 * @param parameterList Parameter lists that is used to extract all parameters
		 * and copy to the new one.
		 */
		void clone(const ParameterList& parameterList);

		/**
		 * Default constructor
		 */
		ParameterList(){}

		/**
		 * Copy constructor
		 *
		 * @param parameterList Reference to the parameterlist that is going to be cloned
		 */
		ParameterList(const ParameterList& parameterList){ clone(parameterList);}

		/**
		 * Default destructor
		 */
		virtual ~ParameterList();

		/**
		 *  Serializing parameters map
		 */
		friend class boost::serialization::access;
		template<class Archive>

		void serialize(Archive & ar, const unsigned int version){
			ar.template register_type<StringParameter>();
			ar.template register_type<RealParameter>();
			ar.template register_type<BytesParameter>();
			ar.template register_type<IntParameter>();
			ar & parametersMap;
		}
	};
  }
 }

#endif /* PARAMETERLIST_H_ */
