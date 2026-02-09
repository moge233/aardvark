/*
 * led.hpp
 *
 *  Created on: Apr 30, 2024
 *      Author: matt
 */

#ifndef LED_INC_LED_HPP_
#define LED_INC_LED_HPP_

#include "lua.hpp"

const char LED_BRIGHTNESS_PATH[] = "/sys/class/leds/am62-sk:green:debug/brightness";

void LedInstall(lua_State *lState);

enum LedFunctions {
	FUNC_GET = 0,
	FUNC_SET
};

class Led
{
public:

	typedef enum {
		OFF,
		ON
	} State;

	Led(void);
	~Led(void);

	void On(void);
	void Off(void);
	void Toggle(void);

	static int On(lua_State *lState);
	static int Off(lua_State *lState);
	static int Toggle(lua_State *lState);
	static int HandleMsg(lua_State *lState);

private:
	int mFileDescriptor;
	State mState;

public:
	State GetState(void) { return mState; }
};


extern Led gLed;


#endif /* LED_INC_LED_HPP_ */
