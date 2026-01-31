/*
 * commandqueue.cpp
 *
 *  Created on: Jan 24, 2026
 *      Author: matt
 */

#include <iostream>
#include <cstdint>
#include <cstdio>

#include "commandmessage.hpp"
#include "messagepipe.hpp"


MessagePipe::MessagePipe(const char *lName, const long lCapacity, PipeType lType)
: mName{lName}
, mCapacity{lCapacity}
, mType(lType)
{
	if ((mType == MASTER_INPUT) || (mType == MASTER_OUTPUT))
	{
		constexpr mode_t lMode{0644};
		mq_attr lAttributes = {
			.mq_flags = 0,
			.mq_maxmsg = mCapacity,
			.mq_msgsize = sizeof(CommandMessage),
			.mq_curmsgs = 0,
		};
		int lFlags = (mType == MASTER_INPUT) ? O_CREAT | O_RDONLY : O_CREAT | O_WRONLY;

		mQueue = mq_open(mName, lFlags, lMode, &lAttributes);
	}
	else
	{
		int lFlags = (mType == CLIENT_INPUT) ? O_RDONLY : O_WRONLY;

		mQueue = mq_open(mName, lFlags);
	}

	if (mQueue == static_cast<mqd_t>(-1))
	{
		perror("could not open input queue for message pipe");
		exit(EXIT_FAILURE);
	}
}

MessagePipe::~MessagePipe()
{
	if ((mType == MASTER_INPUT) || (mType == MASTER_OUTPUT))
	{
		mq_close(mQueue);
		mq_unlink(mName);
	}
	else
	{
		mq_close(mQueue);
	}
}

ssize_t MessagePipe::Receive(CommandMessage *lMessage)
{
	if ((mType == PipeType::MASTER_OUTPUT) || (mType == PipeType::CLIENT_OUTPUT))
	{
		return -1;
	}
	else
	{
		ssize_t lRet = mq_receive(mQueue, reinterpret_cast<char *>(lMessage), sizeof(CommandMessage), nullptr);
		return lRet;
	}
}

int MessagePipe::Send(CommandMessage *lMessage)
{
	if ((mType == PipeType::MASTER_INPUT) || (mType == PipeType::CLIENT_INPUT))
	{
		return -1;
	}
	else
	{
		return mq_send(mQueue, reinterpret_cast<char *>(lMessage), sizeof(CommandMessage), 0);
	}
}