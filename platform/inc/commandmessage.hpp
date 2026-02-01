/*
 * commandmessage.hpp
 *
 *  Created on: Jan 24, 2026
 *      Author: matt
 */

#ifndef AARDVARK_PLATFORM_INC_COMMANDMESSAGE_HPP_
#define AARDVARK_PLATFORM_INC_COMMANDMESSAGE_HPP_

#include <cstddef>

class CommandMessage
{
public:
	CommandMessage(void);
	CommandMessage(const char *lMessage, unsigned long lLength);
	~CommandMessage();
	CommandMessage(CommandMessage& lOther);
	CommandMessage& operator=(CommandMessage& lOther);

	inline const char *GetData(void) { return mMessage; };
	inline unsigned long GetLength(void) { return mLength; };
private:
	char *mMessage;
	unsigned long mLength;
};


#endif /* AARDVARK_PLATFORM_INC_COMMANDMESSAGE_HPP_ */
