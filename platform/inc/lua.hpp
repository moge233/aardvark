/*
 * lua.hpp
 *
 *  Created on: Mar 3, 2024
 *      Author: matt
 */

#ifndef PLATFORM_LUA_HPP_
#define PLATFORM_LUA_HPP_

#include <cassert>
#include <cstring>
#include <iostream>

#include <signal.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}


constexpr int LUA_MAXINPUT = 512;
#if !defined(LUA_PROMPT)
constexpr char LUA_PROMPT[] = "> ";
constexpr char LUA_PROMPT2[] = ">> ";
#endif

constexpr char cDeviceTableIndex[] = "DeviceTableIndex";

#define ReadLine(L,b,p) \
        ((void)L, fputs(p, stdout), fflush(stdout),  /* show prompt */ \
        fgets(b, LUA_MAXINPUT, stdin) != NULL)  /* get line */
#define SaveLine(L,line)	{ (void)L; (void)line; }
#define FreeLine(L,b)	{ (void)L; (void)b; }

#define EOFMARK		"<eof>"
#define marklen		(sizeof(EOFMARK)/sizeof(char) - 1)


#if !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua"
#endif


static inline void Stop(lua_State *L, lua_Debug *ar);
static inline void Action(int i);
static inline void SetSignal(int sig, void (*handler)(int));
static lua_State *globalState;
static const char *progname = LUA_PROGNAME;
/*
** Hook set by signal function to stop the interpreter.
*/
void Stop (lua_State *L, lua_Debug *ar) {
  (void)ar;  /* unused arg. */
  lua_sethook(L, NULL, 0, 0);  /* reset hook */
  luaL_error(L, "interrupted!");
}

/*
** Function to be called at a C signal. Because a C signal cannot
** just change a Lua state (as there is no proper synchronization),
** this function only sets a hook that, when called, will stop the
** interpreter.
*/
void Action (int i) {
  int flag = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
  SetSignal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
  lua_sethook(globalState, Stop, flag, 1);
}

void SetSignal (int sig, void (*handler)(int)) {
  struct sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);  /* do not mask any signal */
  sigaction(sig, &sa, NULL);
}


class BasicLua
{
protected:
	bool mOwnsState;
	lua_State *mState = nullptr;
public:
	BasicLua(void) : mOwnsState(true) { mState = NewState(); }
	explicit BasicLua(lua_State *lState) : mOwnsState(false), mState(lState) { }
	// BasicLua(const BasicLua& lOther) : mOwnsState(false), mState(lOther.mState) { }
	~BasicLua() { if(mOwnsState) Close(); }

	typedef enum {
		OK = LUA_OK,
		ERR_RUN = LUA_ERRRUN,
		ERR_MEM = LUA_ERRMEM,
		ERR_SYNTAX = LUA_ERRSYNTAX,
		YIELD = LUA_YIELD,
		ERR_FILE = LUA_ERRFILE,
	} StatusCode;

