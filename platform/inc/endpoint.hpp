
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

        CommandMessage *BuildMessage(string &lData);
        CommandMessage *BuildMessage(const char *lData, size_t lLength);
        
        inline int Count(void) { int lVal{0}; sem_getvalue(&mSemaphore, &lVal); return lVal; }
    private:
        sem_t mSemaphore;
        CircularBuffer mInputQueue;

        inline int Wait(void) { return sem_wait(&mSemaphore); }
        inline int Post(void) { return sem_post(&mSemaphore); }
};

#endif // ENDPOINT_HPP_
