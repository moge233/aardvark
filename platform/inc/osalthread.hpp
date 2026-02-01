

#ifndef OSALTHREAD_HPP
#define OSALTHREAD_HPP

#include <cstddef>
#include <cstdint>
#include <pthread.h>
#include <semaphore.h>

#include "circulbarbuffer.hpp"
#include "commandmessage.hpp"

class OsalThread
{
    typedef void *(*OsalThreadFxn)(void *);

    typedef enum _SignalEnum
    {
        SIGNAL_DATA_AVAILABLE = 1,
    } SignalEnum;

    public:
        OsalThread(uint32_t lPriority, size_t lStackSize, OsalThreadFxn lFxn, void *lArg);
        ~OsalThread();
        int Join(void **lThreadReturn);
        int Cancel(void);
        int Detach(void);
        CommandMessage *Receive(void);
        int Send(OsalThread *lDestination, CommandMessage *lMessage);
    private:
        uint32_t mPriority;
        size_t mStackSize;
        OsalThreadFxn mFxn;
        void *mArg;
        pthread_t mThread;
        pthread_attr_t mAttributes;
        sem_t mSemaphore;
        CircularBuffer mInputQueue;
        CircularBuffer mOutputQueue;

        inline int Wait(void) { return sem_wait(&mSemaphore); }
        inline int Post(void) { return sem_post(&mSemaphore); }
};

#endif // OSALTHREAD_HPP
