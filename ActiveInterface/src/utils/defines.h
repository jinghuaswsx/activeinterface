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
 * Class that stores all defines needed by the library
 */

#ifndef DEFINES_H_
#define DEFINES_H_

#define abstract =0

//defines exits values
#define EXIT_OK 		0
#define EXIT_ERROR 		1
#define NULL_VALUE		-127

//define the type of connection
#define ACTIVE_CONSUMER 0
#define ACTIVE_PRODUCER 1
#define ACTIVE_CONSUMER_RR 2
#define ACTIVE_PRODUCER_RR 3

//define the type of parameter
#define ACTIVE_INT_PARAMETER 0
#define ACTIVE_REAL_PARAMETER 1
#define ACTIVE_STRING_PARAMETER 2
#define ACTIVE_BYTES_PARAMETER 3

//define the type of property
#define ACTIVE_INT_PROPERTY 10
#define ACTIVE_REAL_PROPERTY 11
#define ACTIVE_STRING_PROPERTY 12
#define ACTIVE_BYTES_PROPERTY 13

//types of message
#define ACTIVE_TEXT_MESSAGE 0
#define ACTIVE_BYTES_MESSAGE 1

//define the max parameter of a message
#define MAX_PARAMETERS 255

///definitions of type of callback
#define ON_PACKET_DROPPED 0
#define ON_EXCEPTION 1
#define ON_TRANSPORT_INTERRUPT 2
#define ON_TRANSPORT_RESUMED 3
#define ON_QUEUE_READY 4

//max % of queue full to send all messages
//Ready to send
#define LEVEL1_PERCENT_MESSAGES_READY 50
#define LEVEL2_PERCENT_MESSAGES_READY 60
#define LEVEL3_PERCENT_MESSAGES_READY 75
#define LEVEL4_PERCENT_MESSAGES_READY 90

//States of the connection
#define CONNECTION_NOT_INITIATED 0
#define CONNECTION_RUNNING 1
#define CONNECTION_IN_PERSISTENCE 2
#define CONNECTION_CLOSED 3
#define CONNECTION_INTERRUPTED 3

//states of the library
#define NOT_INITIALIZED 0
#define INITIALIZING 1
#define INITIALIZED 2
#define CLOSING 3
#define CLOSED 4

//throw activeInputException
#define AI_THROW_AIE throw ActiveInputException("ActiveInterface Library is not initialized. You should initialize it before any operation.");

#endif /* DEFINES_H_ */
