
#ifndef ENDPOINT_HPP_
#define ENDPOINT_HPP_

#include <cassert>
#include <string>

#include <semaphore.h>

#include "circulbarbuffer.hpp"
#include "commandmessage.hpp"

using namespace std;

class Endpoint
{
    public:
        Endpoint(void);
        ~Endpoint();
        CommandMessage *Receive(void);
        int Send(CommandMessage *lMessage);

        inline CommandMessage *BuildMessage(string &lData, void *lDestination) { return BuildMessage(lData.c_str(), lData.length(), lDestination); }
        inline CommandMessage *BuildMessage(const char *lData, size_t lLength, void *lDestination) { return new CommandMessage(lData, lLength, reinterpret_cast<void *>(this), lDestination); }

    private:
        pthread_mutex_t mLock;
        pthread_cond_t mCondition;
        CircularBuffer mMessageQueue;

        inline int Lock(void) { return pthread_mutex_lock(&mLock); }
        inline int Unlock(void) { return pthread_mutex_unlock(&mLock); }
        inline int ConditionWait(void) { return pthread_cond_wait(&mCondition, &mLock); }
        inline int ConditionSignal(void) { return pthread_cond_signal(&mCondition); }
        inline int Wait(void) { int lError{0}; while(mMessageQueue.IsEmpty()) { lError = ConditionWait(); } return lError; }
        inline int Post(void) { int lError = ConditionSignal(); return lError; }
};

#endif // ENDPOINT_HPP_
