/*
 * scriptprocessor.cpp
 *
 *  Created on: Jan 23, 2026
 *      Author: matt
 */


#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <iostream>

#include "errors.hpp"
#include "led.hpp"
#include "logicaldevice.hpp"
#include "model.hpp"
#include "scriptprocessor.hpp"
#include "status.hpp"


static constexpr unsigned long OUTPUT_BUFFER_SIZE = 10000;


pthread_mutex_t ScriptProcessor::mLock = PTHREAD_MUTEX_INITIALIZER;
char ScriptProcessor::mOutputBuffer[OUTPUT_BUFFER_SIZE] = { 0 };
unsigned int ScriptProcessor::mOutputBytes = 0;
char ScriptProcessor::mCommandBuffer[COMMAND_BUFFER_SIZE] = { 0 };
unsigned int ScriptProcessor::mCommandBytes = 0;

constexpr char TST_Q_RESPONSE[] = "0\n";
constexpr char OPC_Q_RESPONSE[] = "1\n";
constexpr char ESE_Q_RESPONSE[] = "0\n";
constexpr char ESR_Q_RESPONSE[] = "128\n";
constexpr char SRE_Q_RESPONSE[] = "0\n";
constexpr char STB_Q_RESPONSE[] = "42";
constexpr char ERROR_RESPONSE[] = "ERROR";
constexpr size_t TST_Q_RESPONSE_LEN = 2;
constexpr size_t OPC_Q_RESPONSE_LEN = 2;
constexpr size_t ESE_Q_RESPONSE_LEN = 2;
constexpr size_t ESR_Q_RESPONSE_LEN = 4;
constexpr size_t SRE_Q_RESPONSE_LEN = 2;
constexpr size_t STB_Q_RESPONSE_LEN = 3;
constexpr size_t ERROR_RESPONSE_LEN = 5;

constexpr char IDN_Q_CMD[] = "print(information.manufacturer..\",\"..information.model..\",\"..information.serial..\",INSTR\")\0";
constexpr char ESE_Q_CMD[] = "print(status.eventenable)\0";
constexpr char ESR_Q_CMD[] = "print(status.event)\0";


ScriptProcessor::ScriptProcessor(void)
: Lua()
{
	memset(&mOutputBuffer[0], 0, OUTPUT_BUFFER_SIZE);

	InstallTokenHandlers();
	StartLua();
}

ScriptProcessor::~ScriptProcessor()
{
}

int ScriptProcessor::InfoHandler(lua_State *lState) {

	int lRet = 0;

	Lua lLua(lState);

	// ScriptProcessor *lScriptProcessor = static_cast<ScriptProcessor *>(lLua.ToUserData(lLua.UpValueIndex(1)));

	DebugFunctions lFunc = static_cast<DebugFunctions>(lLua.ToInteger(lLua.UpValueIndex(2)));

	switch(lFunc) {
	case FUNC_GET_MANUFACTURER:
		lLua.PushString(MANUFACTURER);
		lRet = 1;
		break;
	case FUNC_GET_MODEL:
		lLua.PushString(MODEL);
		lRet = 1;
		break;
	case FUNC_GET_SERIAL:
		lLua.PushString(SERIAL);
		lRet = 1;
		break;
	}

	return lRet;
}

void ScriptProcessor::Main(void)
{
	printf("%s called\n", __func__);

	Lua lLua = this->GetLuaInstance();

	// lLua.PushCFunction(pmain);

	lLua.PCall(0, 0, 0);
}

void ScriptProcessor::InstallTokenHandlers(void)
{
	RegisterHandler(ScriptProcessor::IDNQHandler, IDN_Q_IDX);
	RegisterHandler(ScriptProcessor::CLSHandler, CLS_IDX);
	RegisterHandler(ScriptProcessor::RSTHandler, RST_IDX);
	RegisterHandler(ScriptProcessor::TSTQHandler, TST_Q_IDX);
	RegisterHandler(ScriptProcessor::OPCHandler, OPC_IDX);
	RegisterHandler(ScriptProcessor::OPCQHandler, OPC_Q_IDX);
	RegisterHandler(ScriptProcessor::WAIHandler, WAI_IDX);
	RegisterHandler(ScriptProcessor::ESEHandler, ESE_IDX);
	RegisterHandler(ScriptProcessor::ESEQHandler, ESE_Q_IDX);
	RegisterHandler(ScriptProcessor::ESRQHandler, ESR_Q_IDX);
	RegisterHandler(ScriptProcessor::SREHandler, SRE_IDX);
	RegisterHandler(ScriptProcessor::SREQHandler, SRE_Q_IDX);
	RegisterHandler(ScriptProcessor::STBQHandler, STB_Q_IDX);
}

