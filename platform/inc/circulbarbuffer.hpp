/*
 * circularbuffer.hpp
 *
 *  Created on: Jan 24, 2026
 *      Author: matt
 */

#ifndef AARDVARK_PLATFORM_INC_CIRCULARBUFFER_HPP_
#define AARDVARK_PLATFORM_INC_CIRCULARBUFFER_HPP_

#include <pthread.h>

#include "commandmessage.hpp"

class CircularBuffer
{
public:
    CircularBuffer(size_t lCapacity);
    ~CircularBuffer();
    CircularBuffer(CircularBuffer& lOther) = delete;
    CircularBuffer& operator=(CircularBuffer& lOther) = delete;
    void Put(CommandMessage *lMessage);
    CommandMessage *Get(void);
    inline bool IsEmpty(void) { return mGetIndex == mPutIndex; }
private:
    size_t mCapacity;
    size_t mGetIndex;
    size_t mPutIndex;
    pthread_mutex_t mLock;
    CommandMessage **mMessages;

    inline int Lock(void) { return pthread_mutex_lock(&mLock); }
    inline int Unlock(void) { return pthread_mutex_unlock(&mLock); }
};

#endif // AARDVARK_PLATFORM_INC_CIRCULARBUFFER_HPP_
