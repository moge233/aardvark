/*
 * main.cpp
 *
 *  Created on: Jan 23, 2026
 *      Author: matt
 */

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <ctime>

#include "endpoint.hpp"
#include "monitor.hpp"
#include "osalthread.hpp"
#include "scriptprocessor.hpp"
#include "usbtmc.hpp"
#include "vxi11.hpp"

using namespace std;
using namespace VXI11;

OsalThread *gScriptProcessorThread;
OsalThread *gUsbTmcThread;
OsalThread *gVxi11ServerThread;
#ifdef BUILD_WITH_M4
#include "rpmsg.hpp"
OsalThread *gRpMsgThread;
#endif
#ifdef BUILD_WITH_DISPLAY
#include "display.hpp"
AardvarkDisplay *gDisplay;
#endif
OsalThread *gUsbTmcMonitorThread;
std::atomic<bool> gStop{false};
UsbTmc gUsbTmc;
VXI11Server gVxi11Server("192.168.1.196");

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
#ifdef BUILD_WITH_M4
static void *RpMsgThreadFxn(void *lArg);
#endif
static void *UsbTmcMonitorThreadFxn(void *lArg);
static void *Vxi11ServerThreadFxn(void *lArg);

int main(int argc, char *argv[])
{
	(void) argc; // Unused
	(void) argv; // Unused

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

    return 0;
}
#endif

static void CreateThreads(void)
{
	gScriptProcessorThread = new OsalThread(10, 1024, ScriptProcessorThreadFxn, static_cast<void *>(nullptr));
	gUsbTmcThread = new OsalThread(8, 1024, UsbTmcThreadFxn, static_cast<void *>(nullptr));
#ifdef BUILD_WITH_M4
	gRpMsgThread = new OsalThread(9, 1024, RpMsgThreadFxn, static_cast<void *>(nullptr));
#endif
	gUsbTmcMonitorThread = new OsalThread(8, 1024, UsbTmcMonitorThreadFxn, static_cast<void *>(nullptr));
	gVxi11ServerThread = new OsalThread(8, 1024, Vxi11ServerThreadFxn, static_cast<void *>(nullptr));
}

static void JoinThreads(void)
{
	gScriptProcessorThread->Join(nullptr);
	gUsbTmcThread->Join(nullptr);
#ifdef BUILD_WITH_M4
	gRpMsgThread->Join(nullptr);
#endif
	gUsbTmcMonitorThread->Join(nullptr);
	gVxi11ServerThread->Join(nullptr);
}

static void DetachThreads(void)
{
	gScriptProcessorThread->Detach();
	gUsbTmcThread->Detach();
#ifdef BUILD_WITH_M4
	gRpMsgThread->Detach();
#endif
	gUsbTmcMonitorThread->Detach();
	gVxi11ServerThread->Detach();
}

static void DeleteThreads(void)
{
	delete gScriptProcessorThread;
	delete gUsbTmcThread;
#ifdef BUILD_WITH_M4
	delete gRpMsgThread;
#endif
	delete gUsbTmcMonitorThread;
	delete gVxi11ServerThread;
}

