/*
 * vxi11.cpp
 *
 *  Created on: Mar 30, 2025
 *      Author: matt
 */

#include <csignal>

#include "vxi11.hpp"

using namespace VXI11;

inline static bool flag_bit_is_set(Device_Flags lFlags, Device_Flags lBit)
{
	return static_cast<bool>((lFlags & lBit) >> lBit);
}

InstrumentServer *gInstrumentServer;
static Device_Link sLinkID = -1;
static bool sTermCharBitSet = false;

Device_Error *VXI11::AbortWrapper(Device_Link *lArgp, struct svc_req *lRqstp)
{
	return gInstrumentServer->GetDevice()->Abort(lArgp, lRqstp);
}

Create_LinkResp *VXI11::CreateLinkWrapper(Create_LinkParms *lArgp, struct svc_req *lRqstp)
{
	return gInstrumentServer->CreateLink(lArgp, lRqstp);
}

Device_WriteResp *VXI11::WriteWrapper(Device_WriteParms *lArgp, struct svc_req *lRqstp)
{
	return gInstrumentServer->GetDevice()->Write(lArgp, lRqstp);
}

Device_ReadResp *VXI11::ReadWrapper(Device_ReadParms *lArgp, struct svc_req *lRqstp)
{
	return gInstrumentServer->GetDevice()->Read(lArgp, lRqstp);
}

Device_ReadStbResp *VXI11::ReadSTBWrapper(Device_GenericParms *lArgp, struct svc_req *lRqstp)
{
	return gInstrumentServer->GetDevice()->ReadSTB(lArgp, lRqstp);
}

Device_Error *VXI11::TriggerWrapper(Device_GenericParms *lArgp, struct svc_req *lRqstp)
{
	return gInstrumentServer->GetDevice()->Trigger(lArgp, lRqstp);
}

Device_Error *VXI11::ClearWrapper(Device_GenericParms *lArgp, struct svc_req *lRqstp)
{
	return gInstrumentServer->GetDevice()->Clear(lArgp, lRqstp);
}

Device_Error *VXI11::RemoteWrapper(Device_GenericParms *lArgp, struct svc_req *lRqstp)
{
	return gInstrumentServer->GetDevice()->Remote(lArgp, lRqstp);
}

Device_Error *VXI11::LocalWrapper(Device_GenericParms *lArgp, struct svc_req *lRqstp)
{
	return gInstrumentServer->GetDevice()->Local(lArgp, lRqstp);
}

Device_Error *VXI11::LockWrapper(Device_LockParms *lArgp, struct svc_req *lRqstp)
{
	return gInstrumentServer->GetDevice()->Lock(lArgp, lRqstp);
}

Device_Error *VXI11::UnlockWrapper(Device_Link *lArgp, struct svc_req *lRqstp)
{
	return gInstrumentServer->GetDevice()->Unlock(lArgp, lRqstp);
}

Device_Error *VXI11::EnableSRQWrapper(Device_EnableSrqParms *lArgp, struct svc_req *lRqstp)
{
	return gInstrumentServer->GetDevice()->EnableSRQ(lArgp, lRqstp);
}

Device_DocmdResp *VXI11::DoCMDWrapper(Device_DocmdParms *lArgp, struct svc_req *lRqstp)
{
	return gInstrumentServer->GetDevice()->DoCMD(lArgp, lRqstp);
}

Device_Error *VXI11::DestroyLinkWrapper(Device_Link *lArgp, struct svc_req *lRqstp)
{
	return gInstrumentServer->DestroyLink(lArgp, lRqstp);
}

Device_Error *VXI11::CreateInterruptChannelWrapper(Device_RemoteFunc *lArgp, struct svc_req *lRqstp)
{
	return gInstrumentServer->GetDevice()->CreateInterruptChannel(lArgp, lRqstp);
}

Device_Error *VXI11::DestroyInterruptChannelWrapper(void *lArgp, struct svc_req *lRqstp)
{
	return gInstrumentServer->GetDevice()->DestroyInterruptChannel(lArgp, lRqstp);
}

