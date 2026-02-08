/*
 * commandmessage.cpp
 *
 *  Created on: Jan 24, 2026
 *      Author: matt
 */


#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <utility>

#include "commandmessage.hpp"

CommandMessage::CommandMessage(void)
{
	CommandMessage(static_cast<char *>(nullptr), 0, static_cast<void *>(nullptr));
}

CommandMessage::CommandMessage(const char *lMessage, unsigned long lLength, void *lOrigin)
: mLength{lLength}
, mOrigin{lOrigin}
{
	if (mLength)
	{
		mMessage = new char [mLength + 1];
		if (!mMessage)
		{
			exit(EXIT_FAILURE);
		}

		std::memmove(reinterpret_cast<void *>(const_cast<char *>(mMessage)), lMessage, mLength);
		mMessage[mLength] = '\0';
	}
	else
	{
		mMessage = static_cast<char *>(nullptr);
	}
}

CommandMessage::~CommandMessage(void)
{
	if (mMessage)
	{
		delete [] mMessage;
	}

	mOrigin = static_cast<void *>(nullptr);
}

CommandMessage::CommandMessage(CommandMessage& lOther)
: mLength{lOther.GetLength()}
, mOrigin{lOther.GetOrigin()}
{
	mMessage = new char [mLength + 1];
	std::memmove(lOther.mMessage, mMessage, lOther.GetLength());
	mMessage[mLength + 1] = '\0';
}

CommandMessage& CommandMessage::operator=(CommandMessage& lOther)
{
	if (this != &lOther)
	{
		if (this->mMessage)
		{
			delete [] mMessage;
		}

		if (lOther.GetData() && lOther.GetLength() > 0)
		{
			mLength = lOther.GetLength();
			mMessage = new char[mLength];
			if (!mMessage)
			{
				exit(EXIT_FAILURE);
			}

			memcpy(reinterpret_cast<void *>(const_cast<char *>(mMessage)), lOther.GetData(), mLength);
			mMessage[mLength] = '\0';
		}
		else
		{
			mMessage = nullptr;
			mLength = 0;
		}
	}

	return *this;
}

void CommandMessage::Swap(CommandMessage& lOther)
{
	mLength = lOther.mLength;
	mMessage = lOther.mMessage;
	mOrigin = lOther.mOrigin;
}
