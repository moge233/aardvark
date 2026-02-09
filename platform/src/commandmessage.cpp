/*
 * commandmessage.cpp
 *
 *  Created on: Jan 24, 2026
 *      Author: matt
 */

#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>

#include "commandmessage.hpp"

using namespace std;

CommandMessage::CommandMessage(const char *lMessage, unsigned long lLength, void *lOrigin)
: mLength{lLength}
, mOrigin{lOrigin}
{
	mMessage = new char [mLength + 1];
	if (!mMessage)
	{
		exit(EXIT_FAILURE);
	}

	memcpy(reinterpret_cast<void *>(const_cast<char *>(mMessage)), lMessage, mLength);
	mMessage[mLength] = '\0';
}

CommandMessage::~CommandMessage(void)
{
	mLength = 0;
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
	memcpy(mMessage, lOther.mMessage, mLength);
	mMessage[mLength] = '\0';
}

CommandMessage& CommandMessage::operator=(CommandMessage& lOther)
{
	mLength = lOther.GetLength();
	mMessage = new char[mLength + 1];
	if (!mMessage)
	{
		exit(EXIT_FAILURE);
	}

	memcpy(const_cast<char *>(mMessage), lOther.GetData(), mLength);
	mMessage[mLength] = '\0';

	return *this;
}

void CommandMessage::Swap(CommandMessage& lOther)
{
	mLength = lOther.mLength;
	mMessage = lOther.mMessage;
	mOrigin = lOther.mOrigin;
}