void *VXI11::InterruptSRQWrapper(Device_SrqParms *lArgp, struct svc_req *lRqstp)
{
	return gInstrumentServer->GetDevice()->InterruptSRQ(lArgp, lRqstp);
}


Server::Server(const char *lServer)
: mFileDescriptor{0}
, mServerSocket{0}
, mClientSocket{0}
{
    mServerSocket = socket(AF_INET, SOCK_STREAM, 0);

    int lOpt = 1;
    if (setsockopt(mServerSocket, SOL_SOCKET, SO_REUSEADDR, &lOpt, sizeof(lOpt)))
    {
        perror("setsockopt (SO_REUSEADDR)");
        close(mServerSocket);
        exit(EXIT_FAILURE);
    }

    // char lDeviceName[] = "eth0";
    // if (setsockopt(mServerSocket, SOL_SOCKET, SO_BINDTODEVICE, &lDeviceName, strlen(lDeviceName) + 1))
    // {
    //     perror("setsockopt (SO_BINDTODEVICE)");
    //     close(mServerSocket);
    //     exit(EXIT_FAILURE);
    // }

    struct sockaddr_in lServerSockAddr;
    memset(&lServerSockAddr, 0, sizeof(lServerSockAddr));
    lServerSockAddr.sin_family = AF_INET;
    lServerSockAddr.sin_port = htons(4880);
    lServerSockAddr.sin_addr.s_addr = inet_addr(lServer);

    int lError = bind(mServerSocket, reinterpret_cast<struct sockaddr *>(&lServerSockAddr), sizeof(lServerSockAddr));
    if (lError)
    {
        perror("bind");
        close(mServerSocket);
        exit(EXIT_FAILURE);
    }

    lError = listen(mServerSocket, 1);
    if (lError)
    {
        perror("listen");
        close(mServerSocket);
        exit(EXIT_FAILURE);
    }
}

void Server::Main(void)
{
    struct sockaddr_in lClientAddr;
    socklen_t lLen = sizeof(lClientAddr);

    while ((mClientSocket = accept(mServerSocket, reinterpret_cast<struct sockaddr *>(&lClientAddr), &lLen)) > 0)
    {
		while (mClientSocket > 0)
		{
			char lReadBuffer[1024];
			int lBytesRead = recv(mClientSocket, lReadBuffer, 1023, 0);
			if (lBytesRead < 0)
			{
				perror("read");
				close(mClientSocket);
				close(mServerSocket);
				exit(EXIT_FAILURE);
			}
			else if (lBytesRead == 0)
			{
				close(mClientSocket);
				mClientSocket = 0;
			}
			else
			{
#ifdef SERVER_SOCKET_ECHO
				write(mClientSocket, lReadBuffer, strnlen(lReadBuffer, 1023));
#endif

				if (!strncmp(lReadBuffer, "exit", 4))
				{
					close(mClientSocket);
					mClientSocket = 0;
				}
				else
				{
					/*
					 * Send the transfer size to the command server so it is ready to read the entire data sequence
					 */
					// Send(reinterpret_cast<uint32_t *>(&lBytesRead));
					// Send(lReadBuffer, lBytesRead);
				}
			}
		}
    }
	return;
}

