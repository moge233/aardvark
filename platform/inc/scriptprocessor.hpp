/*
 * scriptprocessor.hpp
 *
 *  Created on: Jan 23, 2026
 *      Author: matt
 */

#ifndef AARDVARK_PLATFORM_SRC_SCRIPTPROCESSOR_HPP_
#define AARDVARK_PLATFORM_SRC_SCRIPTPROCESSOR_HPP_

#include <pthread.h>

#include "endpoint.hpp"
#include "lua.hpp"

constexpr unsigned int ASCII_DEFAULT_PRECISION = 6;

enum DebugFunctions {
	FUNC_GET_MANUFACTURER = 0,
	FUNC_GET_MODEL,
	FUNC_GET_SERIAL,
};

class ScriptProcessor : public Endpoint, public Lua
{
protected:
	enum {
		OUTPUT_BUFFER_SIZE = 10000,
		COMMAND_BUFFER_SIZE = 1000,
	};
	static pthread_mutex_t mLock;
	static char mOutputBuffer[OUTPUT_BUFFER_SIZE];
	static unsigned int mOutputBytes;
	static char mCommandBuffer[COMMAND_BUFFER_SIZE];
	static unsigned int mCommandBytes;

	int mAsciiPrecision = ASCII_DEFAULT_PRECISION;
	bool mTriggered;

	static inline int Lock(void) { return pthread_mutex_lock(&mLock); }
	static inline int TryLock(void) { return pthread_mutex_trylock(&mLock); }
	static inline int Unlock(void) { return pthread_mutex_unlock(&mLock); }

private:
	static constexpr char IDN_Q_TOKEN[] = "*IDN? ";
	static constexpr char CLS_TOKEN[] = "*IDN? ";
	static constexpr char RST_TOKEN[] = "*RST ";
	static constexpr char TST_Q_TOKEN[] = "*TST? ";
	static constexpr char OPC_TOKEN[] = "*OPC ";
	static constexpr char OPC_Q_TOKEN[] = "*OPC? ";
	static constexpr char WAI_TOKEN[] = "*WAI ";
	static constexpr char ESE_TOKEN[] = "*ESE ";
	static constexpr char ESE_Q_TOKEN[] = "*ESE? ";
	static constexpr char ESR_Q_TOKEN[] = "*ESR? ";
	static constexpr char SRE_TOKEN[] = "*SRE ";
	static constexpr char SRE_Q_TOKEN[] = "*SRE? ";
	static constexpr char STB_Q_TOKEN[] = "*STB? ";

	enum
	{
		IDN_Q_IDX = 0,
		CLS_IDX,
		RST_IDX,
		TST_Q_IDX,
		OPC_IDX,
		OPC_Q_IDX,
		WAI_IDX,
		ESE_IDX,
		ESE_Q_IDX,
		ESR_Q_IDX,
		SRE_IDX,
		SRE_Q_IDX,
		STB_Q_IDX,
		MAX_TOKEN_IDX
	};

	static constexpr size_t mNumTokens = MAX_TOKEN_IDX;
	static constexpr const char *mTokens[] = {
			IDN_Q_TOKEN, CLS_TOKEN, RST_TOKEN, TST_Q_TOKEN, OPC_TOKEN, OPC_Q_TOKEN, WAI_TOKEN,
			ESE_TOKEN, ESE_Q_TOKEN, ESR_Q_TOKEN, SRE_TOKEN, SRE_Q_TOKEN, STB_Q_TOKEN
	};

	typedef ssize_t (*TokenHandler)(const char *, size_t);

	/* IEEE-488 Standard Commands/Queries */
	void RegisterHandler(TokenHandler, size_t);
	static ssize_t IDNQHandler(const char *lBuffer, size_t lLength);
	static ssize_t CLSHandler(const char *lBuffer, size_t lLength);
	static ssize_t RSTHandler(const char *lBuffer, size_t lLength);
	static ssize_t TSTQHandler(const char *lBuffer, size_t lLength);
	static ssize_t OPCHandler(const char *lBuffer, size_t lLength);
	static ssize_t OPCQHandler(const char *lBuffer, size_t lLength);
	static ssize_t WAIHandler(const char *lBuffer, size_t lLength);
	static ssize_t ESEHandler(const char *lBuffer, size_t lLength);
	static ssize_t ESEQHandler(const char *lBuffer, size_t lLength);
	static ssize_t ESRQHandler(const char *lBuffer, size_t lLength);
	static ssize_t SREHandler(const char *lBuffer, size_t lLength);
	static ssize_t SREQHandler(const char *lBuffer, size_t lLength);
	static ssize_t STBQHandler(const char *lBuffer, size_t lLength);

	TokenHandler mTokenHandlers[mNumTokens] = { static_cast<TokenHandler>(nullptr) };

public:
	ScriptProcessor(void);
	~ScriptProcessor();

	static int InfoHandler(lua_State *lState);

	Lua & GetLuaInstance(void);

	int GetAsciiPrecision(void) { return mAsciiPrecision; }

	void Main(void);
	void InstallTokenHandlers(void);
	void StartLua(void);
	int HandleCommand(const char *lBuffer, size_t lLength);

	void PushGlobalClosure(const char *lName, lua_CFunction lFunc, int lNumUpValues);
	void PushStatelessGlobalClosure(const char *lName, lua_CFunction lFunc);

	void BackdoorInstall(lua_State *lState);
	void InfoInstall(lua_State *lState);

	// Public getters/setters/misc.
	inline char *GetData(void) { return &mOutputBuffer[0]; }
	inline void ClearData(void) { Lock(); memset(&mOutputBuffer[0], 0, OUTPUT_BUFFER_SIZE); Unlock(); }
	inline unsigned int GetCount(void) { return mOutputBytes; }
	inline void ClearCount(void) { Lock(); mOutputBytes = 0; Unlock(); }
};

class StatelessScriptProcessor : ScriptProcessor
{
public:
	static int Delay(lua_State *lState);
	static int Print(lua_State *lState);
	static int Print(::Lua *lLua);

	static int TriggerClear(lua_State *lState);
	static int ReadStb(lua_State *lState);

	int TriggerClear(void);
	int ReadStb(void);

	// Backdoor
	static int Backdoor(lua_State *lState);
	static int BackdoorProtect(lua_State *lState);
	static int BackdoorDebug(lua_State *lState);
};


#endif /* AARDVARK_PLATFORM_SRC_SCRIPTPROCESSOR_HPP_ */
