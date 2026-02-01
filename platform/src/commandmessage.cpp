/*
 * commandmessage.cpp
 *
 *  Created on: Jan 24, 2026
 *      Author: matt
 */


#include <cstdlib>
#include <cstring>
#include <iostream>
#include <utility>

#include "commandmessage.hpp"

CommandMessage::CommandMessage(void)
{
	CommandMessage(static_cast<char *>(nullptr), 0);
}

CommandMessage::CommandMessage(const char *lMessage, unsigned long lLength)
: mLength{lLength}
{
	if (mLength)
	{
		mMessage = new char [mLength + 1];
		if (!mMessage)
		{
			exit(EXIT_FAILURE);
		}

		memcpy(reinterpret_cast<void *>(const_cast<char *>(mMessage)), lMessage, mLength);
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
}

CommandMessage::CommandMessage(CommandMessage& lOther)
{
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
