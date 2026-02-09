/*
 * commandinterface.cpp
 *
 *  Created on: Jan 24, 2026
 *      Author: matt
 */

#include "commandinterface.hpp"

CommandInterface::CommandInterface(OsalThread *lOwner, size_t lInputQueueCapacity, size_t lOutputQueueCapacity)
: mOwner{lOwner}
, mInputQueue(lInputQueueCapacity)
, mOutputQueue(lOutputQueueCapacity)
{
}

CommandInterface::~CommandInterface(void)
{
    mOwner = static_cast<OsalThread *>(nullptr);
}

CommandMessage *CommandInterface::ReceiveMessage(void)
{
    return mOwner->Receive();
}

int CommandInterface::SendMessage(OsalThread *lDestination, CommandMessage *lMessage)
{
    return mOwner->Send(lDestination, lMessage);
}
