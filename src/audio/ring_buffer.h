// fwMSX -- buffer circular de amostras entre a thread de emulacao (produtora)
// e o callback de audio (consumidora): 1 produtor, 1 consumidor, sem trava.
// Codigo ORIGINAL do fwMSX (BSD-3-Clause). Ver doc/audio-spec.md.
#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace audio {

class SampleRing {
public:
    // `capacity` e' arredondada para a proxima potencia de 2 (minimo 2).
    explicit SampleRing(size_t capacity) {
        size_t c = 2;
        while (c < capacity) c <<= 1;
        buf_.assign(c, 0);
        mask_ = c - 1;
    }

    size_t capacity() const { return buf_.size(); }

    // Amostras disponiveis para ler agora.
    size_t size() const { return head_.load(std::memory_order_acquire) - tail_.load(std::memory_order_acquire); }

    // Chamada SO' pelo produtor. Escreve ate' `n` amostras e devolve quantas
    // coube -- o excedente e' descartado (o produtor nao bloqueia nunca).
    size_t Push(const int16_t *data, size_t n) {
        const size_t head = head_.load(std::memory_order_relaxed);
        const size_t tail = tail_.load(std::memory_order_acquire);
        const size_t free_slots = buf_.size() - (head - tail);
        const size_t count = n < free_slots ? n : free_slots;
        for (size_t i = 0; i < count; ++i) buf_[(head + i) & mask_] = data[i];
        head_.store(head + count, std::memory_order_release);
        return count;
    }

    // Chamada SO' pelo consumidor. Le ate' `n` amostras e devolve quantas leu.
    size_t Pop(int16_t *out, size_t n) {
        const size_t tail = tail_.load(std::memory_order_relaxed);
        const size_t head = head_.load(std::memory_order_acquire);
        const size_t avail = head - tail;
        const size_t count = n < avail ? n : avail;
        for (size_t i = 0; i < count; ++i) out[i] = buf_[(tail + i) & mask_];
        tail_.store(tail + count, std::memory_order_release);
        return count;
    }

private:
    std::vector<int16_t> buf_;
    size_t mask_ = 0;
    std::atomic<size_t> head_{0}; // proxima posicao a escrever (so' o produtor mexe)
    std::atomic<size_t> tail_{0}; // proxima posicao a ler (so' o consumidor mexe)
};

} // namespace audio
