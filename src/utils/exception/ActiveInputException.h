/*
 * ActiveInputException.h
 *
 *  Created on: 14/03/2011
 *      Author: opernas
 */

#ifndef ACTIVEINPUTEXCEPTION_H_
#define ACTIVEINPUTEXCEPTION_H_

#include <string>

class ActiveInputException {
private:
	/**
	 * String that store the cause of exception
	 */
	std::string cause;
public:
	/**
	 * default constructor
	 */
	ActiveInputException(){}

	/**
	 * Constructor
	 *
	 * @param s string to set the cause of the exception
	 */
	ActiveInputException(const std::string& s): cause(s) {};

	/**
	 * Constructor
	 *
	 * @param s to set the cause of exception.
	 */
	ActiveInputException(const std::stringstream& s): cause(s.str()) {};

	/**
	 * Method to get the message of exception
	 *
	 * @return exception message
	 */
	const std::string& getMessage() { return cause;}

	/**
	 * Virtual destructor
	 */
	virtual ~ActiveInputException(){}
};

#endif /* ACTIVEINPUTEXCEPTION_H_ */
