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
 * Class that implements the reading of the xml configuration file, if it is used
 * to startup the application. It is a wrapper about ticpp library.
 */


#ifndef ACTIVEXML_H_
#define ACTIVEXML_H_

#define TIXML_USE_TICPP

#include "ticpp.h"

#include "../ActiveLink.h"

#include "log4cxx/logger.h"
#include "log4cxx/helpers/exception.h"

namespace ai{

	class ActiveXML {
	public:
		/**
		 * Method that loads connections from configuration file to the map
		 *
		 * @throw ActiveException if something bad happens
		 */
		void loadConnections() throw (ActiveException);

		/**
		 * Method that loads links from configuration file and store into links map
		 * link to his connection
		 *
		 * @throw ActiveException if something bad happens
		 */
		void loadActiveLinks() throw (ActiveException);

		/**
		 * Method that loads services in a multimap with his activelinks associated
		 *
		 * @throw ActiveException if something bad happens
		 */
		void loadServices() throw (ActiveException);

		/**
		 * Method that initializes xml library from configuration file
		 *
		 * @param configurationFile the name of configuration file. By default ActiveConfiguration.xml
		 * @throw ActiveException if something bad happens
		 */
		void init(const std::string& configurationFile)throw (ActiveException);

		/**
		 * Constructor by default
		 */
		ActiveXML();

		/**
		 * Default destructor
		 */
		virtual ~ActiveXML();

	private:

		/**
		 * Reference to the document provides by ticpp xml library
		 */
		ticpp::Document* doc;

		/**
		 * Static var use by log4cxx for the logging system
		 */
		static log4cxx::LoggerPtr logger;

		/**
		 * method to log some stringstream that calls to log4cxx
		 *
		 * @param logMessage stringstream to log
		 */
		void logIt (std::stringstream& logMessage);

		/**
		 * Method that for type received returns true if is a consumer and false is a producer
		 *
		 * @param type if is even will be consumer else producer
		 *
		 * @return true if is a consumer, else false
		 */
		bool isConsumer (int type);

		/**
		 * Method that looad properties read from configuration file for each link.
		 *
		 * @param activeLink is the activeLink where we are going to sets properties
		 * @param link is a pointer to a element of ticpp xml library
		 */
		void loadProperties(ActiveLink* activeLink, ticpp::Element* link);

		/**
		 * Method that for a element of xml library returns the string property associated
		 *
		 * @param link is a pointer to a element of ticpp xml library
		 * @param name of property that we want to read
		 * @param result string in which the property is stored by the ticpp xml library
		 * @param forcedParameter is to allow to have parameters forced parameters or not for
		 * a producers or consumers. By default is true, that means that this parameters should exists. If not exists
		 * a exception will be thrown.
		 *
		 * @throw ActiveException if something bad happens
		 */
		void getString(ticpp::Element*  link, std::string name, std::string& result, bool forcedParameter=true)
			throw (ActiveException);

		/**
		 * Method that for a element of xml library returns the int property associated
		 *
		 * @param link is a pointer to a element of ticpp xml library
		 * @param name of property that we want to read
		 * @param result int in which the property is stored by the ticpp xml library
		 * @param forcedParameter is to allow to have parameters forced parameters or not for
		 * a producers or consumers. By default is true, that means that this parameters should exists. If not exists
		 * a exception will be thrown.
		 *
		 * @throw ActiveException if something bad happens
		 */
		void getInt(ticpp::Element*  link, std::string name, int& result, bool forcedParameter=true)
			throw (ActiveException);

		/**
		 * Method that for a element of xml library returns the real property associated
		 *
		 * @param link is a pointer to a element of ticpp xml library
		 * @param name of property that we want to read
		 * @param result real in which the property is stored by the ticpp xml library
		 * @param forcedParameter is to allow to have parameters forced parameters or not for
		 * a producers or consumers. By default is true, that means that this parameters should exists. If not exists
		 * a exception will be thrown.
		 *
		 * @throw ActiveException if something bad happens
		 */
		void getReal(ticpp::Element*  link, std::string name, float& result, bool forcedParameter=true)
			throw (ActiveException);

		/**
		 * Method that for a element of xml library returns the boll property associated
		 *
		 * @param link is a pointer to a element of ticpp xml library
		 * @param name of property that we want to read
		 * @param result bool in which the property is stored by the ticpp xml library
		 * @param forcedParameter is to allow to have parameters forced parameters or not for
		 * a producers or consumers. By default is true, that means that this parameters should exists. If not exists
		 * a exception will be thrown.
		 *
		 * @throw ActiveException if something bad happens
		 */
		void getBool(ticpp::Element*  link, std::string name, bool& result, bool forcedParameter=true)
			throw (ActiveException);
	};
}

#endif /* ACTIVEXML_H_ */
