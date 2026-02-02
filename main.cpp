/*
 * main.cpp
 *
 *  Created on: Jan 23, 2026
 *      Author: matt
 */

#include <atomic>
#include <cstdlib>
#include <iostream>
#include <ctime>

#include "osalthread.hpp"
#include "scriptprocessor.hpp"
#include "usbtmc.hpp"

static void *ThreadFxn1(void *lArg);
static void *ThreadFxn2(void *lArg);
static void *ThreadFxn3(void *lArg);
static void *ScriptProcessorThreadFxn2(void *lArg);
static void *UsbTmcThreadFxn2(void *lArg);
OsalThread *gThread1;
OsalThread *gThread2;
OsalThread *gThread3;
OsalThread *gScriptProcessorThread2;
OsalThread *gUsbTmcThread2;

static int InitDaemon(void);
static void CreateThreads(void);
static void JoinThreads(void);
static void DetachThreads(void);
static void DeleteThreads(void);
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

	if (!gScriptProcessor)
	{
		return EXIT_FAILURE;
	}

	gScriptProcessor->StartLua();

	CreateThreads();

	JoinThreads();

	DetachThreads();

	DeleteThreads();

	if (gScriptProcessor)
	{
		delete gScriptProcessor;
	}

	if (gUsbTmc)
	{
		delete gUsbTmc;
	}
#endif // DAEMONIZE
	return EXIT_SUCCESS;
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

	gScriptProcessor->StartLua();

    if(setsid() < 0)
    {
        perror("could not set SID for daemon");
        return -1;
    }

	CreateThreads();

	JoinThreads();

	DetachThreads();

	DeleteThreads();

	if (gScriptProcessor)
	{
		delete gScriptProcessor;
	}

	if (gUsbTmc)
	{
		delete gUsbTmc;
	}

    return 0;
}
#endif

static void CreateThreads(void)
{
	gThread1 = new OsalThread(5, 1024, ThreadFxn1, static_cast<void *>(nullptr));
	gThread2 = new OsalThread(4, 1024, ThreadFxn2, static_cast<void *>(nullptr));
	gThread3 = new OsalThread(3, 1024, ThreadFxn3, static_cast<void *>(nullptr));
	gScriptProcessorThread2 = new OsalThread(10, 1024, ScriptProcessorThreadFxn2, static_cast<void *>(nullptr));
	gUsbTmcThread2 = new OsalThread(8, 1024, UsbTmcThreadFxn2, static_cast<void *>(nullptr));
}

static void JoinThreads(void)
{
	gThread1->Join(nullptr);
	gThread2->Join(nullptr);
	gThread3->Join(nullptr);
	gScriptProcessorThread2->Join(nullptr);
	gUsbTmcThread2->Join(nullptr);
}

static void DetachThreads(void)
{
	gThread1->Detach();
	gThread2->Detach();
	gThread3->Detach();
	gScriptProcessorThread2->Detach();
	gUsbTmcThread2->Detach();
}

static void DeleteThreads(void)
{
	delete gThread1;
	delete gThread2;
	delete gThread3;
	delete gScriptProcessorThread2;
	delete gUsbTmcThread2;
}

static void *ThreadFxn1(void *lArg)
{
	if (!lArg)
	{
		std::cout << __func__ << ": lArg is null; exiting..." << std::endl;
		exit(EXIT_FAILURE);
	}
	OsalThread *lThisThread = reinterpret_cast<OsalThread *>(lArg);
	while (!gStop)
	{
		char lBuffer[1024]{{}};
		CommandMessage *lMessage = lThisThread->Receive();
		if (lMessage)
		{
			std::cout << "Thread 1 wakes up with a message: " << lMessage->GetData() << std::endl;
			delete lMessage;
		}
		else
		{
			std::cout << "Thread 1 woke up and received a null message (" << lMessage << ")" << std::endl;
		}
	}

	return static_cast<void *>(nullptr);
}

