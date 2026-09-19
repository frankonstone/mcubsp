#pragma once

#include <cstddef>
#include <cstdint>

#include <mcuhw/arch/barrier.hpp>

namespace mcuhw::testing::rtt {

struct up_buffer {
    const char* name;
    char* buffer;
    unsigned size;
    volatile unsigned write_offset;
    volatile unsigned read_offset;
    unsigned flags;
};

struct down_buffer {
    const char* name;
    char* buffer;
    unsigned size;
    volatile unsigned write_offset;
    volatile unsigned read_offset;
    unsigned flags;
};

// Wire-compatible subset of the public SEGGER RTT control-block ABI. One
// non-blocking up channel is sufficient for deterministic mcutest reporting.
struct control_block {
    char id[16];
    int max_up_buffers;
    int max_down_buffers;
    up_buffer up[1];
    down_buffer down[1];
};

inline char terminal_buffer[4096]{};
inline char terminal_down_buffer[16]{};
inline control_block terminal_control{};

inline void initialize() noexcept {
    constexpr char identifier[] = "SEGGER RTT";
    for (std::size_t index = 0; index < sizeof(terminal_control.id); ++index) {
        terminal_control.id[index] = index < sizeof(identifier)
            ? identifier[index]
            : '\0';
    }
    terminal_control.max_up_buffers = 1;
    terminal_control.max_down_buffers = 1;
    terminal_control.up[0].name = "Terminal";
    terminal_control.up[0].buffer = terminal_buffer;
    terminal_control.up[0].size = sizeof(terminal_buffer);
    terminal_control.up[0].write_offset = 0U;
    terminal_control.up[0].read_offset = 0U;
    terminal_control.up[0].flags = 0U;
    terminal_control.down[0].name = "Terminal";
    terminal_control.down[0].buffer = terminal_down_buffer;
    terminal_control.down[0].size = sizeof(terminal_down_buffer);
    terminal_control.down[0].write_offset = 0U;
    terminal_control.down[0].read_offset = 0U;
    terminal_control.down[0].flags = 0U;
    arch::data_memory_barrier();
}

inline void write(const char* data, std::size_t length) noexcept {
    auto& channel = terminal_control.up[0];
    if (channel.size < 2U) {
        return;
    }
    auto write_offset = channel.write_offset;
    const auto read_offset = channel.read_offset;
    for (std::size_t index = 0; index < length; ++index) {
        const auto next = (write_offset + 1U) % channel.size;
        if (next == read_offset) {
            break;
        }
        channel.buffer[write_offset] = data[index];
        write_offset = next;
    }
    arch::data_memory_barrier();
    channel.write_offset = write_offset;
}

// Evidence payloads can exceed the RTT ring. Unlike ordinary diagnostics,
// these bytes must not be dropped: wait for the host logger to make room.
inline void write_blocking(const char* data, std::size_t length) noexcept {
    auto& channel = terminal_control.up[0];
    if (channel.size < 2U) {
        return;
    }
    auto write_offset = channel.write_offset;
    for (std::size_t index = 0; index < length; ++index) {
        auto next = (write_offset + 1U) % channel.size;
        while (next == channel.read_offset) {
            __asm__ volatile("nop");
        }
        channel.buffer[write_offset] = data[index];
        write_offset = next;
        arch::data_memory_barrier();
        channel.write_offset = write_offset;
    }
}

} // namespace mcuhw::testing::rtt
