
#ifndef ENDPOINT_HPP_
#define ENDPOINT_HPP_

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
        int Send(Endpoint *lDestination, CommandMessage *lMessage);

        inline CommandMessage *BuildMessage(string &lData) { return BuildMessage(lData.c_str(), lData.length()); }
        inline CommandMessage *BuildMessage(const char *lData, size_t lLength) { return new CommandMessage(lData, lLength, reinterpret_cast<void *>(this)); }
        
        inline int Count(void) { int lVal{0}; sem_getvalue(&mSemaphore, &lVal); return lVal; }
    private:
        sem_t mSemaphore;
        CircularBuffer mInputQueue;

        inline int Wait(void) { return sem_wait(&mSemaphore); }
        inline int Post(void) { return sem_post(&mSemaphore); }
};

#endif // ENDPOINT_HPP_