	inline int AbsIndex(int lIdx) { return lua_absindex(mState, lIdx); }
	inline void Arith(int lOp) { lua_arith(mState, lOp); }
	lua_CFunction AtPanic(lua_CFunction lFunc) { return lua_atpanic(mState, lFunc); }
	inline void Call(int lNArgs, int lNResults) { lua_call(mState, lNArgs, lNResults); }
	inline void CallK(int lNArgs, int lNResults, lua_KContext lCtx, lua_KFunction lKFunc) { lua_callk(mState, lNArgs, lNResults, lCtx, lKFunc); }
	inline int CheckStack(int lN) { return lua_checkstack(mState, lN); }
	inline void Close(void) { lua_close(mState); }
	inline void CloseSlot(int lIndex) { lua_closeslot(mState, lIndex); }
	inline int CloseThread(lua_State *lFrom) { return lua_closethread(mState, lFrom); }
	inline int Compare(int lIndex1, int lIndex2, int lOp) { return lua_compare(mState, lIndex1, lIndex2, lOp); }
	inline void Concat(int lN) { lua_concat(mState, lN); }
	inline void Copy(int lFromIndex, int lToIndex) { lua_copy(mState, lFromIndex, lToIndex); }
	inline void CreateTable(int lNArr, int lNRec) { lua_createtable(mState, lNArr, lNRec); }
	inline int Dump(lua_Writer lWriter, void *lData, int lStrip) { return lua_dump(mState, lWriter, lData, lStrip); }
	inline int Error(void) { return lua_error(mState); }
	// TODO: varags inline int Gc()
	inline lua_Alloc GetAllcF(void **lUd) { return lua_getallocf(mState, lUd); }
	inline int GetField(int lIndex, const char *lKey) { return lua_getfield(mState, lIndex, lKey); }
	inline void *GetExtraSpace(void) { return lua_getextraspace(mState); }
	inline int GetGlobal(const char *lName) { return lua_getglobal(mState, lName); }
	inline int GetI(int lIndex, lua_Integer lInteger) { return lua_geti(mState, lIndex, lInteger); }
	inline int GetMetaTable(int lIndex) { return lua_getmetatable(mState, lIndex); }
	inline int GetTable(int lIndex) { return lua_gettable(mState, lIndex); }
	inline int GetTop(void) { return lua_gettop(mState); }
	inline int GetIUserValue(int lIndex, int lN) { return lua_getiuservalue(mState, lIndex, lN); }
	inline void Insert(int lIndex) { lua_insert(mState, lIndex); }
	inline int IsBoolean(int lIndex) { return lua_isboolean(mState, lIndex); }
	inline int IsCFunction(int lIndex) { return lua_iscfunction(mState, lIndex); }
	inline int IsFunction(int lIndex) { return lua_isfunction(mState, lIndex); }
	inline int IsInteger(int lIndex) { return lua_isinteger(mState, lIndex); }
	inline int IsLightUserData(int lIndex) { return lua_islightuserdata(mState, lIndex); }
	inline int IsNil(int lIndex) { return lua_isnil(mState, lIndex); }
	inline int IsNone(int lIndex) { return lua_isnone(mState, lIndex); }
	inline int IsNoneOrNil(int lIndex) { return lua_isnoneornil(mState, lIndex); }
	inline int IsNumber(int lIndex) { return lua_isnumber(mState, lIndex); }
	inline int IsString(int lIndex) { return lua_isstring(mState, lIndex); }
	inline int IsTable(int lIndex) { return lua_istable(mState, lIndex); }
	inline int IsThread(int lIndex) { return lua_isthread(mState, lIndex); }
	inline int IsUserData(int lIndex) { return lua_isuserdata(mState, lIndex); }
	inline int IsYieldable(void) { return lua_isyieldable(mState); }
	inline void Len(int lIndex) { return lua_len(mState, lIndex); }
	inline int Load(lua_Reader lReader, void *lData, const char *lChunkName, const char *lMode) { return lua_load(mState, lReader, lData, lChunkName, lMode); }
	inline lua_State *NewState(lua_Alloc lAlloc, void *lUd) { return lua_newstate(lAlloc, lUd); }
	inline void NewTable(void) { lua_newtable(mState); }
	inline lua_State *NewThread(void) { return lua_newthread(mState); }
	inline void *NewUserDataUV(size_t lSize, int lNuValue) { return lua_newuserdatauv(mState, lSize, lNuValue); }
	inline int Next(int lIndex) { return lua_next(mState, lIndex); }
	inline int NumberToInteger(lua_Number lNumber, lua_Integer *lInteger) { return lua_numbertointeger(lNumber, lInteger); }
	inline int PCall(int lNArgs, int lNResults, int lMsgH) { return lua_pcall(mState, lNArgs, lNResults, lMsgH); }
	inline int PCallK(int lNArgs, int lNResults, int lMsgH, lua_KContext lCtx, lua_KFunction lKFunc) { return lua_pcallk(mState, lNArgs, lNResults, lMsgH, lCtx, lKFunc); }
	inline void Pop(int lN) { lua_pop(mState, lN); }
	inline void PushBoolean(int lBool) { lua_pushboolean(mState, lBool); }
	inline void PushCClosure(lua_CFunction lFunc, int n) { lua_pushcclosure(mState, lFunc, n); }
	inline void PushCFunction(lua_CFunction lFunc) { lua_pushcfunction(mState, lFunc); }
	// TODO: varargs inline const char *PushFString()
	inline void PushGlobalTable(void) { lua_pushglobaltable(mState); }
	inline void PushInteger(lua_Integer lInteger) { lua_pushinteger(mState, lInteger); }
	inline void PushLightUserData(void *lData) { lua_pushlightuserdata(mState, lData); }
	inline const char *PushLString(const char *lStr, size_t lLen) { return lua_pushlstring(mState, lStr, lLen); }
	inline void PushNil(void) { lua_pushnil(mState); }
	inline void PushNumber(lua_Number lNum) { lua_pushnumber(mState, lNum); }
	inline const char *PushString(const char *lStr) { return lua_pushstring(mState, lStr); }
	inline int PushThread(void) { return lua_pushthread(mState); }
	inline void PushValue(int lIndex) { lua_pushvalue(mState, lIndex); }
	inline const char *PushVFString(const char *lFormat, va_list lArgp) { return lua_pushvfstring(mState, lFormat, lArgp); } // @suppress("Type cannot be resolved") // @suppress("Invalid arguments")
	inline int RawEqual(int lIndex1, int lIndex2) { return lua_rawequal(mState, lIndex1, lIndex2); }
	inline int RawGet(int lIndex) { return lua_rawget(mState, lIndex); }
	inline int RawGetI(int lIndex, lua_Integer lInteger) { return lua_rawgeti(mState, lIndex, lInteger); }
	inline int RawGetP(int lIndex, const void *lP) { return lua_rawgetp(mState, lIndex, lP); }
	inline lua_Unsigned RawLen(int lIndex) { return lua_rawlen(mState, lIndex); }
	inline void RawSet(int lIndex) { lua_rawset(mState, lIndex); }
	inline void RawSetI(int lIndex, lua_Integer lInteger) { lua_rawseti(mState, lIndex, lInteger); }
	inline void RawSetP(int lIndex, const void *lP) { lua_rawsetp(mState, lIndex, lP); }
	inline void Register(const char *lName, lua_CFunction lFunc) { lua_register(mState, lName, lFunc); }
	inline void Remove(int lIndex) { lua_remove(mState, lIndex); }
	inline void Replace(int lIndex) { lua_replace(mState, lIndex); }
	inline int ResetThread(void) { return lua_resetthread(mState); }
	inline int Resume(lua_State *lFrom, int lNArgs, int *lNResults) { return lua_resume(mState, lFrom, lNArgs, lNResults); }
	inline void Rotate(int lIndex, int lN) { lua_rotate(mState, lIndex, lN); }
	inline void SetAllocF(lua_Alloc lFunc, void *lUd) { lua_setallocf(mState, lFunc, lUd); }
	inline void SetField(int lIndex, const char *lKey) { lua_setfield(mState, lIndex, lKey); }
	inline void SetGlobal(const char *lName) { lua_setglobal(mState, lName); }
	inline void SetI(int lIndex, lua_Integer lInteger) { lua_seti(mState, lIndex, lInteger); }
	inline int SetIUserValue(int lIndex, int lN) { return lua_setiuservalue(mState, lIndex, lN); }
	inline int SetMetaTable(int lIndex) { return lua_setmetatable(mState, lIndex); }
	inline void SetTable(int lIndex) { lua_settable(mState, lIndex); }
	inline void SetTop(int lIndex) { lua_settop(mState, lIndex); }
	inline void SetWarnF(lua_WarnFunction lFunc, void *lUd) { lua_setwarnf(mState, lFunc, lUd); }
	inline int Status(void) { return lua_status(mState); }
	inline size_t StringToNumber(const char *lStr) { return lua_stringtonumber(mState, lStr); }
	inline int ToBoolean(int lIndex) { return lua_toboolean(mState, lIndex); }
	inline lua_CFunction ToCFunction(int lIndex) { return lua_tocfunction(mState, lIndex); }
	inline void ToClose(int lIndex) { lua_toclose(mState, lIndex); }
	inline int ToInteger(int lIndex) { return lua_tointeger(mState, lIndex); }
	inline int ToIntegerX(int lIndex, int *lIsNum) { return lua_tointegerx(mState, lIndex, lIsNum); }
	inline const char *ToLString(int lIndex, size_t *lLen) { return lua_tolstring(mState, lIndex, lLen); }
	inline int ToNumber(int lIndex) { return lua_tonumber(mState, lIndex); }
	inline int ToNumberX(int lIndex, int *lIsNum) { return lua_tonumberx(mState, lIndex, lIsNum); }
	inline const void *ToPointer(int lIndex) { return lua_topointer(mState, lIndex); }
	inline const char *ToString(int lIndex) { return lua_tostring(mState, lIndex); }
	inline lua_State *ToThread(int lIndex) { return lua_tothread(mState, lIndex); }
	inline void *ToUserData(int lIndex) { return lua_touserdata(mState, lIndex); }
	inline int Type(int lIndex) { return lua_type(mState, lIndex); }
	inline const char *TypeName(int lType) { return lua_typename(mState, lType); }
	inline int UpValueIndex(int i) { return lua_upvalueindex(i); }
	inline lua_Number Version(void) { return lua_version(mState); }
	inline void Warning(const char *lMsg, int lToCont) { lua_warning(mState, lMsg, lToCont); }
	inline void XMove(lua_State *lFrom, lua_State *lTo, int n) { lua_xmove(lFrom, lTo, n); }
	inline int YieldK(int lNResults, lua_KContext lCtx, lua_KFunction lKFunc) { return lua_yieldk(mState, lNResults, lCtx, lKFunc); }

