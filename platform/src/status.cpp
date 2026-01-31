/*
 * status.cpp
 *
 *  Created on: Sep 23, 2024
 *      Author: matt
 */


#include <cstdio>

#include "status.hpp"


StatusDataStructure gPlatformStatus;


void StatusInstall(lua_State *lState) {
	Lua lLua(lState);

	// MakeTable
	lLua.NewTable();
	lLua.MakeTableReadOnly();

	/*
	 * Stack: (top down)
	 * 		: * status DeviceTable
	 */
	LuaUtils::AddGetter(lLua, "condition", &gPlatformStatus, FUNC_GET_CONDITION_REGISTER, StatusDataStructure::HandleMsg);
	LuaUtils::AddGetter(lLua, "event", &gPlatformStatus, FUNC_GET_EVENT_REGISTER, StatusDataStructure::HandleMsg);
	LuaUtils::AddSetter(lLua, "event", &gPlatformStatus, FUNC_SET_EVENT_REGISTER, StatusDataStructure::HandleMsg);
	LuaUtils::AddGetter(lLua, "eventenable", &gPlatformStatus, FUNC_GET_EVENT_ENABLE_REGISTER, StatusDataStructure::HandleMsg);
	LuaUtils::AddSetter(lLua, "eventenable", &gPlatformStatus, FUNC_SET_EVENT_ENABLE_REGISTER, StatusDataStructure::HandleMsg);
	LuaUtils::AddGetter(lLua, "positivetransition", &gPlatformStatus, FUNC_GET_POSITIVE_TRANS_REGISTER, StatusDataStructure::HandleMsg);
	LuaUtils::AddSetter(lLua, "positivetransition", &gPlatformStatus, FUNC_SET_POSITIVE_TRANS_REGISTER, StatusDataStructure::HandleMsg);
	LuaUtils::AddGetter(lLua, "negativetransition", &gPlatformStatus, FUNC_GET_NEGATIVE_TRANS_REGISTER, StatusDataStructure::HandleMsg);
	LuaUtils::AddSetter(lLua, "negativetransition", &gPlatformStatus, FUNC_SET_NEGATIVE_TRANS_REGISTER, StatusDataStructure::HandleMsg);
	LuaUtils::AddGetter(lLua, "servicerequestenable", &gPlatformStatus, FUNC_GET_SERVICE_REQUEST_ENABLE_REGISTER, StatusDataStructure::HandleMsg);
	LuaUtils::AddSetter(lLua, "servicerequestenable", &gPlatformStatus, FUNC_SET_SERVICE_REQUEST_ENABLE_REGISTER, StatusDataStructure::HandleMsg);

	/*
	 * Stack: (top down)
	 * 		: * status DeviceTable
	 */
	lLua.SetGlobal("status");

	/*
	 * Stack: (top down)
	 * 		: * DeviceTable
	 */
}


StatusDataStructure::StatusDataStructure(void)
: mConditionRegister(0)
, mEventRegister(0)
, mEventEnableRegister(0)
, mPositiveTransitionRegister(0)
, mNegativeTransitionRegister(0)
, mStatusByteRegister(0)
{
	;
}


