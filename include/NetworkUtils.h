#pragma once
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <vector>

#pragma comment(lib, "iphlpapi.lib")

inline std::string GetLocalIPAddress() {
    std::string ip = "127.0.0.1";
    ULONG outBufLen = 15000;
    std::vector<char> buffer(outBufLen);
    PIP_ADAPTER_ADDRESSES pAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());

    if (GetAdaptersAddresses(AF_INET, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &outBufLen) == NO_ERROR) {
        while (pAddresses) {
            if (pAddresses->OperStatus == IfOperStatusUp && pAddresses->IfType != IF_TYPE_SOFTWARE_LOOPBACK) {
                PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pAddresses->FirstUnicastAddress;
                while (pUnicast) {
                    sockaddr_in* sa_in = (sockaddr_in*)pUnicast->Address.lpSockaddr;
                    char str[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &(sa_in->sin_addr), str, INET_ADDRSTRLEN);
                    std::string currentIp = str;
                    // Prefer 192.168.x.x or 10.x.x.x or 172.16.x.x
                    if (currentIp.find("192.168.") == 0 || currentIp.find("10.") == 0 || currentIp.find("172.") == 0) {
                        return currentIp;
                    }
                    ip = currentIp;
                    pUnicast = pUnicast->Next;
                }
            }
            pAddresses = pAddresses->Next;
        }
    }
    return ip;
}
