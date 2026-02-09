
#include <cstdlib>

#include "endpoint.hpp"

Endpoint::Endpoint(void)
: mInputQueue(64)
{
    int lError = sem_init(&mSemaphore, 0, 0);
    if (lError)
    {
        exit(EXIT_FAILURE);
    }
}

Endpoint::~Endpoint()
{
    sem_destroy(&mSemaphore);
}

CommandMessage *Endpoint::Receive(void)
{
    Wait();
    CommandMessage *lMessage = mInputQueue.Get();
    return lMessage;
}

int Endpoint::Send(Endpoint *lDestination, CommandMessage *lMessage)
{
    lDestination->mInputQueue.Put(lMessage);
    int lError = lDestination->Post();
    return lError;
}
