/*
 * interfaceadatpor.hpp
 *
 *  Created on: Jan 24, 2026
 *      Author: matt
 */

#ifndef AARDVARK_PLATFORM_INC_INTERFACEADAPTOR_HPP_
#define AARDVARK_PLATFORM_INC_INTERFACEADAPTOR_HPP_

#include <cstddef>

#include "commandinterface.hpp"

class InterfaceAdaptor
{
    constexpr static size_t MAX_INTERFACES{4};

    public:
        InterfaceAdaptor(void);
        ~InterfaceAdaptor() { };
        void Connect(CommandInterface& lInterface);
    private:
        int mInterfaces[MAX_INTERFACES];
};

#endif  // AARDVARK_PLATFORM_INC_INTERFACEADAPTOR_HPP_