InstrumentServer::InstrumentServer(const char *lServerName)
: Server(lServerName)
, mName{const_cast<char *>(lServerName)}
, mLinkCreated{false}
, mLocked{false}
, mMaxRecvSize{2048}
, mAbortPort{25}
{
	mDevice = new Device;
	if (!mDevice)
	{
		exit(EXIT_FAILURE);
	}

	pmap_unset(DEVICE_ASYNC, DEVICE_ASYNC_VERSION);
	pmap_unset(DEVICE_CORE, DEVICE_CORE_VERSION);
	pmap_unset(DEVICE_INTR, DEVICE_INTR_VERSION);

	mTransportHandle = svctcp_create(GetServerSocket(), 0, 0);
	if (mTransportHandle)
	{
		if (!svc_register(mTransportHandle, DEVICE_ASYNC, DEVICE_ASYNC_VERSION, InstrumentServer::DeviceAsync, IPPROTO_TCP))
		{
			fprintf(stderr, "%s", "unable to register (DEVICE_ASYNC, DEVICE_ASYNC_VERSION, tcp).");
			exit(1);
		}
		if (!svc_register(mTransportHandle, DEVICE_CORE, DEVICE_CORE_VERSION, InstrumentServer::DeviceCore, IPPROTO_TCP)) {
			fprintf(stderr, "%s", "unable to register (DEVICE_CORE, DEVICE_CORE_VERSION, tcp).");
			exit(1);
		}
		if (!svc_register(mTransportHandle, DEVICE_INTR, DEVICE_INTR_VERSION, InstrumentServer::DeviceInterrupt, IPPROTO_TCP)) {
			fprintf(stderr, "%s", "unable to register (DEVICE_INTR, DEVICE_INTR_VERSION, tcp).");
			exit(1);
		}
	}
}

InstrumentServer::~InstrumentServer()
{
	;
}

void InstrumentServer::Main(void)
{
	svc_run(); // Should not return
	fprintf(stderr, "%s", "svc_run returned");
	exit(1);
}

void InstrumentServer::DeviceAsync(struct svc_req *lRqstp, SVCXPRT *lTransportHandle)
{
	union
	{
		Device_Link device_abort_1_arg;
	} lArg;
	char *lResult;
	xdrproc_t lXdrArgument;
	xdrproc_t lXdrResult;
	char *(*lLocalFxn)(char *, struct svc_req *);

	// Determine the desired procedure and act on it
	switch (lRqstp->rq_proc)
	{
		case NULLPROC:
		{
			// NULLPROC reply to let them know the server is running
			svc_sendreply(lTransportHandle, reinterpret_cast<xdrproc_t>(xdr_void), static_cast<char *>(nullptr));
			return;
		}
		case device_abort:
			// DEVICE ABORT procedure
			lXdrArgument = reinterpret_cast<xdrproc_t>(xdr_Device_Link);
			lXdrResult = reinterpret_cast<xdrproc_t>(xdr_Device_Error);
			lLocalFxn = reinterpret_cast<char *(*)(char *, struct svc_req *)>(AbortWrapper);
			break;
		default:
			svcerr_noproc(lTransportHandle);
			break;
	}

	memset(reinterpret_cast<char *>(&lArg), 0, sizeof(lArg));
	if (!svc_getargs(lTransportHandle, reinterpret_cast<xdrproc_t>(lXdrArgument), reinterpret_cast<caddr_t>(&lArg)))
	{
		svcerr_decode(lTransportHandle);
		return;
	}

	lResult = (*lLocalFxn)(reinterpret_cast<char *>(&lArg), lRqstp);
	if (lResult != NULL && !svc_sendreply(lTransportHandle, reinterpret_cast<xdrproc_t>(lXdrResult), lResult))
	{
		svcerr_systemerr(lTransportHandle);
	}
	if (!svc_freeargs(lTransportHandle, reinterpret_cast<xdrproc_t>(lXdrArgument), reinterpret_cast<caddr_t>(&lArg)))
	{
		fprintf(stderr, "%s", "unable to free arguments");
		exit(1);
	}
	return;
}

