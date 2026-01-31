/*
 * vxi11.hpp
 *
 *  Created on: Mar 30, 2025
 *      Author: matt
 */

#ifndef LAN_VXI11_HPP_
#define LAN_VXI11_HPP_

#include "clientinterface.hpp"
#include "serversocket.hpp"

#include <mutex>
#include <string>

extern "C"
{
#include "vxi11.h"
}

using namespace std;

namespace VXI11
{
	string SERVER_NAME("VXI11.server");

	constexpr ssize_t MAX_BUFFER_SIZE = 2048;

	constexpr Device_Flags WAITLOCK_BIT = (1 << 0);
	constexpr Device_Flags END_BIT = (1 << 3);
	constexpr Device_Flags TERMCHRSET_BIT = (1 << 7);

	constexpr long READ_REASON_END_BIT = (1 << 2);
	constexpr long READ_REASON_CHR_BIT = (1 << 1);
	constexpr long READ_REASON_REQCNT_BIT = (1 << 0);

	Device_Error *AbortWrapper(Device_Link *lArgp, struct svc_req *lRqstp);
	Create_LinkResp *CreateLinkWrapper(Create_LinkParms *lArgp, struct svc_req *lRqstp);
	Device_WriteResp *WriteWrapper(Device_WriteParms *lArgp, struct svc_req *lRqstp);
	Device_ReadResp *ReadWrapper(Device_ReadParms *lArgp, struct svc_req *lRqstp);
	Device_ReadStbResp *ReadSTBWrapper(Device_GenericParms *lArgp, struct svc_req *lRqstp);
	Device_Error *TriggerWrapper(Device_GenericParms *lArgp, struct svc_req *lRqstp);
	Device_Error *ClearWrapper(Device_GenericParms *lArgp, struct svc_req *lRqstp);
	Device_Error *RemoteWrapper(Device_GenericParms *lArgp, struct svc_req *lRqstp);
	Device_Error *LocalWrapper(Device_GenericParms *lArgp, struct svc_req *lRqstp);
	Device_Error *LockWrapper(Device_LockParms *lArgp, struct svc_req *lRqstp);
	Device_Error *UnlockWrapper(Device_Link *lArgp, struct svc_req *lRqstp);
	Device_Error *EnableSRQWrapper(Device_EnableSrqParms *lArgp, struct svc_req *lRqstp);
	Device_DocmdResp *DoCMDWrapper(Device_DocmdParms *lArgp, struct svc_req *lRqstp);
	Device_Error *DestroyLinkWrapper(Device_Link *lArgp, struct svc_req *lRqstp);
	Device_Error *CreateInterruptChannelWrapper(Device_RemoteFunc *lArgp, struct svc_req *lRqstp);
	Device_Error *DestroyInterruptChannelWrapper(void *lArgp, struct svc_req *lRqstp);
	void *InterruptSRQWrapper(Device_SrqParms *lArgp, struct svc_req *lRqstp);

	class Device : public ClientInterface
	{
		public:
			Device(void);
			~Device(void);

			Device_Error *Abort(Device_Link *lArgp, struct svc_req *lRqstp);
			Device_WriteResp *Write(Device_WriteParms *lArgp, struct svc_req *lRqstp);
			Device_ReadResp *Read(Device_ReadParms *lArgp, struct svc_req *lRqstp);
			Device_ReadStbResp *ReadSTB(Device_GenericParms *lArgp, struct svc_req *lRqstp);
			Device_Error *Trigger(Device_GenericParms *lArgp, struct svc_req *lRqstp);
			Device_Error *Clear(Device_GenericParms *lArgp, struct svc_req *lRqstp);
			Device_Error *Remote(Device_GenericParms *lArgp, struct svc_req *lRqstp);
			Device_Error *Local(Device_GenericParms *lArgp, struct svc_req *lRqstp);
			Device_Error *Lock(Device_LockParms *lArgp, struct svc_req *lRqstp);
			Device_Error *Unlock(Device_Link *lArgp, struct svc_req *lRqstp);
			Device_Error *EnableSRQ(Device_EnableSrqParms *lArgp, struct svc_req *lRqstp);
			Device_DocmdResp *DoCMD(Device_DocmdParms *lArgp, struct svc_req *lRqstp);
			Device_Error *CreateInterruptChannel(Device_RemoteFunc *lArgp, struct svc_req *lRqstp);
			Device_Error *DestroyInterruptChannel(void *lArgp, struct svc_req *lRqstp);
			void *InterruptSRQ(Device_SrqParms *lArgp, struct svc_req *lRqstp);

		private:
			Device_Error mError;
			Device_WriteResp mWriteResponse;
			Device_ReadResp mReadResponse;
			Device_ReadStbResp mReadStbResponse;
			Device_DocmdResp mDoCmdResponse;

			char mReadBuffer[MAX_BUFFER_SIZE];
			char mWriteBuffer[MAX_BUFFER_SIZE];
	};

	class Server : public ServerSocket
	{
		public:
			explicit Server(const char *lServerName);
			void Main();

	int GetClientSocket() const {
		return mClientSocket;
	}

	int GetFileDescriptor() const {
		return mFileDescriptor;
	}

	int GetServerSocket() const {
		return mServerSocket;
	}

		private:
			int mFileDescriptor;
			int mServerSocket;
			int mClientSocket;
	};

	class InstrumentServer : public Server
	{
		public:
			static constexpr Device_Link cLinkId = 64;
			static constexpr long cMaxReceiveSize = MAX_BUFFER_SIZE;
			static constexpr long cAbortPort = 25;

			InstrumentServer(const char *lServerName);
			~InstrumentServer(void);

			Device *GetDevice() { return mDevice; }

			void Main(void);

			static void DeviceAsync(struct svc_req *lRqstp, SVCXPRT *lTransportHandle);
			static void DeviceCore(struct svc_req *lRqstp, SVCXPRT *lTransportHandle);
			static void DeviceInterrupt(struct svc_req *lRqstp, SVCXPRT *lTransportHandle);

			Create_LinkResp *CreateLink(Create_LinkParms *lArgp, struct svc_req *lRqstp);
			Device_Error *DestroyLink(Device_Link *lArgp, struct svc_req *lRqstp);

		private:
			char *mName;
			SVCXPRT *mTransportHandle;
			Device *mDevice;

			bool mLinkCreated;
			bool mLocked;
			// long mLinkId;
			long mMaxRecvSize;
			short mAbortPort;

			Create_LinkResp mResponse;
			Device_Error mError;
	};
}


extern VXI11::InstrumentServer *gVxi11Instrument0;


#endif /* LAN_VXI11_HPP_ */
