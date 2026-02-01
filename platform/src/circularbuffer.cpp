/*
 * circularbuffer.cpp
 *
 *  Created on: Oct 5, 2024
 *      Author: matt
 */

#include <cstdint>
#include <iostream>

#include "circulbarbuffer.hpp"

CircularBuffer::CircularBuffer(size_t lCapacity)
: mCapacity{lCapacity}
, mGetIndex{0}
, mPutIndex{0}
{
    mMessages = new CommandMessage * [mCapacity];
    if (!mMessages)
    {
        exit(EXIT_FAILURE);
    }

    mLock = PTHREAD_MUTEX_INITIALIZER;
}

CircularBuffer::~CircularBuffer()
{
    if (mMessages)
    {
        delete [] mMessages;
    }
}

CommandMessage *CircularBuffer::Get(void)
{
    Lock();
    CommandMessage *lReturn = mMessages[mGetIndex];
    mMessages[mGetIndex] = static_cast<CommandMessage *>(nullptr);
    Unlock();
    if (++mGetIndex == mCapacity)
    {
        mGetIndex = 0;
    }
    return lReturn;
}

void CircularBuffer::Put(CommandMessage *lMessage)
{
    Lock();
    mMessages[mPutIndex] = lMessage;
    Unlock();
    if (++mPutIndex == mCapacity)
    {
        mPutIndex = 0;
    }
}
