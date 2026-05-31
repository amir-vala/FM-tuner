#include "DiscoveryServer.h"
#include <iostream>
#include <thread>
#include <vector>

DiscoveryServer::DiscoveryServer(uint32_t deviceId, int httpPort)
    : deviceId_(deviceId), httpPort_(httpPort) {}

DiscoveryServer::~DiscoveryServer() {
    stop();
}

bool DiscoveryServer::start() {
    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_ == INVALID_SOCKET) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(HDHOMERUN_DISCOVER_UDP_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock_, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(sock_);
        return false;
    }

    running_ = true;
    thread_ = std::thread(&DiscoveryServer::run, this);
    return true;
}

void DiscoveryServer::stop() {
    running_ = false;
    if (sock_ != INVALID_SOCKET) {
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
    }
    if (thread_.joinable()) thread_.join();
}

void DiscoveryServer::run() {
    uint8_t buffer[1500];
    while (running_) {
        sockaddr_in clientAddr{};
        int clientAddrLen = sizeof(clientAddr);
        int received = recvfrom(sock_, (char*)buffer, sizeof(buffer), 0, (sockaddr*)&clientAddr, &clientAddrLen);

        if (received < 4) continue;

        uint16_t type = (buffer[0] << 8) | buffer[1];
        if (type != HDHOMERUN_TYPE_DISCOVER_REQ) continue;

        // Simplified response construction
        std::vector<uint8_t> payload;

        // Tag Device Type
        payload.push_back(HDHOMERUN_TAG_DEVICE_TYPE);
        payload.push_back(4); // length
        payload.push_back(0); payload.push_back(0); payload.push_back(0); payload.push_back(1); // Tuner

        // Tag Device ID
        payload.push_back(HDHOMERUN_TAG_DEVICE_ID);
        payload.push_back(4);
        payload.push_back((deviceId_ >> 24) & 0xFF);
        payload.push_back((deviceId_ >> 16) & 0xFF);
        payload.push_back((deviceId_ >> 8) & 0xFF);
        payload.push_back(deviceId_ & 0xFF);

        // Tag Base URL
        std::string baseUrl = "http://";
        // Simple logic for IP (this should ideally be the actual local IP, but 127.0.0.1 for local test)
        // For production, we should detect the interface IP.
        baseUrl += "127.0.0.1:" + std::to_string(httpPort_);
        payload.push_back(HDHOMERUN_TAG_BASE_URL);
        payload.push_back((uint8_t)baseUrl.length());
        for(char c : baseUrl) payload.push_back(c);

        std::vector<uint8_t> packet;
        packet.push_back(0x00); packet.push_back(0x03); // Type: RPY
        packet.push_back((payload.size() >> 8) & 0xFF);
        packet.push_back(payload.size() & 0xFF);
        packet.insert(packet.end(), payload.begin(), payload.end());

        uint32_t crc = calculateCrc(packet.data(), packet.size());
        packet.push_back(crc & 0xFF);
        packet.push_back((crc >> 8) & 0xFF);
        packet.push_back((crc >> 16) & 0xFF);
        packet.push_back((crc >> 24) & 0xFF);

        sendto(sock_, (char*)packet.data(), (int)packet.size(), 0, (sockaddr*)&clientAddr, clientAddrLen);
    }
}

uint32_t DiscoveryServer::calculateCrc(const uint8_t* data, size_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320;
            else crc >>= 1;
        }
    }
    return ~crc;
}