int StatusDataStructure::HandleMsg(lua_State *lState) {

	int lRet = 0;

	Lua lLua(lState);

	StatusDataStructure *lPlatformStatus = static_cast<StatusDataStructure *>(lLua.ToUserData(lLua.UpValueIndex(1)));

	StatusFunctions lFunc = static_cast<StatusFunctions>(lLua.ToInteger(lLua.UpValueIndex(2)));

	switch(lFunc) {
	case FUNC_GET_CONDITION_REGISTER:
		lLua.PushNumber(static_cast<float>(lPlatformStatus->GetConditionRegister()));
		lRet = 1;
		break;
	case FUNC_GET_EVENT_REGISTER:
		lLua.PushNumber(static_cast<float>(lPlatformStatus->GetEventRegister()));
		lRet = 1;
		break;
	case FUNC_SET_EVENT_REGISTER:
		if(lLua.IsInteger(1)) {
			int lArgCount = lLua.GetTop();
			if(lArgCount == 1) {
				int lEvent = lLua.ToNumber(1);
				if(lEvent < 0) {
					// ERROR
				}
				else {
					lPlatformStatus->SetEventRegister(lEvent);
				}
			}
		}
		break;
	case FUNC_GET_EVENT_ENABLE_REGISTER:
		lLua.PushNumber(static_cast<float>(lPlatformStatus->GetEventEnableRegister()));
		lRet = 1;
		break;
	case FUNC_SET_EVENT_ENABLE_REGISTER:
		if(lLua.IsInteger(1)) {
			int lArgCount = lLua.GetTop();
			if(lArgCount == 1) {
				int lEventEnable = lLua.ToNumber(1);
				if(lEventEnable < 0) {
					// ERROR
				}
				else {
					lPlatformStatus->SetEventEnableRegister(lEventEnable);
				}
			}
		}
		break;
	case FUNC_GET_POSITIVE_TRANS_REGISTER:
		lLua.PushNumber(static_cast<float>(lPlatformStatus->GetPositiveTransitionRegister()));
		lRet = 1;
		break;
	case FUNC_SET_POSITIVE_TRANS_REGISTER:
		if(lLua.IsInteger(1)) {
			int lArgCount = lLua.GetTop();
			if(lArgCount == 1) {
				int lPositiveTransition = lLua.ToNumber(1);
				if(lPositiveTransition < 0) {
					// ERROR
				}
				else {
					lPlatformStatus->SetPositiveTransitionRegister(lPositiveTransition);
				}
			}
		}
		break;
	case FUNC_GET_NEGATIVE_TRANS_REGISTER:
		lLua.PushNumber(static_cast<float>(lPlatformStatus->GetNegativeTransitionRegister()));
		lRet = 1;
		break;
	case FUNC_SET_NEGATIVE_TRANS_REGISTER:
		if(lLua.IsInteger(1)) {
			int lArgCount = lLua.GetTop();
			if(lArgCount == 1) {
				int lNegativeTransition = lLua.ToNumber(1);
				if(lNegativeTransition < 0) {
					// ERROR
				}
				else {
					lPlatformStatus->SetNegativeTransitionRegister(lNegativeTransition);
				}
			}
		}
		break;
	case FUNC_GET_SERVICE_REQUEST_ENABLE_REGISTER:
		lLua.PushNumber(static_cast<float>(lPlatformStatus->GetServiceRequestEnableRegister()));
		lRet = 1;
		break;
	case FUNC_SET_SERVICE_REQUEST_ENABLE_REGISTER:
		if(lLua.IsInteger(1)) {
			int lArgCount = lLua.GetTop();
			if(lArgCount == 1) {
				int lServiceRequestEnable = lLua.ToNumber(1);
				if(lServiceRequestEnable < 0) {
					// ERROR
				}
				else {
					lPlatformStatus->SetServiceRequestEnableRegister(lServiceRequestEnable);
				}
			}
		}
		break;
	default:
		break;
	}

	return lRet;
}

void StatusModelApi::SetConditionRegister(StatusDataStructure& lStatus, uint16_t lValue)
{
	lStatus.SetConditionRegister(lValue);
}

void StatusModelApi::ClearConditionRegister(StatusDataStructure& lStatus)
{
	lStatus.ClearConditionRegister();
}

uint16_t StatusModelApi::GetConditionRegister(StatusDataStructure& lStatus)
{
	return lStatus.GetConditionRegister();
}

void StatusModelApi::SetEventRegister(StatusDataStructure& lStatus, uint16_t lValue)
{
	lStatus.SetEventEnableRegister(lValue);
}

void StatusModelApi::ClearEventRegister(StatusDataStructure& lStatus)
{
	lStatus.ClearEventRegister();
}

uint16_t StatusModelApi::GetEventRegister(StatusDataStructure& lStatus)
{
	return lStatus.GetEventRegister();
}

void StatusModelApi::UpdateEventRegister(StatusDataStructure& lStatus)
{
	lStatus.SetEventRegister(
				(lStatus.GetNegativeTransitionRegister() & lStatus.GetConditionRegister()) |
				(lStatus.GetPositiveTransitionRegister() & lStatus.GetConditionRegister())
			);
}

void StatusModelApi::SetEventEnableRegister(StatusDataStructure& lStatus, uint16_t lValue)
{
	lStatus.SetEventEnableRegister(lValue);
}

void StatusModelApi::ClearEventEnableRegister(StatusDataStructure& lStatus)
{
	lStatus.ClearEventEnableRegister();
}

uint16_t StatusModelApi::GetEventEnableRegister(StatusDataStructure& lStatus)
{
	return lStatus.GetEventEnableRegister();
}

uint16_t StatusModelApi::Summarize(StatusDataStructure& lStatus)
{
	StatusModelApi::UpdateEventRegister(lStatus);
	return StatusModelApi::GetEventRegister(lStatus) & StatusModelApi::GetEventEnableRegister(lStatus);
}

#ifdef RUN_STATUS

using namespace StatusModelApi;

int main(void) {
	StatusDataStructure gStatus;

	SetConditionRegister(gStatus, 1 << PON_BIT);
	printf("(1) - gStatus.GetConditionRegister(): %u\n", GetConditionRegister(gStatus));

	SetConditionRegister(gStatus, 1 << OPC_BIT);
	printf("(2) - gStatus.GetConditionRegister(): %u\n", GetConditionRegister(gStatus));

	ClearConditionRegister(gStatus);
	printf("(3) - gStatus.GetConditionRegister(): %u\n", GetConditionRegister(gStatus));
}

#endif
