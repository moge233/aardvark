/*
 * blockingqueue.cpp
 *
 *  Created on: Mar 15, 2024
 *      Author: matt
 */

 #include "blockingqueue.hpp"

 BlockingQueue::BlockingQueue(size_t lCapacity)
 : mCapacity{lCapacity}
 {

 }

void BlockingQueue::Push(CommandMessage *lMessage)
{
    std::unique_lock<std::mutex> lLock(mMutex);

    this->mCondition.wait(lLock, [=, this] { return this->mQueue.size() < this->mCapacity; });
    this->mQueue.push(lMessage);
    lLock.unlock();
    this->mCondition.notify_one();
}

CommandMessage *BlockingQueue::Pop(void)
{
    std::unique_lock<std::mutex> lLock(mMutex);

    this->mCondition.wait(lLock, [=, this] { return !this->mQueue.empty(); });
    CommandMessage *lRet = mQueue.front();
    this->mQueue.pop();
    lLock.unlock();
    this->mCondition.notify_one();
    return lRet;
}