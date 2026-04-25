

#include <iostream>

#include <fcntl.h>
#include <unistd.h>

#include "rpmsg.hpp"

using namespace std;

RpMsg::RpMsg(void)
: CommandInterface()
{
	mFileDescriptor = open(RPMSG_DEVICE_PATH, O_RDWR);
	if(mFileDescriptor < 0)
	{
		perror("could not open RPMSG_DEVICE_PATH");
		exit(EXIT_FAILURE);
	}
}

RpMsg::~RpMsg(void)
{
    close(mFileDescriptor);
}

void RpMsg::Write(string lData)
{
    int lError = write(mFileDescriptor, lData.c_str(), lData.length());
    if (lError == -1)
    {
        std::cerr << "could not write to rpmsg char dev" << std::endl;
    }
}

string RpMsg::Read(void)
{
    constexpr size_t lBufferSize{1024};
    char lBuffer[lBufferSize]{{}};
    int lBytesRead = read(mFileDescriptor, lBuffer, lBufferSize);
    if (lBytesRead < 0)
    {
        std::cerr << "could not read from rpmsg char dev" << std::endl;
    }

    return string(lBuffer);
}
