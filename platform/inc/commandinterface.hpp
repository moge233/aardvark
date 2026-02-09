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
#include "circulbarbuffer.hpp"
#include "endpoint.hpp"

class CommandInterface : public Endpoint
{
public:
	CommandInterface(size_t lQueueCapacity);
	~CommandInterface();

private:
	CircularBuffer mQueue;
};


#endif /* AARDVARK_PLATFORM_INC_COMMANDINTERFACE_HPP_ */
