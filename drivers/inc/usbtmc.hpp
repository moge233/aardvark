/*
 * usbtmc.hpp
 *
 *  Created on: Feb 6, 2024
 *      Author: matt
 */

#ifndef USB_USBTMC_HPP_
#define USB_USBTMC_HPP_

#include <cstddef>
#include <cstdint>
#include <string>

#include <linux/usb/g_tmc.h>

#include "commandinterface.hpp"

constexpr char TMC_DEVICE_PATH[] = "/dev/tmc";
constexpr char TMC_REN_PATH[] = "/sys/kernel/config/usb_gadget/g1/functions/tmc.g1/REN";

constexpr size_t TMC_BULK_ENDPOINT_LENGTH = 512;

using namespace std;

struct UsbTmcXferBuffer {
	int32_t mLength;
	char mData[TMC_BULK_ENDPOINT_LENGTH + 1];
};

class UsbTmc : public CommandInterface
{
private:
	int mFileDescriptor;
	int mREN;				// REN file descriptor
	int mStatusByte;		// StatusByte file descriptor
	int mRemoteLocalState;	// RemoteLocalState file descriptor
	struct UsbTmcXferBuffer mXferBuffers[16];
	size_t mBulkXferIndex;
	gadget_tmc_header mHeader;

public:
	UsbTmc(void);
	~UsbTmc();
	UsbTmc(UsbTmc &) = delete;
	UsbTmc &operator=(UsbTmc &) = delete;

	inline const int &GetFileDescriptor() const { return mFileDescriptor; }

	string ServiceBulkOut(gadget_tmc_header *lHeader);
	void ServiceBulkIn(gadget_tmc_header *lHeader, string lData);
	void Output(gadget_tmc_header *lHeader, const char *lBuffer, size_t lLength);
	bool GetHeader(gadget_tmc_header *lHeader);
	bool Poll(void);
	void AbortBulkOut(void);
	void AbortBulkIn(void);
	void SetREN(uint8_t lNewREN);
	uint8_t GetREN(void);

	void SetRemoteLocalState(gadget_tmc488_localremote_state lNewState);
	gadget_tmc488_localremote_state GetRemoteLocalState(void);
	void SetStatusByte(uint32_t lNewStb);
	uint32_t GetStatusByte(void);
};

#endif /* USB_USBTMC_HPP_ */