void InstrumentServer::DeviceCore(struct svc_req *lRqstp, SVCXPRT *lTransportHandle)
{
	union
	{
		Create_LinkParms create_link_1_arg;
		Device_WriteParms device_write_1_arg;
		Device_ReadParms device_read_1_arg;
		Device_GenericParms device_readstb_1_arg;
		Device_GenericParms device_trigger_1_arg;
		Device_GenericParms device_clear_1_arg;
		Device_GenericParms device_remote_1_arg;
		Device_GenericParms device_local_1_arg;
		Device_LockParms device_lock_1_arg;
		Device_Link device_unlock_1_arg;
		Device_EnableSrqParms device_enable_srq_1_arg;
		Device_DocmdParms device_docmd_1_arg;
		Device_Link destroy_link_1_arg;
		Device_RemoteFunc create_intr_chan_1_arg;
	} lArg;
	char *lResult;
	xdrproc_t lXdrArgument;
	xdrproc_t lXdrResult;
	char *(*lLocal)(char *, struct svc_req *);

	switch (lRqstp->rq_proc)
	{
		case NULLPROC:
			(void) svc_sendreply(lTransportHandle, reinterpret_cast<xdrproc_t>(xdr_void), static_cast<char *>(nullptr));
			return;

		case create_link:
			lXdrArgument = reinterpret_cast<xdrproc_t>(xdr_Create_LinkParms);
			lXdrResult = reinterpret_cast<xdrproc_t>(xdr_Create_LinkResp);
			lLocal = reinterpret_cast<char *(*)(char *, struct svc_req *)>(CreateLinkWrapper);
			break;

		case device_write:
			lXdrArgument = reinterpret_cast<xdrproc_t>(xdr_Device_WriteParms);
			lXdrResult = reinterpret_cast<xdrproc_t>(xdr_Device_WriteResp);
			lLocal = reinterpret_cast<char *(*)(char *, struct svc_req *)>(WriteWrapper);
			break;

		case device_read:
			lXdrArgument = reinterpret_cast<xdrproc_t>(xdr_Device_ReadParms);
			lXdrResult = reinterpret_cast<xdrproc_t>(xdr_Device_ReadResp);
			lLocal = reinterpret_cast<char *(*)(char *, struct svc_req *)>(ReadWrapper);
			break;

		case device_readstb:
			lXdrArgument = reinterpret_cast<xdrproc_t>(xdr_Device_GenericParms);
			lXdrResult = reinterpret_cast<xdrproc_t>(xdr_Device_ReadStbResp);
			lLocal = reinterpret_cast<char *(*)(char *, struct svc_req *)>(ReadSTBWrapper);
			break;

		case device_trigger:
			lXdrArgument = reinterpret_cast<xdrproc_t>(xdr_Device_GenericParms);
			lXdrResult = reinterpret_cast<xdrproc_t>(xdr_Device_Error);
			lLocal = reinterpret_cast<char *(*)(char *, struct svc_req *)>(TriggerWrapper);
			break;

		case device_clear:
			lXdrArgument = reinterpret_cast<xdrproc_t>(xdr_Device_GenericParms);
			lXdrResult = reinterpret_cast<xdrproc_t>(xdr_Device_Error);
			lLocal = reinterpret_cast<char *(*)(char *, struct svc_req *)>(ClearWrapper);
			break;

		case device_remote:
			lXdrArgument = reinterpret_cast<xdrproc_t>(xdr_Device_GenericParms);
			lXdrResult = reinterpret_cast<xdrproc_t>(xdr_Device_Error);
			lLocal = reinterpret_cast<char *(*)(char *, struct svc_req *)>(RemoteWrapper);
			break;

		case device_local:
			lXdrArgument = reinterpret_cast<xdrproc_t>(xdr_Device_GenericParms);
			lXdrResult = reinterpret_cast<xdrproc_t>(xdr_Device_Error);
			lLocal = reinterpret_cast<char *(*)(char *, struct svc_req *)>(LocalWrapper);
			break;

		case device_lock:
			lXdrArgument = reinterpret_cast<xdrproc_t>(xdr_Device_LockParms);
			lXdrResult = reinterpret_cast<xdrproc_t>(xdr_Device_Error);
			lLocal = reinterpret_cast<char *(*)(char *, struct svc_req *)>(LockWrapper);
			break;

		case device_unlock:
			lXdrArgument = reinterpret_cast<xdrproc_t>(xdr_Device_Link);
			lXdrResult = reinterpret_cast<xdrproc_t>(xdr_Device_Error);
			lLocal = reinterpret_cast<char *(*)(char *, struct svc_req *)>(UnlockWrapper);
			break;

		case device_enable_srq:
			lXdrArgument = reinterpret_cast<xdrproc_t>(xdr_Device_EnableSrqParms);
			lXdrResult = reinterpret_cast<xdrproc_t>(xdr_Device_Error);
			lLocal = reinterpret_cast<char *(*)(char *, struct svc_req *)>(EnableSRQWrapper);
			break;

		case device_docmd:
			lXdrArgument = reinterpret_cast<xdrproc_t>(xdr_Device_DocmdParms);
			lXdrResult = reinterpret_cast<xdrproc_t>(xdr_Device_DocmdResp);
			lLocal = reinterpret_cast<char *(*)(char *, struct svc_req *)>(DoCMDWrapper);
			break;

		case destroy_link:
			lXdrArgument = reinterpret_cast<xdrproc_t>(xdr_Device_Link);
			lXdrResult = reinterpret_cast<xdrproc_t>(xdr_Device_Error);
			lLocal = reinterpret_cast<char *(*)(char *, struct svc_req *)>(DestroyLinkWrapper);
			break;

		case create_intr_chan:
			lXdrArgument = reinterpret_cast<xdrproc_t>(xdr_Device_RemoteFunc);
			lXdrResult = reinterpret_cast<xdrproc_t>(xdr_Device_Error);
			lLocal = reinterpret_cast<char *(*)(char *, struct svc_req *)>(CreateInterruptChannelWrapper);
			break;

		case destroy_intr_chan:
			lXdrArgument = reinterpret_cast<xdrproc_t>(xdr_void);
			lXdrResult = reinterpret_cast<xdrproc_t>(xdr_Device_Error);
			lLocal = reinterpret_cast<char *(*)(char *, struct svc_req *)>(DestroyInterruptChannelWrapper);
			break;

		default:
			svcerr_noproc(lTransportHandle);
			return;
	}

	memset((char *)&lArg, 0, sizeof (lArg));

	if (!svc_getargs(lTransportHandle, reinterpret_cast<xdrproc_t>(lXdrArgument), (caddr_t) &lArg))
	{
		svcerr_decode(lTransportHandle);
		return;
	}

	lResult = reinterpret_cast<char *>((*lLocal)(reinterpret_cast<char *>(&lArg), lRqstp));

	if (lResult != nullptr)
	{
		if (!svc_sendreply(lTransportHandle, reinterpret_cast<xdrproc_t>(lXdrResult), lResult))
		{
			svcerr_systemerr(lTransportHandle);
		}
	}

	if (!svc_freeargs(lTransportHandle, reinterpret_cast<xdrproc_t>(lXdrArgument), reinterpret_cast<caddr_t>(&lArg)))
	{
		fprintf(stderr, "%s", "unable to free arguments");
		exit (1);
	}

	return;
}

