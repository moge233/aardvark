/*
 * lua.cpp
 *
 *  Created on: Mar 3, 2024
 *      Author: matt
 */


#include <cstdio>

#include "lua.hpp"
#include "scriptprocessor.hpp"

// static const char sMakeReadOnlyTableKey[] = "MakeReadOnlyTable";
static const char sMakeReadOnlyTable[] =
		"	return function(t)"
		"		local mt = { Objects = {}, Setters = {}, Getters = {} }"
		"		mt.__metatable = mt"
		"		mt.__index = 	function(t, k)"
		"							local g = mt.Getters[k]"
		"							if(g) then"
		"								return g()"
		"							else"
		"								return mt.Objects[k]"
		"							end"
		"						end"
		"		mt.__newindex = function(t, k, v)"
		"							local s = mt.Setters[k]"
		"							if(s) then"
		"								return s(v)"
		"							else"
		"								error([[cannot modify read-only table]], 2)"
		"							end"
		"						end"
		"		setmetatable(t, mt)"
		"		return t"
		"	end";

static const char sInstance[] = "Instance";
static const char sBackdoor[] = "Backdoor";


#if 0
Lua::Lua(void) : BasicLua() {
#else
Lua::Lua(void) {
#endif
	InstallRotConstructor();

	PushLightUserData(static_cast<void *>(const_cast<char *>(sInstance)));
	PushLightUserData(reinterpret_cast<void *>(this));
	SetTable(LUA_REGISTRYINDEX);

	PushLightUserData(static_cast<void *>(const_cast<char *>(sBackdoor)));
	NewTable();
	SetTable(LUA_REGISTRYINDEX);
}

void Lua::MakeTableReadOnly(void) {
	// Fetch the converter function
	PushLightUserData(static_cast<void *>(const_cast<char *>(sMakeReadOnlyTable)));

	GetTable(LUA_REGISTRYINDEX);

	Insert(-2);

	// Run the converter function
	PCall(1, 1, 0);

}

void Lua::AddRotMetaObject(int lTableIndex, const char *lSubtableName) {

	GetMetaTable(lTableIndex);

	/*
	 * Stack: (top down; * = top)
	 * 		: *
	 */

	if(lSubtableName) {
		PushString(lSubtableName);
		GetTable(-2);
		Insert(-4);
		Pop(1);
	}
	else {
		Insert(-3);
	}
	SetTable(-3);
	Pop(1);
}

void Lua::InstallRotConstructor(void) {

	PushLightUserData(static_cast<void *>(const_cast<char *>(sMakeReadOnlyTable)));

	LoadBuffer(sMakeReadOnlyTable, sizeof(sMakeReadOnlyTable) - 1, "");	// * func() userdata

	PCall(0, 1, 0);	// * func() userdata

	/*
	 * t[k] = v
	 * t is the value at LUA_REGISTRYINDEX (the registry table)
	 * k is the value just below the top of the stack
	 * v is the value at the top of the stack
	 *
	 * t = the registry table
	 * k = key for the converter function
	 * v = converter function
	 *
	 * REGISTRYTABLE[sMakeReadOnlyTableKey] = converter_function
	 */
	SetTable(LUA_REGISTRYINDEX);
}

void Lua::GetBackdoor(void) {
	PushLightUserData(static_cast<void *>(const_cast<char *>(sBackdoor)));

	/*
	 * Pushes that value at t[k] onto the stack
	 *
	 * t --> value at the given index (LUA_REGISTRYINDEX)
	 * k --> value on the top of the stack
	 *
	 * t = the registry table
	 * k = Backdoor
	 *
	 * t[k] = REGISTRYTABLE[Backdoor]
	 */
	GetTable(LUA_REGISTRYINDEX);

}

void Lua::DumpStack(void) {
	static unsigned int lCallNumber = 0;
	printf("%s %u\n", __func__, ++lCallNumber);

	int top = GetTop();
	for (int i=1; i <= top; i++) {
		printf("%d\t%s\t", i, TypeName(Type(i)));
		switch (Type(i)) {
		case LUA_TNUMBER:
			printf("%d\n",ToNumber(i));
			break;
		case LUA_TSTRING:
			printf("%s\n",ToString(i));
			break;
		case LUA_TBOOLEAN:
			printf("%s\n", (ToBoolean(i) ? "true" : "false"));
			break;
		case LUA_TNIL:
			printf("%s\n", "nil");
			break;
		case LUA_TLIGHTUSERDATA:
			printf("%s\n", static_cast<char *>(ToUserData(i)));
			break;
		default:
			printf("%p\n",ToPointer(i));
			break;
		}
	}
}

void Lua::REPL(void)
{
	int status;
	while ((status = LoadLine()) != -1) {
		if (status == LUA_OK)
		{
			status = DoCall(0, LUA_MULTRET);
		}
		if(status == LUA_OK)
		{
			Print();
		}
		else
		{
			Report(status);
		}
	}
	return;
}

void Lua::BackdoorInstall(lua_State *lState) {

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


namespace LuaUtils {

	void AddGetter(Lua &lLua, const char *lGetterName, void *lUpValue, int lFunction, int (*lFunctionPtr)(lua_State *)) {

		/*
		 * Stack: (top down)
		 * 		: * {table} {DeviceTable}
		 */

		// AddGetter
		lLua.PushString(lGetterName);
		lLua.PushLightUserData(lUpValue);
		lLua.PushInteger(lFunction);
		lLua.PushCClosure(lFunctionPtr, 2);

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
	}

	void AddSetter(Lua &lLua, const char *lSetterName, void *lUpValue, int lFunction, int (*lFunctionPtr)(lua_State *)) {

		// AddSetter
		lLua.PushString(lSetterName);
		lLua.PushLightUserData(lUpValue);
		lLua.PushInteger(lFunction);
		lLua.PushCClosure(lFunctionPtr, 2);

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
	}

	void AddClosure(Lua &lLua, const char *lClosureName, void *lUpValue, int (*lFunctionPtr)(lua_State *)) {

		/*
		 * Stack: (top down)
		 * 		: * led DeviceTable
		 */
		lLua.PushString(lClosureName);
		lLua.PushLightUserData(lUpValue);
		lLua.PushCClosure(lFunctionPtr, 1);

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
	}
}