void ScriptProcessor::StartLua(void)
{
	OpenLibs();

	BackdoorInstall(mState);
	// stack: * = top
	PushStatelessGlobalClosure("delay", StatelessScriptProcessor::Delay);	// *
	PushStatelessGlobalClosure("print", StatelessScriptProcessor::Print);	// *
	PushStatelessGlobalClosure("stb", StatelessScriptProcessor::ReadStb);	// *

	LocalLogicalDevice::DeviceTableInit(mState);

	LocalLogicalDevice::DeviceTableGet(this);

	/*
	 * Stack: (top down)
	 * 		: * DeviceTable
	 */
	InfoInstall(mState);
	LedInstall(mState);
	StatusInstall(mState);
	ErrorsInstall(mState);

	/*
	 * Pop off the DeviceTable
	 */
	Pop(1);

	/*
	 * Stack: (top down)
	 * 		: *
	 */

	SetTop(0);
}

int ScriptProcessor::HandleCommand(const char *lBuffer, size_t lLength)
{
	bool lMatched = false;

	for(size_t lIndex=0; lIndex<mNumTokens; lIndex++)
	{
		if(!strncmp(lBuffer, mTokens[lIndex], 5))
		{
			lMatched = true;
			mTokenHandlers[lIndex](lBuffer, lLength);

			if(mCommandBytes > 0) {
				int lResult = LoadString(mCommandBuffer);

				if(lResult)
				{
					printf("Error load lua buffer: %s\n", ToString(-1));
					return lResult;
				}

				lResult = PCall(0, LUA_MULTRET, 0);
				if(lResult)
				{
					printf("Error running lua: %s\n", ToString(-1));
				}

				mCommandBytes = 0;

				return lResult;
			}
			break;
		}
	}

	if(!lMatched) {

		int lResult = LoadString(lBuffer);

		if(lResult)
		{
			printf("Error load lua buffer: %s\n", ToString(-1));
			return lResult;
		}

		lResult = PCall(0, LUA_MULTRET, 0);
		if(lResult)
		{
			printf("Error running lua: %s\n", ToString(-1));
		}

		return lResult;
	}

	return 0;
}

void ScriptProcessor::PushGlobalClosure(const char *lName, lua_CFunction lFunc, int lNumUpValues)
{
	/*
	 * The stack should contain the closure's upvalues at the top before calling
	 * this function.
	 */
	PushCClosure(lFunc, lNumUpValues);					// Stack: C-function

	/*
	 * Pop the C-function from the stack and set it equal to lName
	 * 	lName = C-function
	 */
	SetGlobal(lName);									// Stack: (empty)
}

void ScriptProcessor::PushStatelessGlobalClosure(const char *lName, lua_CFunction lFunc)
{
	PushLightUserData(static_cast<void *>(this));	// Stack: `this`
	PushGlobalClosure(lName, lFunc, 1);
}

void ScriptProcessor::RegisterHandler(TokenHandler lHandler, size_t lIndex)
{
	if(mTokenHandlers[lIndex] == nullptr)
	{
		mTokenHandlers[lIndex] = lHandler;
	}
}

void ScriptProcessor::BackdoorInstall(lua_State *lState) {

	GetBackdoor();

	/*
	 * Stack: (top down)
	 * 		: * Backdoor
	 */

	PushString("debug");

	/*
	 * Stack: (top down)
	 * 		: * "debug" Backdoor
	 */
	PushCClosure(StatelessScriptProcessor::BackdoorDebug, 0);

	/*
	 * Stack: (top down)
	 * 		: * BackdoorDebug() "debug" Backdoor
	 */

	/*
	 * t[k] = v
	 *
	 * t --> Backdoor table
	 * k --> debug
	 * v --> C-function (BackdoorDebug)
	 *
	 * BACKDOORTABLE[debug] = StatelessScriptProcessor::BackdoorDebug
	 */
	SetTable(-3);
	/*
	 * Stack: (top down)
	 * 		: * Backdoor
	 */

	NewTable();		// t2

	/*
	 * Stack: (top down)
	 * 		: * t2 Backdoor
	 */

	PushString("__index");

	/*
	 * Stack: (top down)
	 * 		: * "__index" t2 Backdoor
	 */

	PushCClosure(StatelessScriptProcessor::Backdoor, 0);

	/*
	 * Stack: (top down)
	 * 		: * Backdoor() "__index" t2 Backdoor
	 */


	/*
	 * t[k] = v
	 *
	 * t --> t2 table
	 * k --> "__index"
	 * v --> C-function (Backdoor)
	 *
	 * t2[__index] = StatelessScriptProcessor::Backdoor
	 */
	SetTable(-3);

	/*
	 * Stack: (top down)
	 * 		: * t2 Backdoor
	 */

	PushString("__newindex");

	/*
	 * Stack: (top down)
	 * 		: * "__newindex" t2 Backdoor
	 */

	PushCClosure(StatelessScriptProcessor::BackdoorProtect, 0);

	/*
	 * Stack: (top down)
	 * 		: * BackdoorProtect() "__newindex" t2 Backdoor
	 */

	/*
	 * t[k] = v
	 *
	 * t --> t2 table
	 * k --> "__newindex"
	 * v --> C-function (BackdoorProtect)
	 *
	 * t2[__newindex] = StatelessScriptProcessor::BackdoorProtect
	 */
	SetTable(-3);

	/*
	 * Stack: (top down)
	 * 		: * t2 Backdoor
	 */

	/*
	 * Pops t2 from the stack and sets it as Backdoor's metatable
	 */
	SetMetaTable(-2);

	/*
	 * Stack: (top down)
	 * 		: * Backdoor
	 */

	/*
	 * Pops t1 from the stack and sets it equal to the global name "debug"
	 * "debug" = Backdoor
	 */
	SetGlobal("backdoor");

	/*
	 * Stack: (top down)
	 * 		: *
	 */
}

