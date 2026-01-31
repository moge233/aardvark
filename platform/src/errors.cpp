/*
 * errors.cpp
 *
 *  Created on: Oct 5, 2024
 *      Author: matt
 */


#include <cstdio>
#include <cstring>

#include "errors.hpp"


ErrorController SystemErrors::gErrorController;

/*
 * System error definitions
 */
Error SystemErrors::SystemGeneralError(SystemErrors::SYSTEM_GENERAL_ERROR,
		cSystemGeneralErrorMessage,
		strnlen(cSystemGeneralErrorMessage, Error::GetMaxErrorMessageLength()));

const Error cSystemErrors[SystemErrors::END_SYSTEM_ERRORS - 1] = {
		SystemErrors::SystemGeneralError,
};


void ErrorsInstall(lua_State *lState) {
	Lua lLua(lState);

	// MakeTable
	lLua.NewTable();
	lLua.MakeTableReadOnly();

	LuaUtils::AddClosure(lLua, "next", &SystemErrors::gErrorController, ErrorController::FuncNext);

	lLua.SetGlobal("errors");
}


void Error::SetMessage(const char *lMessage, size_t lLength) {
	if(lLength < Error::cMaxErrorMessageLength) {
		memcpy(mMessageBuffer, lMessage, lLength);
	}
}

void Error::ClearMessage(void) {
	memset(mMessageBuffer, 0, cMaxErrorMessageLength);
}

int ErrorController::HandleMsg(lua_State *lState) {
	Lua lLua(lState);

	int lRet = 0;

	ErrorController *lErrorController = static_cast<ErrorController *>(lLua.ToUserData(lLua.UpValueIndex(1)));

	ErrorsFunctions lFunc = static_cast<ErrorsFunctions>(lLua.ToInteger(lLua.UpValueIndex(2)));

	switch(lFunc) {
	case FUNC_NEXT:
		break;
	case FUNC_POP:
		break;
	case FUNC_COUNT:
		break;
	case FUNC_IS_EMPTY:
		break;
	}

	return lRet;
}

int ErrorController::FuncNext(lua_State *lState) {
	printf("%s, %s\n", __FILE__, __func__);
	Lua lLua(lState);

	ErrorController *lController = static_cast<ErrorController *>(lLua.ToUserData(lLua.UpValueIndex(1)));

	if(!lController->IsEmpty()) {

		size_t lErrorBufferSize = Error::GetMaxErrorMessageLength() + 10;
		char lErrorBuffer[lErrorBufferSize];
		Error lError = lController->Next();

		snprintf(lErrorBuffer, lErrorBufferSize, "%s (-%d)", lError.GetMessage(), lError.GetNumber());

		lLua.PushString(lErrorBuffer);
	}
	else {
		lLua.PushNil();
	}

	return 1;
}


#ifdef RUN_ERRORS

using namespace SystemErrors;

int main(void) {

	gErrorController.Push(SystemGeneralError);
	printf("gErrorController.Count(): %u\n", gErrorController.Count());
	while(!gErrorController.IsEmpty()) {
		Error lError = gErrorController.Next();
		printf("SYSTEM ERROR: %s (%d)\n", lError.GetMessage(), lError.GetNumber());
		gErrorController.Pop();
	}

	return 0;
}
#endif