static void *ScriptProcessorThreadFxn(void *lArg)
{
	if (!lArg)
	{
		exit(EXIT_FAILURE);
	}

	OsalThread *lThisThread = reinterpret_cast<OsalThread *>(lArg);
	(void) lThisThread; // Unused
	while (!gStop)
	{
		// Wait for an incoming message from another endpoint
		CommandMessage *lMessage = gScriptProcessor->Receive();
		if (lMessage)
		{
			gScriptProcessor->HandleCommand(lMessage->GetData(), lMessage->GetLength());
	
			if (gScriptProcessor->GetCount() > 0)
			{
				// There is data for us to send back
				CommandMessage *lReplyMessage = gScriptProcessor->BuildMessage(gScriptProcessor->GetData(),
																			   gScriptProcessor->GetCount(),
																			   reinterpret_cast<Endpoint *>(lMessage->GetOrigin()));
				gScriptProcessor->Send(lReplyMessage);
	
				gScriptProcessor->ClearData();
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
	(void) lThisThread; // Unused
	while(!gStop)
	{
		if(gUsbTmc.GetFileDescriptor())
		{
			if (gUsbTmc.Poll())
			{
				gadget_tmc_header lHeader;
				if (gUsbTmc.GetHeader(&lHeader))
				{
					switch (lHeader.MsgID)
					{
						case GADGET_TMC_DEV_DEP_MSG_OUT:
						case GADGET_TMC_VENDOR_SPECIFIC_OUT:
						{
							string lData = gUsbTmc.ServiceBulkOut(&lHeader);
							CommandMessage *lMessage = gUsbTmc.BuildMessage(lData, gScriptProcessor);
							gUsbTmc.Send(lMessage);
							break;
						}
						case GADGET_TMC_REQUEST_DEV_DEP_MSG_IN:
						case GADGET_TMC_REQUEST_VENDOR_SPECIFIC_IN:
						{
							CommandMessage *lMessage = gUsbTmc.Receive();
							if (lMessage)
							{
								gUsbTmc.ServiceBulkIn(lMessage->GetData());
								delete lMessage;
							}
							break;
						}
						case GADGET_TMC488_TRIGGER:
							break;
					}
				}
			}
		}
	}

	return nullptr;
}

#ifdef BUILD_WITH_M4
static void *RpMsgThreadFxn(void *lArg)
{
	if (!lArg)
	{
		exit(EXIT_FAILURE);
	}

	OsalThread *lThisThread = reinterpret_cast<OsalThread *>(lArg);
	RpMsg lRpMsg;
	const struct timespec cSleepTime{
		.tv_sec{2},
		.tv_nsec{500000000}
	};
	while (!gStop)
	{
		struct timespec lRemainingTime;
		int lError = nanosleep(&cSleepTime, &lRemainingTime);
		while (lError == -1 && errno == EINTR)
		{
			lError = nanosleep(&lRemainingTime, &lRemainingTime);
		}
		lRpMsg.Write(string("led.state = not led.state\n\r"));
		string lEchoBack = lRpMsg.Read();
		CommandMessage *lMessage = lRpMsg.BuildMessage(lEchoBack, gScriptProcessor);
		lRpMsg.Send(lMessage);
	}

	return nullptr;
}
#endif

static void *UsbTmcMonitorThreadFxn(void *lArg)
{
	if (!lArg)
	{
		exit(EXIT_FAILURE);
	}

	OsalThread *lThisThread = reinterpret_cast<OsalThread *>(lArg);
	(void) lThisThread; // Unused
	while (!gStop)
	{
		MonitorEvent *lEvent = gUsbTmc.WaitForEvent();
		gadget_tmc488_localremote_state lRemoteLocalState = gUsbTmc.GetRemoteLocalState();
		uint8_t lRemoteEnable = gUsbTmc.GetREN();
		(void) lRemoteLocalState; // Unused
		(void) lRemoteEnable; // Unused
		delete lEvent;
	}

	return nullptr;
}

static void *Vxi11ServerThreadFxn(void *lArg)
{
	if (!lArg)
	{
		exit(EXIT_FAILURE);
	}

	OsalThread *lThisThread = reinterpret_cast<OsalThread *>(lArg);
	(void) lThisThread; // Unused
	while (!gStop)
	{
		gVxi11Server.Main();
	}

	return nullptr;
}

void SignalHandler(int lSignal)
{
	switch (lSignal)
	{
		case SIGINT:
		case SIGTERM:
			gScriptProcessorThread->Cancel();
			gUsbTmcThread->Cancel();
#ifdef BUILD_WITH_M4
			gRpMsgThread->Cancel();
#endif
			gUsbTmcMonitorThread->Cancel();
			gVxi11ServerThread->Cancel();
			gStop = true;
		break;

		case SIGUSR1:
		break;

		case SIGUSR2:
		break;
	}
}
