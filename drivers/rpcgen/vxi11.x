
typedef long Device_Link;
typedef long Device_Flags;
typedef long Device_ErrorCode;

struct Device_Error
{
    Device_ErrorCode error;
};

struct Device_GenericParms
{
    Device_Link         lid;            /* Device_Link ID from create_link */
    Device_Flags        flags;          /* flags with options */
    unsigned long       lock_timeout;   /* Time to wait for a lock */
    unsigned long       io_timeout;     /* Time to wait for I/O */
};

struct Create_LinkParms
{
    long                clientId;       /* Implementation-specific value */
    bool                lockDevice;     /* Attempt to lock the device */
    unsigned long       lock_timeout;   /* Time to wait on a lock */
    string              device<>;       /* Name of a device */
};

struct Create_LinkResp
{
    Device_ErrorCode    error;
    Device_Link         lid;
    unsigned short      abortPort;      /* for the abort rpc */
    unsigned long       maxRecvSize;    /* specifies max data size in bytes device will accept on a write */
};

struct Device_WriteParms
{
    Device_Link         lid;            /* link ID from create_link */
    unsigned long       io_timeout;     /* time to wait for I/O */
    unsigned long       lock_timeout;   /* time to wait for lock */
    Device_Flags        flags;
    opaque              data<>;         /* the data length and the data itself */
};

struct Device_WriteResp
{
    Device_ErrorCode    error;
    unsigned long       size;           /* Number of bytes written */
};

struct Device_ReadParms
{
    Device_Link         lid;            /* link ID from create_link */
    unsigned long       requestSize;    /* Bytes requested */
    unsigned long       io_timeout;     /* Time to wait for I/O */
    unsigned long       lock_timeout;   /* Time to wait for lock */
    Device_Flags        flags;
    char                termChar;       /* Valid if flags & termchrset */
};

struct Device_ReadResp
{
    Device_ErrorCode    error;
    long                reason;         /* Reason(s) read completed */
    opaque              data<>;         /* data_len and data_val */
};

struct Device_ReadStbResp
{
    Device_ErrorCode    error;          /* Error code */
    unsigned char       stb;            /* The returned status byte */
};

struct Device_LockParms
{
    Device_Link         lid;            /* Link ID from create_link */
    Device_Flags        flags;          /* Contains the waitlock flag */
    unsigned long       lock_timeout;   /* Time to wait to acquire lock */
};

enum Device_AddrFamily
{
    DEVICE_TCP,
    DEVICE_UDP
};

struct Device_RemoteFunc
{
    unsigned long       hostAddr;       /* Host servicing the interrupt */
    unsigned short      hostPort;       /* Valid port number on client */
    unsigned long       progNum;        /* DEVICE_INTR */
    unsigned long       progVers;       /* DEVICE_INTR_VERSION */
    Device_AddrFamily   progFamily;     /* DEVICE_TCP | DEVICE_UDP */
};

struct Device_EnableSrqParms
{
    Device_Link         lid;
    bool                enable;         /* Enable or disable interrupts */
    opaque              handle<40>;     /* Host-specific data */
};

struct Device_DocmdParms
{
    Device_Link         lid;            /* Link ID from create_link */
    Device_Flags        flags;          /* Flags specifying various options */
    unsigned long       io_timeout;     /* Time to wait for I/O */
    unsigned long       lock_timeout;   /* Time to wait on a lock */
    long                cmd;            /* Which command to execute */
    bool                network_order;  /* Client's byte order */
    long                data_size;      /* Size of individual data elements */
    opaque              data_in<>;      /* Docmd data parameters */
};

struct Device_DocmdResp
{
    Device_ErrorCode    error;          /* Returned status */
    opaque              data_out<>;     /* Returned data parameter */
};

struct Device_SrqParms
{
    opaque              handle<>;
};


program DEVICE_ASYNC
{
    version DEVICE_ASYNC_VERSION
    {
        Device_Error            device_abort        (Device_Link)           = 1;
    } = 1;
} = 0x0607B0;

program DEVICE_CORE
{
    version DEVICE_CORE_VERSION
    {
        Create_LinkResp         create_link         (Create_LinkParms)      = 10;
        Device_WriteResp        device_write        (Device_WriteParms)     = 11;
        Device_ReadResp         device_read         (Device_ReadParms)      = 12;
        Device_ReadStbResp      device_readstb      (Device_GenericParms)   = 13;
        Device_Error            device_trigger      (Device_GenericParms)   = 14;
        Device_Error            device_clear        (Device_GenericParms)   = 15;
        Device_Error            device_remote       (Device_GenericParms)   = 16;
        Device_Error            device_local        (Device_GenericParms)   = 17;
        Device_Error            device_lock         (Device_LockParms)      = 18;
        Device_Error            device_unlock       (Device_Link)           = 19;
        Device_Error            device_enable_srq   (Device_EnableSrqParms) = 20;
        Device_DocmdResp        device_docmd        (Device_DocmdParms)     = 22;
        Device_Error            destroy_link        (Device_Link)           = 23;
        Device_Error            create_intr_chan    (Device_RemoteFunc)     = 25;
        Device_Error            destroy_intr_chan   (void)                  = 26;
    } = 1;
} = 0x0607AF;

program DEVICE_INTR
{
    version DEVICE_INTR_VERSION
    {
        void                    device_intr_srq     (Device_SrqParms)       = 30;
    } = 1;
} = 0x0607B1;
