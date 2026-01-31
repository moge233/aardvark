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
    mLock = PTHREAD_MUTEX_INITIALIZER;
    mCondition = PTHREAD_COND_INITIALIZER;

    if (mCapacity)
    {
        mMessages = new CommandMessage * [mCapacity];
    }
}

CircularBuffer::~CircularBuffer()
{
    if (mMessages)
    {
        CommandMessage *lMessage = Get();
        while(lMessage)
        {
            delete lMessage;
        }
        delete [] mMessages;
    }
}

CommandMessage *CircularBuffer::Get(void)
{
    WaitForMessage();
    CommandMessage *lMessage = nullptr;
    if (mGetIndex != mPutIndex)
    {
        lMessage = mMessages[mGetIndex];
        mMessages[mGetIndex] = nullptr;
        ++mGetIndex;

        if (mGetIndex == mCapacity)
        {
            mGetIndex = 0;
        }
    }
    else
    {
        Unlock();
        return nullptr;;
    }

    return lMessage;
}

int CircularBuffer::Put(CommandMessage *lMessage)
{
    mMessages[mPutIndex] = lMessage;
    ++mPutIndex;
    if (mPutIndex == mCapacity)
    {
        mPutIndex = 0;
    }

    SignalMessage();

    return 0;
}

void CircularBuffer::WaitForMessage(void)
{
    if (Lock())
    {
        perror("could not get circular buffer lock");
        return;
    }

    while (IsEmpty())
    {
        if (pthread_cond_wait(&mCondition, &mLock))
        {
            perror("could not wait for circular buffer condition");
        }
    }

    Unlock();
}

void CircularBuffer::SignalMessage(void)
{
    if (Lock())
    {
        perror("could not get circular buffer lock");
        return;
    }

    pthread_cond_broadcast(&mCondition);

    Unlock();
}