	// Debug Interface
	inline lua_Hook GetHook(void) { return lua_gethook(mState); }
	inline int GetHookCount(void) { return lua_gethookcount(mState); }
	inline int GetHookMask(void) { return lua_gethookmask(mState); }
	inline int GetInfo(const char *lWhat, lua_Debug *lAr) { return lua_getinfo(mState, lWhat, lAr); }
	inline const char *GetLocal(const lua_Debug *lAr, int n) { return lua_getlocal(mState, lAr, n); }
	inline int GetStack(int lLevel, lua_Debug *lAr) { return lua_getstack(mState, lLevel, lAr); }
	inline const char *GetUpValue(int lFuncIndex, int n) { return lua_getupvalue(mState, lFuncIndex, n); }
	inline void SetHook(lua_Hook lFunc, int lMask, int lCount) { lua_sethook(mState, lFunc, lMask, lCount); }
	inline const char *SetLocal(const lua_Debug *lAr, int n) { return lua_setlocal(mState, lAr, n); }
	inline const char *SetUpValue(int lFuncIndex, int n) { return lua_setupvalue(mState, lFuncIndex, n); }
	inline void *UpValueID(int lFuncIndex, int n) { return lua_upvalueid(mState, lFuncIndex, n); }
	inline void UpValueJoin(int lFuncIndex1, int n1, int lFuncIndex2, int n2) { lua_upvaluejoin(mState, lFuncIndex1, n1, lFuncIndex2, n2); }

