/*
 * led.cpp
 *
 *  Created on: Apr 30, 2024
 *      Author: matt
 */


#include "led.hpp"

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>


Led gLed;


void LedInstall(lua_State *lState) {
	Lua lLua(lState);

	// MakeTable
	lLua.NewTable();
	lLua.MakeTableReadOnly();

	/*
	 * Stack: (top down)
	 * 		: * led DeviceTable
	 */

	// AddGetter
	lLua.PushString("state");
	lLua.PushLightUserData(&gLed);
	lLua.PushInteger(FUNC_GET);
	lLua.PushCClosure(Led::HandleMsg, 2);

	/*
	 * Stack: (top down)
	 * 		: * HandleMsg "state" led DeviceTable
	 */

	// AddRotMetaObject
	lLua.GetMetaTable(-3);

	/*
	 * Stack: (top down)
	 * 		: * led.__metatable HandleMsg "state" led DeviceTable
	 */
	lLua.PushString("Getters");
	lLua.GetTable(-2);

	/*
	 * Stack: (top down)
	 * 		: * led.__metatable.Getters led.__metatable HandleMsg "state" led DeviceTable
	 */
	lLua.Insert(-4);
	lLua.Pop(1);

	/*
	 * Stack: (top down)
	 * 		: * HandleMsg "state" led.__metatable.Getters led DeviceTable
	 */
	lLua.SetTable(-3);

	/*
	 * Stack: (top down)
	 * 		: * led.__metatable.Getters led DeviceTable
	 */
	lLua.Pop(1);

	/*
	 * Stack: (top down)
	 * 		: * led DeviceTable
	 */

	// AddSetter
	lLua.PushString("state");
	lLua.PushLightUserData(&gLed);
	lLua.PushInteger(FUNC_SET);
	lLua.PushCClosure(Led::HandleMsg, 2);

	/*
	 * Stack: (top down)
	 * 		: * HandleMsg "state" led DeviceTable
	 */

	// AddRotMetaObject
	lLua.GetMetaTable(-3);

	/*
	 * Stack: (top down)
	 * 		: * led.__metatable HandleMsg "state" led DeviceTable
	 */
	lLua.PushString("Setters");
	lLua.GetTable(-2);

	/*
	 * Stack: (top down)
	 * 		: * led.__metatable.Setters led.__metatable HandleMsg "state" led DeviceTable
	 */
	lLua.Insert(-4);
	lLua.Pop(1);

	/*
	 * Stack: (top down)
	 * 		: * HandleMsg "state" led.__metatable.Setters led DeviceTable
	 */
	lLua.SetTable(-3);

	/*
	 * Stack: (top down)
	 * 		: * led.__metatable.Getters led DeviceTable
	 */
	lLua.Pop(1);

	/*
	 * Stack: (top down)
	 * 		: * led DeviceTable
	 */
	lLua.PushString("toggle");
	lLua.PushLightUserData(&gLed);
	lLua.PushCClosure(Led::Toggle, 1);

	/*
	 * Stack: (top down)
	 * 		: * Toggle "toggle" led DeviceTable
	 */
	lLua.GetMetaTable(-3);

	/*
	 * Stack: (top down)
	 * 		: * led.__metatable Toggle "toggle" led DeviceTable
	 */
	lLua.PushString("Objects");
	lLua.GetTable(-2);

	/*
	 * Stack: (top down)
	 * 		: * led.__metatable.Objects led.__metatable Toggle "toggle" led DeviceTable
	 */
	lLua.Insert(-4);
	lLua.Pop(1);

	/*
	 * Stack: (top down)
	 * 		: * Toggle "toggle" led.__metatable.Objects led DeviceTable
	 */
	lLua.SetTable(-3);

	/*
	 * Stack: (top down)
	 * 		: * led.__metatable.Objects led DeviceTable
	 */
	lLua.Pop(1);

	lLua.SetGlobal("led");

	/*
	 * Stack: (top down)
	 * 		: * DeviceTable
	 */
}


