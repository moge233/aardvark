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

#include "display.hpp"

OsalThread *gScriptProcessorThread;
OsalThread *gUsbTmcThread;
ScriptProcessor *gScriptProcessor;
#ifdef BUILD_WITH_DISPLAY
AardvarkDisplay *gDisplay;
#endif
std::atomic<bool> gStop{false};

#ifdef DAEMONIZE
static int InitDaemon(void);
#endif
static void CreateThreads(void);
static void JoinThreads(void);
static void DetachThreads(void);
static void DeleteThreads(void);
static void SignalHandler(int lSignal);
static void *ScriptProcessorThreadFxn(void *lArg);
static void *UsbTmcThreadFxn(void *lArg);

int main(int argc, char *argv[])
{
	signal(SIGINT, SignalHandler);
	signal(SIGTERM, SignalHandler);
	signal(SIGUSR1, SignalHandler);
	signal(SIGUSR2, SignalHandler);

#ifdef BUILD_WITH_DISPLAY
	gDisplay = new AardvarkDisplay(argc, argv);
#endif

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
	gScriptProcessorThread = new OsalThread(10, 1024, ScriptProcessorThreadFxn, static_cast<void *>(nullptr));
	gUsbTmcThread = new OsalThread(8, 1024, UsbTmcThreadFxn, static_cast<void *>(nullptr));
}

static void JoinThreads(void)
{
	gScriptProcessorThread->Join(nullptr);
	gUsbTmcThread->Join(nullptr);
}

static void DetachThreads(void)
{
	gScriptProcessorThread->Detach();
	gUsbTmcThread->Detach();
}

static void DeleteThreads(void)
{
	delete gScriptProcessorThread;
	delete gUsbTmcThread;
}

static void *ScriptProcessorThreadFxn(void *lArg)
{
	if (!lArg)
	{
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
				CommandMessage *lReplyMessage = new CommandMessage(gScriptProcessor->GetData(),
																   gScriptProcessor->GetCount(),
																   reinterpret_cast<void *>(lThisThread));
				OsalThread *lDestination = reinterpret_cast<OsalThread *>(lMessage->GetOrigin());
				lThisThread->Send(lDestination, lReplyMessage);

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
	if (!lArg)
	{
		exit(EXIT_FAILURE);
	}

	OsalThread *lThisThread = reinterpret_cast<OsalThread *>(lArg);
	UsbTmc *lUsbTmc = new UsbTmc(lThisThread);
	if (!lUsbTmc)
	{
		exit(EXIT_FAILURE);
	}
	while(!gStop)
	{
		if(lUsbTmc->GetFileDescriptor())
		{
			if (lUsbTmc->Poll())
			{
				gadget_tmc_header lHeader;
				if (lUsbTmc->GetHeader(&lHeader))
				{
					switch (lHeader.MsgID)
					{
						case GADGET_TMC_DEV_DEP_MSG_OUT:
						case GADGET_TMC_VENDOR_SPECIFIC_OUT:
							lUsbTmc->ServiceBulkOut(&lHeader);
							break;
						case GADGET_TMC_REQUEST_DEV_DEP_MSG_IN:
						case GADGET_TMC_REQUEST_VENDOR_SPECIFIC_IN:
							lUsbTmc->ServiceBulkIn(&lHeader);
							break;
						case GADGET_TMC488_TRIGGER:
							break;
					}
				}
			}
		}
	}

	delete lUsbTmc;

	return nullptr;
}

void SignalHandler(int lSignal)
{
	switch (lSignal)
	{
		case SIGINT:
		case SIGTERM:
			gStop = true;
			gScriptProcessorThread->Cancel();
			gUsbTmcThread->Cancel();
		break;

		case SIGUSR1:
		break;

		case SIGUSR2:
		break;
	}
}