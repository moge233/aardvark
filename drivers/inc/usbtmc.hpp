/*
 * usbtmc.hpp
 *
 *  Created on: Feb 6, 2024
 *      Author: matt
 */

#ifndef DRIVERS_USBTMC_HPP_
#define DRIVERS_USBTMC_HPP_

#include <cstddef>
#include <cstdint>
#include <string>

#include <linux/usb/g_tmc.h>

#include "commandinterface.hpp"
#include "monitor.hpp"

constexpr char TMC_DEVICE_PATH[] = "/dev/tmc";

constexpr size_t TMC_BULK_ENDPOINT_LENGTH = 512;

void UsbTmcMonitorCallback(void *lArg);

using namespace std;

struct UsbTmcXferBuffer {
	int32_t mLength;
	char mData[TMC_BULK_ENDPOINT_LENGTH + 1];
};

class UsbTmc : public CommandInterface, public Monitor
{
private:
	int mFileDescriptor;
	uint32_t mREN;				// REN file descriptor
	uint32_t mStatusByte;		// StatusByte file descriptor
	uint32_t mRemoteLocalState;	// RemoteLocalState file descriptor
	struct UsbTmcXferBuffer mXferBuffers[16];
	size_t mBulkXferIndex;

public:
	UsbTmc(void);
	~UsbTmc();
	UsbTmc(UsbTmc &) = delete;
	UsbTmc &operator=(UsbTmc &) = delete;

	inline const int &GetFileDescriptor() const { return mFileDescriptor; }

	string ServiceBulkOut(struct gadget_tmc_header *lHeader);
	void ServiceBulkIn(string lData);
	void Output(const char *lBuffer, size_t lLength);
	bool GetHeader(struct gadget_tmc_header *lHeader);
	bool Poll(void);
	void AbortBulkOut(void);
	void AbortBulkIn(void);
	void SetREN(uint8_t lNewREN);
	uint8_t GetREN(void);

	void SetRemoteLocalState(enum gadget_tmc488_localremote_state lNewState);
	enum gadget_tmc488_localremote_state GetRemoteLocalState(void);
	void SetStatusByte(uint32_t lNewStb);
	uint32_t GetStatusByte(void);
};

#endif /* DRIVERS_USBTMC_HPP_ */
