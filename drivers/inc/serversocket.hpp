/*
 * serversocket.hpp
 *
 *  Created on: Mar 14, 2025
 *      Author: matt
 */

#ifndef LAN_SERVERSOCKET_HPP_
#define LAN_SERVERSOCKET_HPP_


#include <string>

using namespace std;

class ServerSocket
{
	public:
		virtual ~ServerSocket(void) = default;
		virtual void Main(void) = 0;
};

#endif /* LAN_SERVERSOCKET_HPP_ */
