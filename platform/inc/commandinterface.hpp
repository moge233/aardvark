/*
 * commandinterface.hpp
 *
 *  Created on: Jan 24, 2026
 *      Author: matt
 */

#ifndef AARDVARK_PLATFORM_INC_COMMANDINTERFACE_HPP_
#define AARDVARK_PLATFORM_INC_COMMANDINTERFACE_HPP_

#include <cstdint>

#include "commandmessage.hpp"
#include "circulbarbuffer.hpp"
#include "osalthread.hpp"

class CommandInterface
{
public:
	CommandInterface(void *lOwner, size_t lInputQueueCapacity, size_t lOutputQueueCapacity);
	~CommandInterface();
	CommandMessage *ReceiveMessage(void);
	int SendMessage(OsalThread *lDestination, CommandMessage *lMessage);

private:
	void *mOwner;
	CircularBuffer mInputQueue;
	CircularBuffer mOutputQueue;
};


#endif /* AARDVARK_PLATFORM_INC_COMMANDINTERFACE_HPP_ */