static void *ThreadFxn2(void *lArg)
{
	if (!lArg)
	{
		std::cout << __func__ << ": lArg is null; exiting..." << std::endl;
		exit(EXIT_FAILURE);
	}
	OsalThread *lThisThread = reinterpret_cast<OsalThread *>(lArg);
	static size_t lCounter{0};
	while (!gStop)
	{
		struct timespec lTimespec;
		lTimespec.tv_sec = 2;
		lTimespec.tv_nsec = 250000000;
		std::cout << "Thread 2 sleeping" << std::endl;
		while (nanosleep(&lTimespec, &lTimespec) == -1 && errno == EINTR)
		{
			continue;
		}
		if (++lCounter == 3)
		{
			lCounter = 0;
			const char lBuffer[] = "Hello, Thread 1! From, Thread 2";
			CommandMessage *lMessage = new CommandMessage(lBuffer, strlen(lBuffer));
			std::cout << "Thread 2 wakes up and signals Thread 1" << std::endl;
			lThisThread->Send(gThread1, lMessage);
		}
		else
		{
			std::cout << "Thread 2 wakes up but does not signal Thread 1" << std::endl;
		}
	}

	return static_cast<void *>(nullptr);
}

static void *ThreadFxn3(void *lArg)
{
	if (!lArg)
	{
		std::cout << __func__ << ": lArg is null; exiting..." << std::endl;
		exit(EXIT_FAILURE);
	}
	OsalThread *lThisThread = reinterpret_cast<OsalThread *>(lArg);
	static size_t lCounter{0};
	while (!gStop)
	{
		struct timespec lTimespec;
		lTimespec.tv_sec = 1;
		lTimespec.tv_nsec = 0;
		std::cout << "Thread 3 sleeping" << std::endl;
		while (nanosleep(&lTimespec, &lTimespec) == -1 && errno == EINTR)
		{
			continue;
		}
		if (++lCounter == 5)
		{
			lCounter = 0;
			const char lBuffer[] = "Hello, Thread 1! From, Thread 3";
			CommandMessage *lMessage = new CommandMessage(lBuffer, strlen(lBuffer));
			std::cout << "Thread 3 wakes up and signals Thread 1" << std::endl;
			lThisThread->Send(gThread1, lMessage);
		}
		else
		{
			std::cout << "Thread 3 wakes up but does not signal Thread 1" << std::endl;
		}
	}

	return static_cast<void *>(nullptr);
}

static void *ScriptProcessorThreadFxn2(void *lArg)
{
	if (!lArg)
	{
		std::cout << __func__ << ": lArg is null; exiting..." << std::endl;
		exit(EXIT_FAILURE);
	}
	OsalThread *lThisThread = reinterpret_cast<OsalThread *>(lArg);
	while (!gStop)
	{
		// Receive message in queue; block if not available;
		CommandMessage *lMessage = lThisThread->Receive();
		if (lMessage)
		{
			gScriptProcessor->HandleCommand(lMessage->GetData(), lMessage->GetLength());

			if (gScriptProcessor->GetCount() > 0)
			{
				CommandMessage *lReplyMessage = new CommandMessage(gScriptProcessor->GetData(), gScriptProcessor->GetCount());
				lThisThread->Send(gUsbTmcThread2, lReplyMessage);

				gScriptProcessor->ClearData();
				gScriptProcessor->ClearCount();
			}

			delete lMessage;
		}
	}

	return nullptr;
}

static void *UsbTmcThreadFxn2(void *lArg)
{
	if (!lArg)
	{
		std::cout << __func__ << ": lArg is null; exiting..." << std::endl;
		exit(EXIT_FAILURE);
	}
	gUsbTmc = new UsbTmc(reinterpret_cast<OsalThread *>(lArg));
	if (!gUsbTmc)
	{
		std::cout << __func__ << ": could not create USB TMC instance; exiting..." << std::endl;
		exit(EXIT_FAILURE);
	}
	while(!gStop)
	{
		if(gUsbTmc->GetFileDescriptor())
		{
			if (gUsbTmc->Poll())
			{
				gadget_tmc_header lHeader;
				if (gUsbTmc->GetHeader(&lHeader))
				{
					switch (lHeader.MsgID)
					{
						case GADGET_TMC_DEV_DEP_MSG_OUT:
						case GADGET_TMC_VENDOR_SPECIFIC_OUT:
							gUsbTmc->ServiceBulkOut(&lHeader);
							break;
						case GADGET_TMC_REQUEST_DEV_DEP_MSG_IN:
						case GADGET_TMC_REQUEST_VENDOR_SPECIFIC_IN:
							gUsbTmc->ServiceBulkIn(&lHeader);
							break;
						case GADGET_TMC488_TRIGGER:
							break;
					}
				}
			}
		}
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
			gThread1->Cancel();
			gThread2->Cancel();
			gThread3->Cancel();
			gScriptProcessorThread2->Cancel();
			gUsbTmcThread2->Cancel();
		break;

		case SIGUSR1:
		break;

		case SIGUSR2:
		break;
	}
}