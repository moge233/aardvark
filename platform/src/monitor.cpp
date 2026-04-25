

#include <cstdlib>
#include <cstring>
#include <iostream>

#include <poll.h>

#include "monitor.hpp"


MonitorEvent::MonitorEvent(const char *lDeviceName, const char *lAction, const char *lEvent)
{
    size_t lDeviceNameLen = strlen(lDeviceName);
    size_t lActionLen = strlen(lAction);
    size_t lEventLen = strlen(lEvent);

    mDeviceName = new char [lDeviceNameLen];
    if (!mDeviceName)
    {
        perror("could not allocate buffer for device name");
        exit(EXIT_FAILURE);
    }

    mAction = new char [lActionLen];
    if (!mAction)
    {
        perror("could not allocate buffer for device name");
        delete [] mDeviceName;
        exit(EXIT_FAILURE);
    }

    mEvent = new char [lEventLen];
    if (!mEvent)
    {
        perror("could not allocate buffer for device name");
        delete [] mDeviceName;
        delete [] mAction;
        exit(EXIT_FAILURE);
    }

    strcpy(mDeviceName, lDeviceName);
    strcpy(mAction, lAction);
    strcpy(mEvent, lEvent);
}

MonitorEvent::~MonitorEvent(void)
{
    if (mDeviceName)
    {
        delete [] mDeviceName;
    }

    if (mAction)
    {
        delete [] mAction;
    }

    if (mEvent)
    {
        delete [] mEvent;
    }
}

Monitor::Monitor(const char *lName)
{
    mName = new char [strlen(lName)];
    if (!mName)
    {
        perror("could not allocate space for monitor name");
        exit(EXIT_FAILURE);
    }
    strcpy(mName, lName);

    mUdev = udev_new();
    if (!mUdev)
    {
        std::cout << "could not create new udev" << std::endl;
        exit(EXIT_FAILURE);
    }

    mMonitor = udev_monitor_new_from_netlink(mUdev, "kernel");
    if (!mMonitor)
    {
        std::cout << "could not create new monitor from netlink" << std::endl;
        while (mUdev)
        {
            mUdev = udev_unref(mUdev);
        }
        std::cout << "could not create new monitor" << std::endl;
        exit(EXIT_FAILURE);
    }

    int lError = udev_monitor_filter_add_match_subsystem_devtype(mMonitor, lName, nullptr);
    if (lError)
    {
        std::cout << "could not add match subsystem devtype" << std::endl;
    }

    lError = udev_monitor_enable_receiving(mMonitor);
    if (lError)
    {
        std::cout << "could not enable receiving" << std::endl;
    }

    mFileDescriptor = udev_monitor_get_fd(mMonitor);
}

Monitor::~Monitor()
{
    mFileDescriptor = -1;

    if (mMonitor)
    {
        mMonitor = udev_monitor_unref(mMonitor);
        mMonitor = nullptr;
    }

    if (mUdev)
    {
        mUdev = udev_unref(mUdev);
        mUdev = nullptr;
    }
}

MonitorEvent *Monitor::WaitForEvent(void)
{
    struct pollfd lPollFds[1];
    lPollFds[0].fd = mFileDescriptor;
    lPollFds[0].events = POLLIN;
    int lCount = poll(lPollFds, 1, -1);
    if (lCount > 0)
    {
        if (lPollFds[0].revents & POLLIN)
        {
            struct udev_device *lDev = udev_monitor_receive_device(mMonitor);
            if (lDev)
            {
                const char *lName = udev_device_get_sysname(lDev);
                if (lName && !strcmp(lName, mName))
                {
                    const char *lAction = udev_device_get_action(lDev);
                    const char *lEvent = udev_device_get_property_value(lDev, "EVENT");

                    MonitorEvent *lMonitorEvent = new MonitorEvent(lName, lAction, lEvent);
                    udev_device_unref(lDev);
                    return lMonitorEvent;
                }
                else
                {
                    udev_device_unref(lDev);
                }
            }
        }
    }
    else if (lCount == 0)
    {
        // Timeout
        std::cerr << "timeout (errno: " << errno << ")" << std::endl;
    }
    else
    {
        std::cerr << "could not monitor for events (errno: " << errno << ")" << std::endl;
    }

    return nullptr;
}
