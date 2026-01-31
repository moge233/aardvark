/*
 * main.cpp
 *
 *  Created on: Jan 23, 2026
 *      Author: matt
 */


#include <atomic>
#include <cstdlib>
#include <iostream>

#include <pthread.h>

#include "blockingqueue.hpp"
#include "scriptprocessor.hpp"
#include "usbtmc.hpp"

constexpr size_t MAX_THREADS = 5;
pthread_t gScriptProcessorThread;
static void *ScriptProcessorThreadFxn(void *lArg);
pthread_t gUsbTmcThread;
static void *UsbTmcThreadFxn(void *lArg);

static int InitDaemon(void);
static void ConfigureThread(pthread_attr_t *lAttributes, size_t lStackSize, int lPriority);
static void CreateThread(pthread_t *lThread, pthread_attr_t *lAttributes, void *(*lFxn)(void *), void *lArg);
static void CreateThreads(void);
static void StartThreads(void);
static void SignalHandler(int lSignal);

ScriptProcessor *gScriptProcessor;
UsbTmc *gUsbTmc;
std::atomic<bool> gStop{false};

int main(int argc, char *argv[])
{
	signal(SIGINT, SignalHandler);
	signal(SIGTERM, SignalHandler);
	signal(SIGUSR1, SignalHandler);
	signal(SIGUSR2, SignalHandler);

#ifdef DAEMONIZE
	InitDaemon();
#else
	gScriptProcessor = new ScriptProcessor;
	gUsbTmc = new UsbTmc;

	if (!gScriptProcessor || !gUsbTmc)
	{
		exit(EXIT_FAILURE);
	}

	gScriptProcessor->StartLua();

	CreateThreads();
	StartThreads();

	if (gScriptProcessor)
	{
		delete gScriptProcessor;
	}
	if (gUsbTmc)
	{
		delete gUsbTmc;
	}

	pthread_detach(gScriptProcessorThread);
	pthread_detach(gUsbTmcThread);

	pthread_exit(nullptr);
#endif
	return 0;
}

#ifdef DAEMONIZE
static int InitDaemon(void)
{
    pid_t lPid = fork();
    if(lPid < 0)
    {
        perror("could not fork() for daemon");
        return lPid;
    }
    if(lPid > 0)
    {
        return -1;
    }

	gScriptProcessor = new ScriptProcessor;
	gUsbTmc = new UsbTmc;

	gScriptProcessor->StartLua();

    if(setsid() < 0)
    {
        perror("could not set SID for daemon");
        return -1;
    }

	CreateThreads();
    StartThreads();

    return 0;
}
#endif

void ConfigureThread(pthread_attr_t *lAttributes, size_t lStackSize, int lPriority)
{
	struct sched_param lSchedParam;
	lSchedParam.sched_priority = lPriority;

	pthread_attr_init(lAttributes);
	pthread_attr_setstacksize(lAttributes, lStackSize);
	pthread_attr_setinheritsched(lAttributes, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(lAttributes, SCHED_RR);
	pthread_attr_setschedparam(lAttributes, &lSchedParam);
}

void CreateThread(pthread_t *lThread, pthread_attr_t *lAttributes, void *(*lFxn)(void *), void *lArg)
{
	pthread_create(lThread, lAttributes, lFxn, lArg);
}

void CreateThreads(void)
{
	constexpr size_t DEFAULT_STACK_SIZE{1024};
	int lScriptProcessorThreadPriority{32};
	int lUsbTmcThreadPriority{22};
	pthread_attr_t lScriptProcessorThreadAttr;
	pthread_attr_t lUsbTmcThreadAttr;

	ConfigureThread(&lScriptProcessorThreadAttr, DEFAULT_STACK_SIZE, lScriptProcessorThreadPriority);
	ConfigureThread(&lUsbTmcThreadAttr, DEFAULT_STACK_SIZE, lUsbTmcThreadPriority);

	CreateThread(&gScriptProcessorThread, &lScriptProcessorThreadAttr, ScriptProcessorThreadFxn, nullptr);
	CreateThread(&gUsbTmcThread, &lUsbTmcThreadAttr, UsbTmcThreadFxn, nullptr);
}

void StartThreads(void)
{
	pthread_join(gScriptProcessorThread, nullptr);
	pthread_join(gUsbTmcThread, nullptr);
}

static void *ScriptProcessorThreadFxn(void *lArg)
{
	while (!gStop)
	{
		// Receive message in queue; block if not available;
		CommandMessage *lMessage = gScriptProcessor->Receive();
		if (lMessage)
		{
			gScriptProcessor->HandleCommand(lMessage->GetData(), lMessage->GetLength());

			if (gScriptProcessor->GetCount() > 0)
			{
				CommandMessage *lReplyMessage = new CommandMessage(gScriptProcessor->GetData(), gScriptProcessor->GetCount());
				gScriptProcessor->Send(lReplyMessage);

				gScriptProcessor->ClearData();
				gScriptProcessor->ClearCount();
			}

			delete lMessage;
		}
	}

	return nullptr;
}

static void *UsbTmcThreadFxn(void *lArg)
{
	while(!gStop)
	{
		gUsbTmc->Main();
	}

	return nullptr;
}

void SignalHandler(int lSignal)
{
	switch (lSignal)
	{
		case SIGINT:
		case SIGTERM:
		gStop = true;
		pthread_cancel(gScriptProcessorThread);
		pthread_cancel(gUsbTmcThread);
		break;

		case SIGUSR1:
		break;

		case SIGUSR2:
		break;
	}
}