#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#define HDHOMERUN_DISCOVER_UDP_PORT 65001
#define HDHOMERUN_TYPE_DISCOVER_REQ 0x0002
#define HDHOMERUN_TYPE_DISCOVER_RPY 0x0003
#define HDHOMERUN_TAG_DEVICE_TYPE 0x01
#define HDHOMERUN_TAG_DEVICE_ID 0x02
#define HDHOMERUN_TAG_BASE_URL 0x2A
#define HDHOMERUN_DEVICE_TYPE_TUNER 0x00000001

class DiscoveryServer {
public:
    DiscoveryServer(uint32_t deviceId, int httpPort, const std::string& localIp);
    ~DiscoveryServer();
    bool start();
    void stop();

private:
    void run();
    uint32_t calculateCrc(const uint8_t* data, size_t len);

    uint32_t deviceId_;
    int httpPort_;
    std::string localIp_;
    SOCKET sock_ = INVALID_SOCKET;
    bool running_ = false;
    std::thread thread_;
};