Led::Led(void) : mState(OFF) {
	mFileDescriptor = open(LED_BRIGHTNESS_PATH, O_RDWR);
	if(mFileDescriptor < 0)
	{
		perror("could not open led brightness path");
		exit(EXIT_FAILURE);
	}

	int lError = write(mFileDescriptor, "0", 1);
	if(lError < 0) {
		perror("could not turn LED on");
		exit(EXIT_FAILURE);
	}
}

Led::~Led(void) {
	close(mFileDescriptor);
}

void Led::On(void) {

	mState = ON;

	int lError = write(mFileDescriptor, "1", 1);
	if(lError < 0) {
		perror("could not turn LED on");
		exit(EXIT_FAILURE);
	}

	fsync(mFileDescriptor);
}

void Led::Off(void) {

	mState = OFF;

	int lError = write(mFileDescriptor, "0", 1);
	if(lError < 0) {
		perror("could not turn LED off");
		exit(EXIT_FAILURE);
	}

	fsync(mFileDescriptor);
}

void Led::Toggle(void) {
	if(ON == mState) {
		Off();
	}
	else {
		On();
	}
}

int Led::On(lua_State *lState) {
	Lua lLua(lState);

	Led *lLed = static_cast<Led *>(lLua.ToUserData(lLua.UpValueIndex(1)));

	lLed->On();

	return 0;
}

int Led::Off(lua_State *lState) {
	Lua lLua(lState);

	Led *lLed = static_cast<Led *>(lLua.ToUserData(lLua.UpValueIndex(1)));

	lLed->Off();

	return 0;
}

int Led::Toggle(lua_State *lState) {
	Lua lLua(lState);

	Led *lLed = static_cast<Led *>(lLua.ToUserData(lLua.UpValueIndex(1)));

	lLed->Toggle();

	return 0;
}

int Led::HandleMsg(lua_State *lState) {

	int lRet = 0;

	Lua lLua(lState);

	Led *lLed = static_cast<Led *>(lLua.ToUserData(lLua.UpValueIndex(1)));

	LedFunctions lFunc = static_cast<LedFunctions>(lLua.ToInteger(lLua.UpValueIndex(2)));

	switch(lFunc) {
	case FUNC_GET:
		lLua.PushBoolean(lLed->GetState());
		lRet = 1;
		break;
	case FUNC_SET:
		if(lLua.IsBoolean(1)) {
			int lArgCount = lLua.GetTop();
			if(lArgCount == 1) {
				bool lNewLedState = lLua.ToBoolean(1);
				switch(lNewLedState) {
				case true:
					lLed->On();
					break;
				case false:
					lLed->Off();
					break;
				}
			}
		}
		break;
	}

	return lRet;
}


#ifdef RUN_LED_TEST

#ifdef __cplusplus
extern "C" {
#endif

void SignalHandler(int lSigNum)
{
    switch(lSigNum)
    {
        case SIGTERM:
    		gLed.Off();
    		gLed.~Led();
            exit(0);
            break;
        default:
            break;
    }
    return;
}

#ifdef __cplusplus
}
#endif

static void MainLedTest(void) {
	while(1) {
		gLed.Toggle();

		sleep(2);
	}
}

static int InitDaemon(void) {
    pid_t lPid = fork();
    if(lPid < 0)
    {
        perror("could not fork() for daemon");
        return lPid;
    }
    if(lPid > 0)
    {
        return 0;
    }

    if(setsid() < 0)
    {
        perror("could not set SID for daemon");
        return -1;
    }

    while(true)
    {
    	MainLedTest();
    }
    return 0;
}

int main(int argc, char *argv[]) {

    signal(SIGTERM, &SignalHandler);

	int lErr = InitDaemon();
	if(lErr)
	{
		perror("could not create LED daemon");
		return -1;
	}
	return 0;
}

#endif // RUN_LED_TEST
