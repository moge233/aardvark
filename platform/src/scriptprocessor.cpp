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
#include "model.hpp"
#include "scriptprocessor.hpp"
#include "status.hpp"

string ScriptProcessor::mOutputString;

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
constexpr char TST_Q_CMD[] = "print(0)\0";
constexpr char OPC_Q_CMD[] = "print(1)\0";
constexpr char ESE_Q_CMD[] = "print(status.eventenable)\0";
constexpr char ESR_Q_CMD[] = "print(status.event)\0";
constexpr char SRE_Q_CMD[] = "print(0)\0";
constexpr char STB_Q_CMD[] = "print(69)\0";

static const char sDeviceTableIndex[] = "DeviceTableIndex";

ScriptProcessor *gScriptProcessor;

static void InitDeviceTable(lua_State *lState)
{
	Lua lLua(lState);

	// Create the device table as a read-only table
	lLua.PushLightUserData(const_cast<char *>(sDeviceTableIndex));
	lLua.NewTable();
	lLua.MakeTableReadOnly();

	/*
	 * Stack: (top down)
	 * 		: {} "DeviceTableIndex"
	 */

	// Put the device table into the registry for safekeeping.
	lLua.SetTable(LUA_REGISTRYINDEX);			// registry["DeviceTableIndex"] = {}

	// Put the device table into localnode.
	lLua.PushLightUserData(const_cast<char *>(sDeviceTableIndex));
	lLua.GetTable(LUA_REGISTRYINDEX);
	lLua.SetGlobal("localnode");
}

static void GetDeviceTable(Lua *lLua) {
	lLua->PushLightUserData(const_cast<char *>(sDeviceTableIndex));
	lLua->GetTable(LUA_REGISTRYINDEX);
}

ssize_t IDNQHandler(const char *lBuffer, size_t lLength)
{
	return gScriptProcessor->HandleCommand(IDN_Q_CMD, strlen(IDN_Q_CMD), false);
}

ssize_t CLSHandler(const char *lBuffer, size_t lLength)
{
#if 0
	Lock();
	// TODO: gPlatformStatus.ClearEventRegister();
	Unlock();
#else
#endif
	return 0;
}

ssize_t RSTHandler(const char *lBuffer, size_t lLength)
{
#if 0
	Lock();
	Unlock();
#else
#endif
	return 0;
}

ssize_t TSTQHandler(const char *lBuffer, size_t lLength)
{
	return gScriptProcessor->HandleCommand(TST_Q_CMD, strlen(TST_Q_CMD), false);
}

ssize_t OPCHandler(const char *lBuffer, size_t lLength)
{
#if 0
	Lock();
	Unlock();
#else
#endif
	return 0;
}

ssize_t OPCQHandler(const char *lBuffer, size_t lLength)
{
	return gScriptProcessor->HandleCommand(OPC_Q_CMD, strlen(OPC_Q_CMD), false);
}

ssize_t WAIHandler(const char *lBuffer, size_t lLength)
{
#if 0
	Lock();
	Unlock();
#else
#endif
	return 0;
}

ssize_t ESEHandler(const char *lBuffer, size_t lLength)
{
#if 0
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
#else
#endif
	return 0;
}

ssize_t ESEQHandler(const char *lBuffer, size_t lLength)
{
	return gScriptProcessor->HandleCommand(ESE_Q_CMD, strlen(ESE_Q_CMD), false);
}

ssize_t ESRQHandler(const char *lBuffer, size_t lLength)
{
	return gScriptProcessor->HandleCommand(ESR_Q_CMD, strlen(ESR_Q_CMD), false);
}

ssize_t SREHandler(const char *lBuffer, size_t lLength)
{
	return 0;
}

ssize_t SREQHandler(const char *lBuffer, size_t lLength)
{
	return gScriptProcessor->HandleCommand(SRE_Q_CMD, strlen(SRE_Q_CMD), false);
}

ssize_t STBQHandler(const char *lBuffer, size_t lLength)
{
	return gScriptProcessor->HandleCommand(STB_Q_CMD, strlen(STB_Q_CMD), false);
}

ScriptProcessor::ScriptProcessor(void)
: Endpoint()
, Lua()
{
	mOutputString.clear();

	pthread_mutex_init(&mLock, nullptr);

	InstallTokenHandlers();
	StartLua();
}

ScriptProcessor::~ScriptProcessor()
{
}

int ScriptProcessor::InfoHandler(lua_State *lState) {

	int lRet = 0;

	Lua lLua(lState);

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

void ScriptProcessor::InstallTokenHandlers(void)
{
	RegisterHandler(IDNQHandler, IDN_Q_IDX);
	RegisterHandler(CLSHandler, CLS_IDX);
	RegisterHandler(RSTHandler, RST_IDX);
	RegisterHandler(TSTQHandler, TST_Q_IDX);
	RegisterHandler(OPCHandler, OPC_IDX);
	RegisterHandler(OPCQHandler, OPC_Q_IDX);
	RegisterHandler(WAIHandler, WAI_IDX);
	RegisterHandler(ESEHandler, ESE_IDX);
	RegisterHandler(ESEQHandler, ESE_Q_IDX);
	RegisterHandler(ESRQHandler, ESR_Q_IDX);
	RegisterHandler(SREHandler, SRE_IDX);
	RegisterHandler(SREQHandler, SRE_Q_IDX);
	RegisterHandler(STBQHandler, STB_Q_IDX);
}

void ScriptProcessor::StartLua(void)
{
	OpenLibs();

	BackdoorInstall(mState);
	// stack: * = top
	PushStatelessGlobalClosure("delay", StatelessScriptProcessor::Delay);	// *
	PushStatelessGlobalClosure("print", StatelessScriptProcessor::Print);	// *
	PushStatelessGlobalClosure("stb", StatelessScriptProcessor::ReadStb);	// *

	InitDeviceTable(mState);

	GetDeviceTable(this);

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

int ScriptProcessor::HandleCommand(const char *lBuffer, size_t lLength, bool lCheckTokens)
{
	bool lMatched = false;

	if (lCheckTokens)
	{
		for (size_t lIndex=0; lIndex<mNumTokens; lIndex++)
		{
			if(!strncmp(lBuffer, mTokens[lIndex], 5))
			{
				lMatched = true;
				mTokenHandlers[lIndex](lBuffer, lLength);

				break;
			}
		}
	}

	if (!lMatched)
	{

		int lResult = RunScript(lBuffer, strlen(lBuffer));

		return lResult;
	}

	return 0;
}

int ScriptProcessor::RunScript(const char *lScript, size_t lLength)
{
	int lResult = LoadString(lScript);

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
			mOutputString += string(lLua->ToString(lIndex));
			break;
		default:
			lLua->PushValue(-1);
			lLua->PushValue(lIndex);
			static_cast<::BasicLua *>(lLua)->Call(1, 1);
			lLua->Replace(lIndex);
			mOutputString += string(lLua->ToString(lIndex));
			break;
		}
	}

	if(lPosition) {
		mOutputString += string(lBuffer);
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
