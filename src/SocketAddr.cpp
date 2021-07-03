/*
   Copyright © 2017-2019, orcaer@yeah.net  All rights reserved.
   Author: orcaer@yeah.net
   Last modified: 2019-9-11
   Description: https://github.com/wlgq2/uv-cpp
   Memo:changed something by robin 2021-07-02
*/

#include "../include/SocketAddr.h"

#pragma warning(disable:4996)

namespace robin
{
	SocketAddr::SocketAddr(const std::string&& ip, unsigned short port, IPV ipv)
		:ip_(ip),
		port_(port),
		ipv_(ipv)
	{
		if (ipv == Ipv6)
		{
			::uv_ip6_addr(ip.c_str(), port, &ipv6_);
		}
		else
		{
			::uv_ip4_addr(ip.c_str(), port, &ipv4_);
		}
	}

	SocketAddr::SocketAddr(const std::string& ip, unsigned short port, IPV ipv)
		:SocketAddr(std::move(ip), port, ipv)
	{

	}

	SocketAddr::SocketAddr(const sockaddr* addr, IPV ipv)
		: ipv_(ipv)
	{
		if (ipv_ == Ipv4)
		{
			ipv4_ = *(reinterpret_cast<const sockaddr_in*>(addr));
		}
		else
		{
			ipv6_ = *(reinterpret_cast<const sockaddr_in6*>(addr));
		}
		port_ = GetIpAndPort((const sockaddr_storage *)(addr), ip_, ipv);
	}

	const sockaddr * SocketAddr::Addr()
	{
		return (ipv_ == Ipv6) ? reinterpret_cast<const sockaddr*>(&ipv6_) : reinterpret_cast<const sockaddr*>(&ipv4_);
	}

	void SocketAddr::toStr(std::string & str)
	{
		str = ip_ + ":" + std::to_string(port_);
	}

	std::string SocketAddr::toStr()
	{
		std::string str = ip_ + ":" + std::to_string(port_);
		return str;
	}

	SocketAddr::IPV SocketAddr::Ipv()
	{
		return ipv_;
	}

	void SocketAddr::AddrToStr(uv_tcp_t* client, std::string& addrStr, IPV ipv)
	{
		struct sockaddr_storage addr;
		int len = sizeof(struct sockaddr_storage);
		::uv_tcp_getpeername(client, (struct sockaddr *)&addr, &len);

		uint16_t port = GetIpAndPort(&addr, addrStr, ipv);
		addrStr += ":" + std::to_string(port);
	}

	uint16_t SocketAddr::GetIpAndPort(const sockaddr_storage* addr, std::string& out, IPV ipv)
	{
		auto inet = (Ipv6 == ipv) ? AF_INET6 : AF_INET;
		if (Ipv6 == ipv)
		{
			char ip[64];
			struct sockaddr_in6* addr6 = (struct sockaddr_in6 *)addr;
			//some version windows may not find  inet_ntop()
#if    _MSC_VER
			DWORD size = sizeof(ip);
			WSAAddressToString((LPSOCKADDR)addr6, sizeof(sockaddr_in6), NULL, ip, &size);
			out = std::string(ip);
			auto index = out.rfind(":");
			if (index >= 0)
			{
				out.resize(index);
			}
			return (htons(addr6->sin6_port));
#else
			std::string str(inet_ntop(inet, (void *)&(addr6->sin6_addr), ip, 64));
			out.swap(str);
			return(htons(addr6->sin6_port));
#endif
		}
		else
		{
			struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
			std::string str(inet_ntoa(addr4->sin_addr));
			out.swap(str);
			return htons(addr4->sin_port);
		}
	}
}
