#include "RingBuffer.h"
#include <algorithm>
#include <cstring> // memcpy

RingBuffer::RingBuffer(size_t capacity): buf_(capacity) {}

size_t RingBuffer::size() const {
    std::lock_guard<std::mutex> lock(m_);
    if (full_) return buf_.size();
    if (head_ >= tail_) return head_ - tail_;
    return buf_.size() - (tail_ - head_);
}

size_t RingBuffer::write(const uint8_t* data, size_t bytes) {
    std::lock_guard<std::mutex> lock(m_);
    size_t wrote = 0; size_t cap = buf_.size();
    while (wrote < bytes) {
        if (full_ && head_ == tail_) break; // buffer full
        size_t space = full_ ? 0 : (head_ >= tail_ ? cap - head_ : tail_ - head_);
        if (!space) { full_ = true; break; }
        size_t chunk = std::min(space, bytes - wrote);
        std::memcpy(&buf_[head_], data + wrote, chunk);
        head_ = (head_ + chunk) % cap;
        wrote += chunk;
        if (head_ == tail_) full_ = true;
    }
    return wrote;
}

size_t RingBuffer::read(uint8_t* dst, size_t bytes) {
    std::lock_guard<std::mutex> lock(m_);
    size_t readb = 0; size_t cap = buf_.size();
    while (readb < bytes) {
        if (!full_ && head_ == tail_) break; // empty
        size_t avail = full_ ? (head_ >= tail_ ? cap : cap - tail_) : (head_ >= tail_ ? head_ - tail_ : cap - tail_);
        size_t chunk = std::min(avail, bytes - readb);
        std::memcpy(dst + readb, &buf_[tail_], chunk);
        tail_ = (tail_ + chunk) % cap;
        readb += chunk;
        full_ = false;
    }
    return readb;
}
