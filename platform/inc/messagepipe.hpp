/*
 * MessagePipe.hpp
 *
 *  Created on: Jan 24, 2026
 *      Author: matt
 */

#ifndef AARDVARK_DRIVERS_INC_MESSAGEPIPE_HPP_
#define AARDVARK_DRIVERS_INC_MESSAGEPIPE_HPP_


#include <cstddef>

#include <mqueue.h>
#include <pthread.h>

#include "commandmessage.hpp"

constexpr size_t MESSAGE_QUEUE_SIZE{100};

typedef enum
{
	MASTER_INPUT,
	MASTER_OUTPUT,
	CLIENT_INPUT,
	CLIENT_OUTPUT,
} PipeType;

class MessagePipe
{

public:
	explicit MessagePipe(const char *lName, const long lCapacity, PipeType lType);
	~MessagePipe();

	int Send(CommandMessage *lMessage);
	ssize_t Receive(CommandMessage *lMessage);

private:
	const char *mName;
	const long mCapacity;
	const PipeType mType;
	mqd_t mQueue;
};


#endif /* AARDVARK_DRIVERS_INC_MESSAGEQUEUE_HPP_ */
