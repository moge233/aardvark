/*
 * blockingqueue.hpp
 *
 *  Created on: Jan 24, 2026
 *      Author: matt
 */

#ifndef AARDVARK_PLATFORM_INC_BLOCKINGQUEUE_HPP_
#define AARDVARK_PLATFORM_INC_BLOCKINGQUEUE_HPP_

#include <condition_variable>
#include <mutex>
#include <queue>

#include "commandmessage.hpp"

class BlockingQueue
{
public:
    explicit BlockingQueue(size_t lCapacity);
    ~BlockingQueue() { }
    void Push(CommandMessage *lMessage);
    CommandMessage *Pop(void);
private:
    const size_t mCapacity;
    std::condition_variable mCondition;
    std::mutex mMutex;
    std::queue<CommandMessage *> mQueue;
};

#endif  // AARDVARK_PLATFORM_INC_BLOCKINGQUEUE_HPP_