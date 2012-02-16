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
 * Class that implements a simple exception used.
 *
 */

#ifndef ACTIVEEXCEPTION_H_
#define ACTIVEEXCEPTION_H_

#include <string>

class ActiveException{
private:
	/**
	 * String that store the cause of exception
	 */
	std::string cause;
public:
	/**
	 * default constructor
	 */
	ActiveException() {};

	/**
	 * Constructor
	 *
	 * @param s string to set the cause of the exception
	 */
	ActiveException(const std::string& s): cause(s) {};

	/**
	 * Constructor
	 *
	 * @param s to set the cause of exception.
	 */
	ActiveException(const std::stringstream& s): cause(s.str()) {};

	/**
	 * Method to get the message of exception
	 *
	 * @return exception message
	 */
	const std::string& getMessage() { return cause;}

	/**
	 * Virtual destructor
	 */
	virtual ~ActiveException(){}
};

#endif /* ACTIVEEXCEPTION_H_ */