void ScriptProcessor::InfoInstall(lua_State *lState)
{
	Lua lLua(lState);

	// MakeTable
	lLua.NewTable();
	lLua.MakeTableReadOnly();

	LuaUtils::AddGetter(lLua, "manufacturer", this, FUNC_GET_MANUFACTURER, ScriptProcessor::InfoHandler);
	LuaUtils::AddGetter(lLua, "model", this, FUNC_GET_MODEL, ScriptProcessor::InfoHandler);
	LuaUtils::AddGetter(lLua, "serial", this, FUNC_GET_SERIAL, ScriptProcessor::InfoHandler);

	lLua.SetGlobal("information");
}

ssize_t ScriptProcessor::IDNQHandler(const char *lBuffer, size_t lLength)
{
	Lock();
	size_t lCopyLen = strnlen(IDN_Q_CMD, COMMAND_BUFFER_SIZE) + 1;
	mCommandBytes += snprintf(mCommandBuffer, lCopyLen, "%s", IDN_Q_CMD);
	Unlock();
	return 0;
}

ssize_t ScriptProcessor::CLSHandler(const char *lBuffer, size_t lLength)
{
	Lock();
	// TODO: gPlatformStatus.ClearEventRegister();
	Unlock();
	return 0;
}

ssize_t ScriptProcessor::RSTHandler(const char *lBuffer, size_t lLength)
{
	Lock();
	Unlock();
	return 0;
}

ssize_t ScriptProcessor::TSTQHandler(const char *lBuffer, size_t lLength)
{
	Lock();
	memcpy(&mOutputBuffer[0], &TST_Q_RESPONSE[0], TST_Q_RESPONSE_LEN);
	mOutputBytes += TST_Q_RESPONSE_LEN;
	Unlock();
	return 0;
}

ssize_t ScriptProcessor::OPCHandler(const char *lBuffer, size_t lLength)
{
	Lock();
	Unlock();
	return 0;
}

ssize_t ScriptProcessor::OPCQHandler(const char *lBuffer, size_t lLength)
{
	Lock();
	memcpy(&mOutputBuffer[0], &OPC_Q_RESPONSE[0], OPC_Q_RESPONSE_LEN);
	mOutputBytes += OPC_Q_RESPONSE_LEN;
	Unlock();
	return 0;
}

ssize_t ScriptProcessor::WAIHandler(const char *lBuffer, size_t lLength)
{
	Lock();
	Unlock();
	return 0;
}

ssize_t ScriptProcessor::ESEHandler(const char *lBuffer, size_t lLength)
{
	Lock();
	uint16_t lValue = 0;
	int lCount = sscanf(lBuffer, "%*s %hu", &lValue);	// Ignore the command
	if(lCount == 1) {
		// TODO: gPlatformStatus.SetEventEnableRegister(lValue);
	}
	else {
		// ERROR
	}
	Unlock();
	return 0;
}

ssize_t ScriptProcessor::ESEQHandler(const char *lBuffer, size_t lLength)
{
	Lock();
	size_t lCopyLen = strnlen(ESE_Q_CMD, COMMAND_BUFFER_SIZE) + 1;
	mCommandBytes += snprintf(mCommandBuffer, lCopyLen, "%s", ESE_Q_CMD);
	Unlock();
	return 0;
}

ssize_t ScriptProcessor::ESRQHandler(const char *lBuffer, size_t lLength)
{
	Lock();
	size_t lCopyLen = strnlen(ESR_Q_CMD, COMMAND_BUFFER_SIZE) + 1;
	mCommandBytes += snprintf(mCommandBuffer, lCopyLen, "%s", ESR_Q_CMD);
	// TODO: gPlatformStatus.ClearEventRegister();
	Unlock();
	return 0;
}

