/*
 * logicaldevice.cpp
 *
 *  Created on: Mar 15, 2024
 *      Author: matt
 */


#include "logicaldevice.hpp"


LocalLogicalDevice *LocalLogicalDevice::smRegistry[LocalLogicalDevice::MAX_DEVICES] = { nullptr };


static char sDeviceTableIndex[] = "DeviceTableIndex";


LocalLogicalDevice::LocalLogicalDevice(void) {
	Register();
	mRemotelyAccessible = true;
}

void LocalLogicalDevice::Register(void) {
	for(mLogicalId = 2; mLogicalId < MAX_DEVICES; ++mLogicalId) {
		if(nullptr == smRegistry[mLogicalId]) {
			smRegistry[mLogicalId] = this;
		}
	}
}

LocalLogicalDevice *LocalLogicalDevice::Find(unsigned short lId) {
	if(lId < MAX_DEVICES) {
		return smRegistry[lId];
	}
	else {
		return nullptr;
	}
}

void LocalLogicalDevice::DeviceTableInit(lua_State *lState) {
	Lua lLua(lState);

	// Create the device table as a read-only table
	lLua.PushLightUserData(sDeviceTableIndex);
	lLua.NewTable();
	lLua.MakeTableReadOnly();

	/*
	 * Stack: (top down)
	 * 		: {} "DeviceTableIndex"
	 */

	// Put the device table into the registry for safekeeping.
	lLua.SetTable(LUA_REGISTRYINDEX);			// registry["DeviceTableIndex"] = {}

	// Put the device table into localnode.
	lLua.PushLightUserData(sDeviceTableIndex);
	lLua.GetTable(LUA_REGISTRYINDEX);
	lLua.SetGlobal("localnode");
}

void LocalLogicalDevice::DeviceTablePromote(lua_State *lState) {
	Lua lLua(lState);							// Lua Stack (* = top of stack)
												// *
#if 0
	lLua.PushLightUserData(sDeviceTableIndex);	// "DeviceTableIndex" *
	lLua.RawGet(LUA_REGISTRYINDEX);				// devices *
	lLua.GetMetaTable(-1);						// devices devices_meta *
	lLua.PushString("Objects");					// devices devices_meta "Objects" *
	lLua.GetTable(-2);							// devices devices_meta Objects *

	lLua.PushNil();								// devices devices_meta Objects nil *

	while(lLua.Next(-2) != 0) {					// devices devices_meta Objects key value *
		// Make this entry global
		lLua.PushValue(-2);						// devices devices_meta Objects key value key *
		lLua.Insert(-2);						// devices devices_meta Objects key key value *

		// t[k] = v
		// t = value at index
		// v = value at top of stack
		// k = value just below the top
		lLua.RawSet(LUA_REGISTRYINDEX);
	}
#else
#endif
}

void LocalLogicalDevice::DeviceTableGet(Lua *lLua) {
	lLua->PushLightUserData(sDeviceTableIndex);
	lLua->GetTable(LUA_REGISTRYINDEX);
}

void LocalLogicalDevice::DeviceTableGet(lua_State *lState) {
	Lua lLua(lState);
	DeviceTableGet(&lLua);
}

void LocalLogicalDevice::MakeTable(Lua *lLua) {
	lLua->NewTable();
	lLua->MakeTableReadOnly();
}
