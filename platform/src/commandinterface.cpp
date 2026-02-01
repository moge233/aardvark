/*
 * commandinterface.cpp
 *
 *  Created on: Jan 24, 2026
 *      Author: matt
 */

#include "commandinterface.hpp"

CommandInterface::CommandInterface(void *lOwner, size_t lInputQueueCapacity, size_t lOutputQueueCapacity)
: mOwner{lOwner}
, mInputQueue(lInputQueueCapacity)
, mOutputQueue(lOutputQueueCapacity)
{
}

CommandInterface::~CommandInterface(void)
{
    mOwner = static_cast<void *>(nullptr);
}

CommandMessage *CommandInterface::ReceiveMessage(void)
{
    OsalThread *lOwnerThread = reinterpret_cast<OsalThread *>(mOwner);
    return lOwnerThread->Receive();
}

int CommandInterface::SendMessage(OsalThread *lDestination, CommandMessage *lMessage)
{
    OsalThread *lOwnerThread = reinterpret_cast<OsalThread *>(mOwner);
    return lOwnerThread->Send(lDestination, lMessage);
}
