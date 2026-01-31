/*
 * logicaldevice.hpp
 *
 *  Created on: Mar 15, 2024
 *      Author: matt
 */

#ifndef PLATFORM_LOGICALDEVICE_HPP_
#define PLATFORM_LOGICALDEVICE_HPP_


#include <cstddef>
#include <cstdint>

#include "lua.hpp"


class LogicalDevice
{
public:
	uint16_t mLogicalId;
	uint16_t mRemotelyAccessible;

	LogicalDevice(void) : mLogicalId(0), mRemotelyAccessible(0) { }
	virtual ~LogicalDevice(void) { }

	virtual int HandleMsg(lua_State *lState) = 0;
};

class LocalLogicalDevice : public LogicalDevice
{
public:
	constexpr static size_t MAX_DEVICES = 64;

	static LocalLogicalDevice *smRegistry[MAX_DEVICES];

	LocalLogicalDevice(void);
	explicit LocalLogicalDevice(unsigned short lReservedId);
	~LocalLogicalDevice(void) { smRegistry[mLogicalId] = nullptr; }

	void Register(void);
	LocalLogicalDevice *Find(unsigned short lId);

	static void DeviceTableInit(lua_State *lState);
	static void DeviceTablePromote(lua_State *lState);
	static void DeviceTableGet(Lua *lLua);
	static void DeviceTableGet(lua_State *lState);

	static void MakeTable(Lua *lLua);
};


#endif /* PLATFORM_LOGICALDEVICE_HPP_ */
