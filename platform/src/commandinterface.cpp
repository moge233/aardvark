/*
 * commandinterface.cpp
 *
 *  Created on: Jan 24, 2026
 *      Author: matt
 */

#include "commandinterface.hpp"

CommandInterface::CommandInterface(size_t lQueueCapacity)
: mQueue(lQueueCapacity)
{
}

CommandInterface::~CommandInterface(void)
{
}