	// Aux. Library
	inline void AddChar(luaL_Buffer *lBuffer, char c) { luaL_addchar(lBuffer, c); }
	inline const void AddGSub(luaL_Buffer *lBuffer, const char *lStr, const char *lPtr, const char *r) { return luaL_addgsub(lBuffer, lStr, lPtr, r); }
	inline void AddLString(luaL_Buffer *lBuffer, const char *lStr, size_t lLen) { luaL_addlstring(lBuffer, lStr, lLen); }
	inline void AddSize(luaL_Buffer *lBuffer, size_t n) { luaL_addsize(lBuffer, n); }
	inline void AddString(luaL_Buffer *lBuffer, const char *lStr) { luaL_addstring(lBuffer, lStr); }
	inline void AddValue(luaL_Buffer *lBuffer) { luaL_addvalue(lBuffer); }
	inline void ArgCheck(int lCond, int lArg, const char *lExtraMsg) { luaL_argcheck(mState, lCond, lArg, lExtraMsg); }
	inline int ArgError(int lArg, const char *lExtraMsg) { return luaL_argerror(mState, lArg, lExtraMsg); }
	inline char *BuffAddr(luaL_Buffer *lBuffer) { return luaL_buffaddr(lBuffer); }
	inline void BuffInit(luaL_Buffer *lBuffer) { luaL_buffinit(mState, lBuffer); }
	inline size_t BuffLen(luaL_Buffer *lBuffer) { return luaL_bufflen(lBuffer); }
	inline char *BuffInitSize(luaL_Buffer *lBuffer, size_t lSize) { return luaL_buffinitsize(mState, lBuffer, lSize); }
	inline void BuffSub(luaL_Buffer *lBuffer, int n) { luaL_buffsub(lBuffer, n); }
	inline int CallMeta(int lObj, const char *e) { return luaL_callmeta(mState, lObj, e); }
	inline void CheckAny(int lArg) { luaL_checkany(mState, lArg); }
	inline lua_Integer CheckInteger(int lArg) { return luaL_checkinteger(mState, lArg); }
	inline const char *CheckLString(int lArg, size_t *l) { return luaL_checklstring(mState, lArg, l); }
	inline lua_Number CheckNumber(int lArg) { return luaL_checknumber(mState, lArg); }
	inline int CheckOption(int lArg, const char *lDef, const char *const lList[]) { return luaL_checkoption(mState, lArg, lDef, lList); }
	inline void CheckStack(int lSize, const char *lMsg) { luaL_checkstack(mState, lSize, lMsg); }
	inline const char *CheckString(int lArg) { return luaL_checkstring(mState, lArg); }
	inline void CheckType(int lArg, int lType) { luaL_checktype(mState, lArg, lType); }
	inline void *CheckUData(int lArg, const char *lName) { return luaL_checkudata(mState, lArg, lName); }
	inline void CheckVersion(void) { luaL_checkversion(mState); }
	inline int DoFile(const char *lFileName) { return luaL_dofile(mState, lFileName); }
	inline int DoString(const char *lStr) { return luaL_dostring(mState, lStr); }
	// TODO: varargs inline int Error()
	inline int ExecResult(int lStat) { return luaL_execresult(mState, lStat); }
	inline int FileResult(int lStat, const char *lFileName) { return luaL_fileresult(mState, lStat, lFileName); }
	inline int GetMetaField(int lObj, const char *e) { return luaL_getmetafield(mState, lObj, e); }
	inline int GetMetaTable(const char *lName) { return luaL_getmetatable(mState, lName); }
	inline int GetSubTable(int lIndex, const char *lName) { return luaL_getsubtable(mState, lIndex, lName); }
	inline const char *GSub(const char *s, const char *p, const char *r) { return luaL_gsub(mState, s, p, r); }
	inline lua_Integer LenAux(int lIndex) { return luaL_len(mState, lIndex); }
	inline int LoadBuffer(const char *lBuffer, size_t lSize, const char *lName) { return luaL_loadbuffer(mState, lBuffer, lSize, lName); }
	inline int LoadBufferX(const char *lBuffer, size_t lSize, const char *lName, const char *lMode) { return luaL_loadbufferx(mState, lBuffer, lSize, lName, lMode); }
	inline int LoadFile(const char *lFileName) { return luaL_loadfile(mState, lFileName); }
	inline int LoadFileX(const char *lFileName, const char *lMode) { return luaL_loadfilex(mState, lFileName, lMode); }
	inline int LoadString(const char *lStr) { return luaL_loadstring(mState, lStr); }
#ifdef DEFINE_NEWLIB_FUNCS
	inline void NewLib(const luaL_Reg l[]) { luaL_newlib(mState, l); }
	inline void NewLibTable(const luaL_Reg l[]) { luaL_newlibtable(mState, l); }
#endif
	inline int NewMetaTable(const char *lName) { return luaL_newmetatable(mState, lName); }
	inline lua_State *NewState(void) { return luaL_newstate(); }
	inline void OpenLibs(void) { luaL_openlibs(mState); }
	inline lua_Integer OptInteger(int lArg, lua_Integer d) { return luaL_optinteger(mState, lArg, d); }
	// TODO: these ones
	inline void PushFail(void) { luaL_pushfail(mState); }
	inline void PushResult(luaL_Buffer *lBuffer) { luaL_pushresult(lBuffer); }
	inline void PushResultSize(luaL_Buffer *lBuffer, size_t lSize) { luaL_pushresultsize(lBuffer, lSize); }
	// TODO: these ones
	inline void SetMetaTable(const char *lName) { luaL_setmetatable(mState, lName); }
	inline void *TestUData(int lArg, const char *lName) { return luaL_testudata(mState, lArg, lName); }
	inline const char *ToLStringAux(int lIndex, size_t *lLen) { return luaL_tolstring(mState, lIndex, lLen); }
	inline void Traceback(lua_State *L1, const char *lMsg, int lLevel) { luaL_traceback(mState, L1, lMsg, lLevel); }
	inline int TypeError(int lArg, const char *lName) { return luaL_typeerror(mState, lArg, lName); }
	inline const char *TypeNameAux(int lIndex) { return luaL_typename(mState, lIndex); }
	inline void Unref(int t, int ref) { luaL_unref(mState, t, ref); }
	inline void Where(int lLevel) { luaL_where(mState, lLevel); }
};


