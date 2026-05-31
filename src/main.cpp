#include <httplib/httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <process.h>
#include <windows.h>
#include "DiscoveryServer.h"

using json = nlohmann::json;

class VirtualTuner {
public:
    VirtualTuner(uint32_t deviceId, int port) : deviceId_(deviceId), port_(port), discovery_(deviceId, port) {}

    void run() {
        httplib::Server svr;

        svr.Get("/discover.json", [this](const httplib::Request&, httplib::Response& res) {
            json j;
            j["FriendlyName"] = "Virtual Radio Tuner";
            j["ModelNumber"] = "HDHR4-2US";
            j["FirmwareName"] = "hdhomerun4_atsc";
            j["TunerCount"] = 1;
            j["FirmwareVersion"] = "20230713";
            char idStr[9];
            snprintf(idStr, sizeof(idStr), "%08X", deviceId_);
            j["DeviceID"] = idStr;
            // Note: In real scenarios, use actual LAN IP instead of 127.0.0.1
            j["BaseURL"] = "http://127.0.0.1:" + std::to_string(port_);
            j["LineupURL"] = "http://127.0.0.1:" + std::to_string(port_) + "/lineup.json";
            res.set_content(j.dump(), "application/json");
        });

        svr.Get("/lineup_status.json", [](const httplib::Request&, httplib::Response& res) {
            json j;
            j["ScanInProgress"] = 0;
            j["ScanPossible"] = 1;
            j["Source"] = "Antenna";
            j["TunerCount"] = 1;
            res.set_content(j.dump(), "application/json");
        });

        svr.Get("/lineup.json", [this](const httplib::Request&, httplib::Response& res) {
            std::ifstream f("stations.json");
            if (!f) {
                res.status = 404;
                return;
            }
            json stations;
            try { f >> stations; } catch(...) { res.status = 500; return; }

            json lineup = json::array();
            for (auto& s : stations) {
                json item;
                item["GuideNumber"] = s["guideNumber"];
                item["GuideName"] = s["guideName"];
                item["URL"] = "http://127.0.0.1:" + std::to_string(port_) + "/auto/v" + s["guideNumber"].get<std::string>();
                lineup.push_back(item);
            }
            res.set_content(lineup.dump(), "application/json");
        });

        svr.Get("/auto/v(.*)", [this](const httplib::Request& req, httplib::Response& res) {
            std::string channel = req.matches[1];
            std::cout << "Request for channel: " << channel << std::endl;

            std::string streamUrl;
            try {
                std::ifstream f("stations.json");
                if (!f) throw std::runtime_error("File not found");
                json stations;
                f >> stations;
                for (auto& s : stations) {
                    if (s["guideNumber"] == channel) {
                        streamUrl = s["url"];
                        break;
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Error reading stations.json: " << e.what() << std::endl;
                res.status = 500;
                return;
            }

            if (streamUrl.empty()) {
                res.status = 404;
                return;
            }

            res.set_header("Content-Type", "video/mp2t");

            res.set_content_provider(
                "video/mp2t",
                [streamUrl](size_t offset, httplib::DataSink& sink) {
                    // Start FFmpeg
                    // We add a dummy video stream for better WMC compatibility
                    std::string cmd = "ffmpeg.exe -re -i \"" + streamUrl + "\" -f lavfi -i color=c=black:s=320x240 -c:v libx264 -preset ultrafast -tune zerolatency -c:a mp2 -b:a 192k -f mpegts -";

                    SECURITY_ATTRIBUTES saAttr;
                    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
                    saAttr.bInheritHandle = TRUE;
                    saAttr.lpSecurityDescriptor = NULL;

                    HANDLE hRead, hWrite;
                    if (!CreatePipe(&hRead, &hWrite, &saAttr, 0)) return false;
                    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

                    STARTUPINFOA siStartInfo;
                    PROCESS_INFORMATION piProcInfo;
                    ZeroMemory(&siStartInfo, sizeof(STARTUPINFOA));
                    siStartInfo.cb = sizeof(STARTUPINFOA);
                    siStartInfo.hStdOutput = hWrite;
                    siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
                    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

                    if (!CreateProcessA(NULL, (char*)cmd.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo)) {
                        CloseHandle(hRead);
                        CloseHandle(hWrite);
                        return false;
                    }

                    CloseHandle(hWrite);

                    char buffer[4096];
                    DWORD dwRead;
                    while (ReadFile(hRead, buffer, sizeof(buffer), &dwRead, NULL) && dwRead > 0) {
                        if (!sink.is_writable()) break;
                        sink.write(buffer, dwRead);
                    }

                    TerminateProcess(piProcInfo.hProcess, 0);
                    CloseHandle(piProcInfo.hProcess);
                    CloseHandle(piProcInfo.hThread);
                    CloseHandle(hRead);
                    sink.done();
                    return true;
                }
            );
        });

        discovery_.start();
        std::cout << "Server started at http://0.0.0.0:" << port_ << std::endl;
        svr.listen("0.0.0.0", port_);
    }

private:
    uint32_t deviceId_;
    int port_;
    DiscoveryServer discovery_;
};

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    VirtualTuner tuner(0x12345678, 8080);
    tuner.run();

    WSACleanup();
    return 0;
}
