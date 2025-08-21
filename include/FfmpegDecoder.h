#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <cstdint>
#include "RingBuffer.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

struct DecodedFormat {
    int sampleRate = 48000;
    int channels = 2;
    int bitsPerSample = 16; // S16
};

class FfmpegDecoder {
public:
    FfmpegDecoder();
    ~FfmpegDecoder();
    bool open(const std::wstring& url, const DecodedFormat& outFmt);
    void close();
    size_t readPcm(uint8_t* dst, size_t bytes); // pulls from ring buffer
    bool isRunning() const { return running_; }
private:
    void threadMain(std::string urlUtf8, DecodedFormat outFmt);

    std::thread th_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stop_{false};

    AVFormatContext* fmt_ = nullptr;
    AVCodecContext*  dec_ = nullptr;
    SwrContext*      swr_ = nullptr;
    int audioStreamIndex_ = -1;

    RingBuffer ring_{ 48000 * 2 * 2 }; // ~1 second @ 48k S16 stereo
};
