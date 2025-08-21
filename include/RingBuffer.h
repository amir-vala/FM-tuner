#pragma once
#include <vector>
#include <mutex>
#include <cstdint>

class RingBuffer {
public:
    explicit RingBuffer(size_t capacity);
    size_t write(const uint8_t* data, size_t bytes);
    size_t read(uint8_t* dst, size_t bytes);
    size_t size() const;
    size_t capacity() const { return buf_.size(); }
private:
    std::vector<uint8_t> buf_;
    size_t head_{0}, tail_{0};
    bool full_{false};
    mutable std::mutex m_;
};