void InstrumentServer::DeviceInterrupt(svc_req *lRqstp, SVCXPRT *lTransportHandle)
{
	return;
}

Create_LinkResp *InstrumentServer::CreateLink(Create_LinkParms *lArgp, struct svc_req *lRqstp)
{
	printf("%s: lArgp->device = %s\n", __func__, lArgp->device);
	bool lLockDevice = static_cast<bool>(lArgp->lockDevice);
	long lLockTimeout = lArgp->lock_timeout;

	if (!mLinkCreated)
	{
		if (lLockDevice)
		{
			mLocked = true;
		}

		mResponse.lid = cLinkId;
		mResponse.maxRecvSize = cMaxReceiveSize;
		mResponse.abortPort = cAbortPort;
		mResponse.error = static_cast<Device_ErrorCode>(0);

		sLinkID = cLinkId;
		mLinkCreated = true;
	}
	else
	{
		mResponse.error = static_cast<Device_ErrorCode>(9);
	}

	return &mResponse;
}

Device_Error *InstrumentServer::DestroyLink(Device_Link *lArgp, struct svc_req *lRqstp)
{
	printf("%s\n", __func__);

	long lLinkId = *lArgp;
	if (lLinkId == sLinkID)
	{

		// TODO: Disable the link from using the interrupt mechanism

		mLinkCreated = false;
		mLocked = false;
		sLinkID = 0;

		memset(&mResponse, 0, sizeof(mResponse));
		memset(&mError, 0, sizeof(mError));

		mError.error = static_cast<Device_ErrorCode>(0);
	}
	else
	{
		printf("\terror attempting to destroy link id %ld; received %ld\n", sLinkID, lLinkId);
		mError.error = static_cast<Device_ErrorCode>(4);
	}

	return &mError;
}

