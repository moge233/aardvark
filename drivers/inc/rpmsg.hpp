/*
 * rpmsg.hpp
 *
 *  Created on: Feb 15, 2026
 *      Author: matt
 */

 #ifndef DRIVERS_RPMSG_HPP_
 #define DRIVERS_RPMSG_HPP_

#include <string>

#include "commandinterface.hpp"

using namespace std;

constexpr char RPMSG_DEVICE_PATH[] = "/dev/rpmsg1";

class RpMsg : public CommandInterface
{
    public:
        RpMsg(void);
        ~RpMsg(void);
        RpMsg(RpMsg &) = delete;
        RpMsg &operator=(RpMsg &) = delete;

        void Write(string lData);
        string Read(void);
    private:
        int mFileDescriptor;
};

 #endif // DRIVERS_RPMSG_HPP_
