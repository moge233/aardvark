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
	CommandMessage(const char *lMessage, unsigned long lLength, void *lOrigin);
	~CommandMessage();
	CommandMessage(CommandMessage& lOther);
	CommandMessage& operator=(CommandMessage& lOther);

	inline const char *GetData(void) const { return mMessage; };
	inline unsigned long GetLength(void) const { return mLength; };
	inline void *GetOrigin(void) const { return mOrigin; };
private:
	char *mMessage;
	unsigned long mLength;
	void *mOrigin;

	void Swap(CommandMessage& lOther);
};


#endif /* AARDVARK_PLATFORM_INC_COMMANDMESSAGE_HPP_ */
