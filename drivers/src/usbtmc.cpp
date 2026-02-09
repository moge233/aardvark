/*
 * usbtmc.cpp
 *
 *  Created on: Feb 6, 2024
 *      Author: matt
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "usbtmc.hpp"
#include "commandmessage.hpp"

using namespace std;

UsbTmc::UsbTmc(void)
: CommandInterface(64)
, mBulkXferIndex(0)
, mHeader({0})
{
	for(size_t i=0; i<16; i++)
	{
		memset(mXferBuffers[i].mData, 0, TMC_BULK_ENDPOINT_LENGTH + 1);
	}

	mFileDescriptor = open(TMC_DEVICE_PATH, O_RDWR);
	if(mFileDescriptor < 0)
	{
		perror("could not open TMC_DEVICE_PATH");
		exit(EXIT_FAILURE);
	}

	mREN = open(TMC_REN_PATH, O_RDWR);
	if(mREN < 0)
	{
		perror("could not open TMC_REN_PATH");
		mREN = 0;
	}

	mStatusByte = GetStatusByte();
	mRemoteLocalState = GetRemoteLocalState();
}

UsbTmc::~UsbTmc(void)
{
	close(mFileDescriptor);
}

string UsbTmc::ServiceBulkOut(gadget_tmc_header *lHeader)
{
	string lDataString;
	uint32_t lBytesRemaining = lHeader->TransferSize;

	if(lBytesRemaining % 4)
		lBytesRemaining += (4 - lBytesRemaining % 4);

	do
	{
		uint32_t lTransferSize = lBytesRemaining;

		if(lTransferSize > TMC_BULK_ENDPOINT_LENGTH)
			lTransferSize = TMC_BULK_ENDPOINT_LENGTH;

		memset(&mXferBuffers[mBulkXferIndex].mData[0], 0, TMC_BULK_ENDPOINT_LENGTH + 1);

		mXferBuffers[mBulkXferIndex].mLength = read(mFileDescriptor,
													mXferBuffers[mBulkXferIndex].mData,
													lTransferSize);
		if(mXferBuffers[mBulkXferIndex].mLength < 0)
		{
			perror("error reading bulk data");
			exit(EXIT_FAILURE);
		}
		else
		{
			lDataString.append(mXferBuffers[mBulkXferIndex].mData, mXferBuffers[mBulkXferIndex].mLength);
		}

		lBytesRemaining -= mXferBuffers[mBulkXferIndex].mLength;

		mBulkXferIndex = (++mBulkXferIndex == 16) ? 0 : mBulkXferIndex;
	} while(lBytesRemaining);

	return lDataString;
}

void UsbTmc::ServiceBulkIn(gadget_tmc_header *lHeader, string lData)
{
	Output(lHeader, lData.c_str(), lData.length());
}

void UsbTmc::Output(gadget_tmc_header *lHeader, const char *lBuffer, size_t lLength)
{
	int lBytesSent = write(mFileDescriptor, lBuffer, lLength);
	if (lBytesSent < 0)
	{
		perror("write");
	}
}

bool UsbTmc::GetHeader(gadget_tmc_header *lHeader)
{
	if (!ioctl(mFileDescriptor, GADGET_TMC_IOCTL_GET_HEADER, lHeader))
	{
		return true;
	}

	return false;
}

bool UsbTmc::Poll(void)
{
	int lTimeout = -1;
	vector<pollfd> lPollFds(1);
	lPollFds[0].fd = mFileDescriptor;
	lPollFds[0].events = POLLIN;
	lPollFds[0].revents = 0;

	int lError = poll(lPollFds.data(), lPollFds.size(), lTimeout);
	if (lError < 0)
	{
		perror("Poll");
		return false;
	}

	if (lPollFds[0].revents & POLLIN)
	{
		return true;
	}

	return false;
}

void UsbTmc::AbortBulkOut(void)
{
	int lError = ioctl(mFileDescriptor, GADGET_TMC_IOCTL_ABORT_BULK_OUT);
	if (lError)
	{
		perror("could not abort bulk out transfer");
	}
}

void UsbTmc::AbortBulkIn(void)
{
	int lError = ioctl(mFileDescriptor, GADGET_TMC_IOCTL_ABORT_BULK_IN);
	if (lError)
	{
		perror("could not abort bulk in transfer");
	}
}

void UsbTmc::SetREN(uint8_t lNewREN)
{
	if (!mREN)
	{
		return;
	}
}

uint8_t UsbTmc::GetREN(void)
{
	uint8_t lNewREN = 0;

	if (!mREN)
	{
		return 0;
	}

	constexpr char lCommand[] = "cat /sys/kernel/config/usb_gadget/g1/functions/tmc.g1/REN";
	FILE *lPipe = popen(lCommand, "r");
	if (lPipe != nullptr)
	{
		char lBuffer[10];
		if (fgets(lBuffer, 10, lPipe))
		{
			char *lEnd{};
			lNewREN = std::strtol(lBuffer, &lEnd, 10); // @suppress("Function cannot be resolved")
		}
		pclose(lPipe);
	}
	return lNewREN;
}

void UsbTmc::SetRemoteLocalState(gadget_tmc488_localremote_state lNewState)
{
	int lError = ioctl(mFileDescriptor, GADGET_TMC488_IOCTL_SET_RL_STATE, &lNewState);
	if (lError)
	{
		perror("could not set remote-local state");
	}
}

gadget_tmc488_localremote_state UsbTmc::GetRemoteLocalState(void)
{
	gadget_tmc488_localremote_state lState;
	int lError = ioctl(mFileDescriptor, GADGET_TMC488_IOCTL_GET_RL_STATE, &lState);
	if (lError)
	{
		perror("could not get remote-local state");
	}

	return lState;
}

void UsbTmc::SetStatusByte(uint32_t lStatusByte)
{
	int lError = ioctl(mFileDescriptor, GADGET_TMC488_IOCTL_SET_STB, &lStatusByte);
	if(lError)
	{
		perror("could not set status byte");
	}
}

uint32_t UsbTmc::GetStatusByte(void)
{
	uint32_t lStatusByte = 0;
	int lError = ioctl(mFileDescriptor, GADGET_TMC488_IOCTL_GET_STB, &lStatusByte);
	if(lError)
	{
		perror("could not read status byte");
	}
	return lStatusByte;
}