ssize_t ScriptProcessor::SREHandler(const char *lBuffer, size_t lLength)
{
	Lock();
	Unlock();
	return 0;
}

ssize_t ScriptProcessor::SREQHandler(const char *lBuffer, size_t lLength)
{
	Lock();
	memcpy(&mOutputBuffer[0], &SRE_Q_RESPONSE[0], SRE_Q_RESPONSE_LEN);
	mOutputBytes += SRE_Q_RESPONSE_LEN;
	Unlock();
	return 0;
}

ssize_t ScriptProcessor::STBQHandler(const char *lBuffer, size_t lLength)
{
	Lock();
	memcpy(&mOutputBuffer[0], &STB_Q_RESPONSE[0], STB_Q_RESPONSE_LEN);
	mOutputBytes += STB_Q_RESPONSE_LEN;
	Unlock();
	return 0;
}

inline Lua & ScriptProcessor::GetLuaInstance(void)
{
	return static_cast<Lua &>(*this);
}


int StatelessScriptProcessor::Delay(lua_State *lState)
{
	useconds_t lInterval;

	lInterval = lua_tonumber(lState, 1);

	usleep(lInterval);

	return 0;
}

int StatelessScriptProcessor::Print(lua_State *lState)
{
	::Lua lLua(lState);

	StatelessScriptProcessor *lScriptProcessor = static_cast<StatelessScriptProcessor *>(lLua.ToUserData(lLua.UpValueIndex(1)));

	return lScriptProcessor->Print(&lLua);
}

int StatelessScriptProcessor::Print(::Lua *lLua)
{
	size_t lPosition = 0;
	char lBuffer[OUTPUT_BUFFER_SIZE];
	int lArgCount = lLua->GetTop();
	lLua->GetGlobal("tostring");

	for(int lIndex=1; lIndex<=lArgCount; ++lIndex)
	{
		if(lIndex > 1) {
			lBuffer[lPosition++] = '\t';
		}

		switch(lLua->Type(lIndex))
		{
		case LUA_TNUMBER:
			lPosition += snprintf(lBuffer + lPosition, 250, "%1.*e", ASCII_DEFAULT_PRECISION-1, static_cast<double>(lLua->ToNumber(lIndex)));
			break;
		case LUA_TSTRING:
			memcpy(&mOutputBuffer[0], lLua->ToString(lIndex), strnlen(lLua->ToString(lIndex), OUTPUT_BUFFER_SIZE));
			mOutputBytes += strnlen(lLua->ToString(lIndex), OUTPUT_BUFFER_SIZE);
			break;
		default:
			lLua->PushValue(-1);
			lLua->PushValue(lIndex);
			static_cast<::BasicLua *>(lLua)->Call(1, 1);
			lLua->Replace(lIndex);
			memcpy(&mOutputBuffer[0], lLua->ToString(lIndex), strnlen(lLua->ToString(lIndex), OUTPUT_BUFFER_SIZE));
			mOutputBytes += strnlen(lLua->ToString(lIndex), OUTPUT_BUFFER_SIZE);
			break;
		}
	}

	if(lPosition) {
		memcpy(&mOutputBuffer[0], lBuffer, strnlen(lBuffer, OUTPUT_BUFFER_SIZE));
		mOutputBytes += strnlen(lBuffer, OUTPUT_BUFFER_SIZE);
	}

	return 0;
}

int StatelessScriptProcessor::TriggerClear(lua_State *lState) {
	::Lua lLua(lState);

	StatelessScriptProcessor *lScriptProcessor = static_cast<StatelessScriptProcessor *>(lLua.ToUserData(lLua.UpValueIndex(1)));
	return lScriptProcessor->TriggerClear();
}

int StatelessScriptProcessor::ReadStb(lua_State *lState) {
	::Lua lLua(lState);

	lLua.PushInteger(42);

	return 1;
}

int StatelessScriptProcessor::TriggerClear(void) {
	mTriggered = false;
	return 0;
}

int StatelessScriptProcessor::ReadStb(void) {
	return 1;
}

int StatelessScriptProcessor::Backdoor(lua_State *lState) {
	::Lua lLua(lState);

	lLua.GetBackdoor();

	lLua.Insert(-2);

	lLua.GetTable(-2);

	return 1;
}

int StatelessScriptProcessor::BackdoorProtect(lua_State *lState) {
	printf("%s\n", __func__);

	return 0;
}

int StatelessScriptProcessor::BackdoorDebug(lua_State *lState) {
	printf("%s\n", __func__);

	return 0;
}
