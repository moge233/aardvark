/*
 * errors.hpp
 *
 *  Created on: Oct 5, 2024
 *      Author: matt
 */

#ifndef PLATFORM_INC_ERRORS_HPP_
#define PLATFORM_INC_ERRORS_HPP_


#include <cstdint>
#include <queue>

#include "lua.hpp"


void ErrorsInstall(lua_State *lState);


enum ErrorsFunctions {
	FUNC_NEXT = 0,
	FUNC_POP,
	FUNC_COUNT,
	FUNC_IS_EMPTY,
};


class Error {
public:
	Error(void) : mErrorNumber(0), mMessageBuffer({0}) { }
	Error(size_t lErrorNumber, const char *lMessage, size_t lLength) : mErrorNumber(lErrorNumber), mMessageBuffer({0}) { SetMessage(lMessage, lLength); }

	static size_t GetMaxErrorMessageLength(void) { return cMaxErrorMessageLength; }

	int32_t GetNumber(void) { return mErrorNumber; }
	void SetNumber(int32_t lErrorNumber) { mErrorNumber = lErrorNumber; }
	const char *GetMessage(void) { return mMessageBuffer; }
	void SetMessage(const char *lMessage, size_t lLength);
	void ClearMessage(void);

private:
	static constexpr size_t cMaxErrorMessageLength = 1000;
	int32_t mErrorNumber;
	char mMessageBuffer[cMaxErrorMessageLength];
};


class ErrorController {
public:
	ErrorController(void) { }

	/*
	 * Lua bindings
	 */
	static int HandleMsg(lua_State *lState);
	static int FuncNext(lua_State *lState);

	void Push(Error lError) { mQueue.push(lError); }
	Error Next(void) { return mQueue.front(); }
	void Pop(void) { mQueue.pop(); }
	size_t Count(void) { return mQueue.size(); }
	bool IsEmpty(void) { return mQueue.empty(); }

private:
	std::queue<Error> mQueue;
};


namespace SystemErrors {

extern ErrorController gErrorController;

extern Error SystemGeneralError;


enum SystemErrors {
	SYSTEM_GENERAL_ERROR = 1,
	END_SYSTEM_ERRORS
};

constexpr char cSystemGeneralErrorMessage[] = "general error";

}


#endif /* PLATFORM_INC_ERRORS_HPP_ */
