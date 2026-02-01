
#include <iostream>

#include "osalthread.hpp"

OsalThread::OsalThread(uint32_t lPriority, size_t lStackSize, OsalThreadFxn lFxn, void *lArg)
: mPriority{lPriority}
, mStackSize{lStackSize}
, mFxn{lFxn}
, mArg{lArg}
, mInputQueue(64)
, mOutputQueue(64)
{
    if (mStackSize < PTHREAD_STACK_MIN)
    {
        mStackSize = PTHREAD_STACK_MIN;
    }

    if (!mArg)
    {
        mArg = static_cast<void *>(this);
    }

	struct sched_param lSchedParam;
	lSchedParam.sched_priority = mPriority;
    int lError{0};

	lError = pthread_attr_init(&mAttributes);
    if (lError)
    {
        std::cerr << "could not initialize pthread attributes (ERRNO: " << lError << ")" << std::endl;
        exit(EXIT_FAILURE);
    }

	lError = pthread_attr_setstacksize(&mAttributes, mStackSize);
    if (lError)
    {
        std::cerr << "could not set stack size in attributes (ERRNO: " << lError << ")" << std::endl;
        exit(EXIT_FAILURE);
    }

	lError = pthread_attr_setinheritsched(&mAttributes, PTHREAD_EXPLICIT_SCHED);
    if (lError)
    {
        std::cerr << "could not set inherit schedule in attributes" << std::endl;
        exit(EXIT_FAILURE);
    }

	lError = pthread_attr_setschedpolicy(&mAttributes, SCHED_RR);
    if (lError)
    {
        std::cerr << "could not set schedule policy" << std::endl;
        exit(EXIT_FAILURE);
    }

	lError = pthread_attr_setschedparam(&mAttributes, &lSchedParam);
    if (lError)
    {
        std::cerr << "could not set schedule paramters" << std::endl;
        exit(EXIT_FAILURE);
    }

	lError = pthread_create(&mThread, &mAttributes, mFxn, mArg);
    if (lError)
    {
        std::cerr << "could not set create thread" << std::endl;
        exit(EXIT_FAILURE);
    }

    lError = sem_init(&mSemaphore, 0, 0);
    if (lError)
    {
        std::cerr << "could not initialize semaphore" << std::endl;
        exit(EXIT_FAILURE);
    }
}

OsalThread::~OsalThread()
{
    ;
}

int OsalThread::Join(void **lThreadReturn)
{
    return pthread_join(mThread, lThreadReturn);
}

int OsalThread::Cancel(void)
{
    return pthread_cancel(mThread);
}

int OsalThread::Detach(void)
{
    return pthread_detach(mThread);
}

CommandMessage *OsalThread::Receive(void)
{
    Wait();
    CommandMessage *lMessage = mInputQueue.Get();
    return lMessage;
}

int OsalThread::Send(OsalThread *lDestination, CommandMessage *lMessage)
{
    lDestination->mInputQueue.Put(lMessage);
    int lError = lDestination->Post();
    return lError;
}