class Lua : public BasicLua
{
public:
	Lua(void);
	explicit Lua(lua_State *lState) : BasicLua(lState) { }
	~Lua() { }

	void MakeTableReadOnly(void);
	void AddRotMetaObject(int lTableIndex, const char *lSubtableName);
	void AddRotMetaObject(int lTableIndex) { AddRotMetaObject(lTableIndex, nullptr); }
	void AddRotObject(int lTableIndex) { AddRotMetaObject(lTableIndex, "Objects"); }
	void AddRotSetter(int lTableIndex) { AddRotMetaObject(lTableIndex, "Setters"); }
	void AddRotGetter(int lTableIndex) { AddRotMetaObject(lTableIndex, "Getters"); }

	void InstallRotConstructor(void);

	void GetBackdoor(void);

	// DEBUG
	void DumpStack(void);

	void REPL(void);
	void BackdoorInstall(lua_State *lState);

private:

	inline const char *GetPrompt (int firstline) {
		if (lua_getglobal(mState, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
			return (firstline ? LUA_PROMPT : LUA_PROMPT2);  /* use the default */
		else {  /* apply 'tostring' over the value */
			const char *p = luaL_tolstring(mState, -1, NULL);
			lua_remove(mState, -2);  /* remove original value */
			return p;
		}
	}

	inline int Incomplete(int status) {
		if (status == LUA_ERRSYNTAX) {
			size_t lmsg;
			const char *msg = lua_tolstring(mState, -1, &lmsg);
			if (lmsg >= marklen && strcmp(msg + lmsg - marklen, EOFMARK) == 0) {
				lua_pop(mState, 1);
				return 1;
			}
		}
		return 0;  /* else... */
	}

	inline int PushLine(int firstline) {
		char buffer[LUA_MAXINPUT];
		char *b = buffer;
		size_t l;
		const char *prmt = GetPrompt(firstline);
		int readstatus = ReadLine(mState, b, prmt);
		if (readstatus == 0)
			return 0;  /* no input (prompt will be popped by caller) */
		lua_pop(mState, 1);  /* remove prompt */
		l = strlen(b);
		if (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
			b[--l] = '\0';  /* remove it */
		if (firstline && b[0] == '=')  /* for compatibility with 5.2, ... */
			lua_pushfstring(mState, "return %s", b + 1);  /* change '=' to 'return' */
		else
			lua_pushlstring(mState, b, l);
		FreeLine(mState, b);
		return 1;
	}

	inline int AddReturn(void) {
		const char *line = lua_tostring(mState, -1);  /* original line */
		const char *retline = lua_pushfstring(mState, "return %s;", line);
		int status = luaL_loadbuffer(mState, retline, strlen(retline), "=stdin");
		if (status == LUA_OK) {
			lua_remove(mState, -2);  /* remove modified line */
			if (line[0] != '\0')  /* non empty? */
				SaveLine(mState, line);  /* keep history */
		}
		else
			lua_pop(mState, 2);  /* pop result from 'luaL_loadbuffer' and modified line */
		return status;
	}

	inline int Multiline(void) {
		for (;;) {  /* repeat until gets a complete statement */
			size_t len;
			const char *line = lua_tolstring(mState, 1, &len);  /* get what it has */
			int status = luaL_loadbuffer(mState, line, len, "=stdin");  /* try it */
			if (!Incomplete(status) || !PushLine(0)) {
				SaveLine(mState, line);  /* keep history */
				return status;  /* cannot or should not try to add continuation line */
			}
			lua_pushliteral(mState, "\n");  /* add newline... */
			lua_insert(mState, -2);  /* ...between the two lines */
			lua_concat(mState, 3);  /* join them */
		}

		return 0;
	}

	inline int LoadLine(void) {
		int status;
		SetTop(0);
		if (!PushLine(1))
			return -1;  /* no input */
		if ((status = AddReturn()) != LUA_OK)  /* 'return ...' did not work? */
			status = Multiline();  /* try as command, maybe with continuation lines */
		lua_remove(mState, 1);  /* remove line from the stack */
		return status;
	}

	/*
	 ** Prints an error message, adding the program name in front of it
	 ** (if present)
	 */
	inline void Message (const char *pname, const char *msg) {
		if (pname) lua_writestringerror("%s: ", pname);
		lua_writestringerror("%s\n", msg);
	}

	/*
	 ** Check whether 'status' is not OK and, if so, prints the error
	 ** message on the top of the stack. It assumes that the error object
	 ** is a string, as it was either generated by Lua or by 'msghandler'.
	 */
	inline int Report(int status) {
		if (status != LUA_OK) {
			const char *msg = lua_tostring(mState, -1);
			Message(progname, msg);
			lua_pop(mState, 1);  /* remove message */
		}
		return status;
	}

	inline static int MsgHandler(lua_State *L) {
		const char *msg = lua_tostring(L, 1);
		if (msg == NULL) {  /* is error object not a string? */
			if (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
					lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
				return 1;  /* that is the message */
			else
				msg = lua_pushfstring(L, "(error object is a %s value)",
						luaL_typename(L, 1));
		}
		luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
		return 1;  /* return the traceback */
	}

	/*
	 ** Prints (calling the Lua 'print' function) any values on the stack
	 */
	inline void Print(void) {
		int n = lua_gettop(mState);
		if (n > 0) {  /* any result to be printed? */
			luaL_checkstack(mState, LUA_MINSTACK, "too many results to print");
			lua_getglobal(mState, "print");
			lua_insert(mState, 1);
			if (lua_pcall(mState, n, 0, 0) != LUA_OK)
				Message(progname, lua_pushfstring(mState, "error calling 'print' (%s)",
						lua_tostring(mState, -1)));
		}
	}

	inline int DoCall(int narg, int nres) {
		int status;
		int base = lua_gettop(mState) - narg;  /* function index */
		PushCFunction(MsgHandler);  /* push message handler */
		lua_insert(mState, base);  /* put it under function and args */
		globalState = mState;  /* to be available to 'laction' */
		SetSignal(SIGINT, Action);  /* set C-signal handler */
		status = lua_pcall(mState, narg, nres, base);
		SetSignal(SIGINT, SIG_DFL); /* reset C-signal handler */
		lua_remove(mState, base);  /* remove message handler from the stack */
		return status;
	}
};


namespace LuaUtils {
	void AddGetter(Lua &lLua, const char *lGetterName, void *lUpValue, int lFunction, int (*lFunctionPtr)(lua_State *));
	void AddSetter(Lua &lLua, const char *lGetterName, void *lUpValue, int lFunction, int (*lFunctionPtr)(lua_State *));
	void AddClosure(Lua &lLua, const char *lClosureName, void *lUpValue, int (*lFunctionPtr)(lua_State *));
}


#endif /* PLATFORM_LUA_HPP_ */
