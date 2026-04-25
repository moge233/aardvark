

#ifndef MONITOR_HPP_
#define MONITOR_HPP_

#include <libudev.h>

#include "commandinterface.hpp"


class MonitorEvent
{
    public:
        MonitorEvent(const char *lDeviceName, const char *lAction, const char *lEvent);
        ~MonitorEvent(void);
	    MonitorEvent(MonitorEvent& lOther) = delete;
	    MonitorEvent& operator=(MonitorEvent& lOther) = delete;

        inline char *GetName(void) { return mDeviceName; }
        inline char *GetAction(void) { return mAction; }
        inline char *GetEvent(void) { return mEvent; }
    private:
        char *mDeviceName;
        char *mAction;
        char *mEvent;
};

class Monitor
{
    public:
        Monitor(const char *lName);
        ~Monitor();
        MonitorEvent *WaitForEvent(void); // Blocking
    private:
        struct udev *mUdev;
        struct udev_monitor *mMonitor;
        int mFileDescriptor;
        char *mName;
};

#endif // MONITOR_HPP_