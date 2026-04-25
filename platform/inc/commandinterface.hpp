/*
 * commandinterface.hpp
 *
 *  Created on: Jan 24, 2026
 *      Author: matt
 */

#ifndef AARDVARK_PLATFORM_INC_COMMANDINTERFACE_HPP_
#define AARDVARK_PLATFORM_INC_COMMANDINTERFACE_HPP_

#include <cstdint>

#include "commandmessage.hpp"
#include "circularbuffer.hpp"
#include "endpoint.hpp"

class CommandInterface : public Endpoint
{
public:
	CommandInterface(void);
		~CommandInterface();
};


#endif /* AARDVARK_PLATFORM_INC_COMMANDINTERFACE_HPP_ */