Device::Device()
: ClientInterface(WEB_SERVER_CLIENT_NAME, COMMAND_SERVER_NAME)
, mReadBuffer{0}
{
	memset(&mError, 0, sizeof(mError));
	memset(&mWriteResponse, 0, sizeof(mWriteResponse));
	memset(&mReadResponse, 0, sizeof(mReadResponse));
	memset(&mReadStbResponse, 0, sizeof(mReadStbResponse));
	memset(&mDoCmdResponse, 0, sizeof(mDoCmdResponse));
}

Device::~Device()
{
	;
}

Device_Error *Device::Abort(Device_Link *lArgp, svc_req *lRqstp)
{
	printf("%s\n", __func__);

	return &mError;
}

Device_WriteResp *Device::Write(Device_WriteParms *lArgp, struct svc_req *lRqstp)
{
	Device_Link lLinkID = lArgp->lid;
	unsigned int lDataLen = lArgp->data.data_len;
	char *lData = lArgp->data.data_val;
	Device_Flags lFlags = lArgp->flags;
	memset(mWriteBuffer, 0, 2048);
	memcpy(mWriteBuffer, lData, lDataLen);

	printf("%s: lDataLen: %u; mWriteBuffer: %s; lFlags: %ld\n", __func__, lDataLen, mWriteBuffer, lFlags);

	if (lLinkID != sLinkID)
	{
		mWriteResponse.error = static_cast<Device_ErrorCode>(4);
	}

	/*
	 * Send the transfer size to the command server so it is ready to read the entire data sequence
	 */
	if (Send(reinterpret_cast<uint32_t *>(&lDataLen)) == -1)
	{
		// ERROR
		perror("error sending size to command server\n");
	}
	else
	{
		ssize_t lCountBytesSent = Send(mWriteBuffer, lDataLen);
		if (lCountBytesSent == -1)
		{
			// ERROR
			perror("error sending data to command server\n");
		}
		else
		{
			// SUCCESS
			mWriteResponse.size = lCountBytesSent;
			mWriteResponse.error = static_cast<Device_ErrorCode>(0);
		}
	}

	return &mWriteResponse;
}

Device_ReadResp *Device::Read(Device_ReadParms *lArgp, struct svc_req *lRqstp)
{
	Device_Link lLinkID = lArgp->lid;
	unsigned long lRequestSize = lArgp->requestSize;
	unsigned long lIOTimeout = lArgp->io_timeout;
	unsigned long lLockTimeout = lArgp->lock_timeout;
	Device_Flags lFlags = lArgp->flags;
	char lTermChar = lArgp->termChar;


	if (lLinkID != sLinkID)
	{
		// Bad link ID
	}

	if (flag_bit_is_set(lFlags, TERMCHRSET_BIT))
	{
		sTermCharBitSet = true;
	}
	else
	{
		sTermCharBitSet = false;
	}

	// First read the size from the command server
	uint32_t lSize;
	if (Receive(&lSize) <= 0)
	{
		perror("could not receive size from command server");
		mReadResponse.error = static_cast<Device_ErrorCode>(17);
	}

	memset(mReadBuffer, 0, 2048);
	int lLength = Receive(mReadBuffer, lSize);
	if (lLength < 0)
	{
		perror("receive from command server failed\n");
	}
	else
	{
		if (lLength == lRequestSize)
		{
			mReadResponse.reason |= READ_REASON_REQCNT_BIT;
		}
		else if (lLength < lRequestSize)
		{
			mReadResponse.reason |= READ_REASON_END_BIT;
		}
		if (sTermCharBitSet)
		{
			mReadResponse.reason |= READ_REASON_CHR_BIT;
			if (lLength < MAX_BUFFER_SIZE)
			{
				mReadBuffer[lLength] = lTermChar;
				lLength += 1;
			}
		}
		mReadResponse.data.data_len = lLength;
		mReadResponse.data.data_val = mReadBuffer;
		mReadResponse.error = static_cast<Device_ErrorCode>(0);
	}

	return &mReadResponse;
}

