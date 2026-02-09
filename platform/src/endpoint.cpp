
#include <cstdlib>

#include "endpoint.hpp"

Endpoint::Endpoint(void)
: mMessageQueue(64)
{
    pthread_mutex_init(&mLock, nullptr);
    pthread_cond_init(&mCondition, nullptr);
}

Endpoint::~Endpoint()
{
    Post();
    Unlock();
    pthread_mutex_destroy(&mLock);
    pthread_cond_destroy(&mCondition);
}

CommandMessage *Endpoint::Receive(void)
{
    Lock();
    Wait();
    CommandMessage *lMessage = mMessageQueue.Get();
    Unlock();
    return lMessage;
}

int Endpoint::Send(CommandMessage *lMessage)
{
    Endpoint *lDestination = reinterpret_cast<Endpoint *>(lMessage->GetDestination());
    lDestination->Lock();
    lDestination->mMessageQueue.Put(lMessage);
    lDestination->Post();
    lDestination->Unlock();
    return 0;
}