Device_ReadStbResp *Device::ReadSTB(Device_GenericParms *lArgp, struct svc_req *lRqstp)
{
	printf("%s\n", __func__);

	return &mReadStbResponse;
}

Device_Error *Device::Trigger(Device_GenericParms *lArgp, struct svc_req *lRqstp)
{
	printf("%s\n", __func__);

	return &mError;
}

Device_Error *Device::Clear(Device_GenericParms *lArgp, struct svc_req *lRqstp)
{
	printf("%s\n", __func__);

	return &mError;
}

Device_Error *Device::Remote(Device_GenericParms *lArgp, struct svc_req *lRqstp)
{
	printf("%s\n", __func__);

	return &mError;
}

Device_Error *Device::Local(Device_GenericParms *lArgp, struct svc_req *lRqstp)
{
	printf("%s\n", __func__);

	return &mError;
}

Device_Error *Device::Lock(Device_LockParms *lArgp, struct svc_req *lRqstp)
{
	// TODO: Try to get the lock for timeout milliseconds
	// TODO: If device_abort is called during execution, error needs set to 23

	printf("%s\n", __func__);

	return &mError;
}

Device_Error *Device::Unlock(Device_Link *lArgp, struct svc_req *lRqstp)
{
	printf("%s\n", __func__);

	return &mError;
}

Device_Error *Device::EnableSRQ(Device_EnableSrqParms *lArgp, struct svc_req *lRqstp)
{
	static Device_Error sError;

	printf("%s\n", __func__);

	return &mError;
}

Device_DocmdResp *Device::DoCMD(Device_DocmdParms *lArgp, struct svc_req *lRqstp)
{
	printf("%s\n", __func__);

	return &mDoCmdResponse;
}

Device_Error *Device::CreateInterruptChannel(Device_RemoteFunc *lArgp, struct svc_req *lRqstp)
{
	printf("%s\n", __func__);

	return &mError;
}

Device_Error *Device::DestroyInterruptChannel(void *lArgp, struct svc_req *lRqstp)
{
	printf("%s\n", __func__);

	return &mError;
}

void *Device::InterruptSRQ(Device_SrqParms *lArgp, struct svc_req *lRqstp)
{
	static void *sRet;

	return sRet;
}

#ifdef RUN_VXI11_SERVER_DAEMON

#include <csignal>

static int InitDaemon(void);

// Server gVXI11Server("192.168.1.197");

#ifdef __cplusplus
extern "C" {
#endif

void SignalHandler(int lSigNum)
{
    switch(lSigNum)
    {
        case SIGTERM:
            exit(0); // @suppress("Function cannot be resolved")
            break;
        default:
            break;
    }
    return;
}

#ifdef __cplusplus
}
#endif

int main(int argc, char *argv[])
{
	gInstrumentServer = new InstrumentServer("192.168.1.196");
	if (gInstrumentServer == nullptr)
	{
		exit(EXIT_FAILURE);
	}

    signal(SIGTERM, &SignalHandler);

	int lErr = InitDaemon();
	if(lErr)
	{
		perror("could not create VXI11::Server daemon");
		return -1;
	}

	return 0;
}


static int InitDaemon(void)
{
    pid_t lPid = fork();
    if(lPid < 0)
    {
        perror("could not fork() for daemon");
        return lPid;
    }
    if(lPid > 0)
    {
        return 0;
    }

    if(setsid() < 0)
    {
        perror("could not set SID for daemon");
        return -1;
    }

    while(true)
    {
    	gInstrumentServer->Main();
    }
    return 0;
}
#endif
