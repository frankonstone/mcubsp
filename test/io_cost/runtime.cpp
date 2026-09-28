#include <cstddef>

// The same minimal freestanding runtime is linked into every cost probe.
// There is deliberately no allocator, hosted diagnostics, or test harness.
extern "C" [[noreturn]] void mcuhw_io_cost_trap() noexcept {
    for (;;) __asm__ volatile("bkpt #0");
}

namespace std {
[[noreturn]] void __glibcxx_assert_fail(const char*, int, const char*,
                                      const char*) noexcept {
    mcuhw_io_cost_trap();
}
} // namespace std

extern "C" void* memset(void* destination, int value,
                        std::size_t count) noexcept {
    auto* bytes = static_cast<unsigned char*>(destination);
    for (std::size_t index = 0; index < count; ++index) {
        bytes[index] = static_cast<unsigned char>(value);
    }
    return destination;
}

extern "C" void* memcpy(void* destination, const void* source,
                        std::size_t count) noexcept {
    auto* output = static_cast<unsigned char*>(destination);
    const auto* input = static_cast<const unsigned char*>(source);
    for (std::size_t index = 0; index < count; ++index) {
        output[index] = input[index];
    }
    return destination;
}

extern "C" void __aeabi_memclr(void* destination, std::size_t count) noexcept {
    static_cast<void>(memset(destination, 0, count));
}
extern "C" void __aeabi_memclr4(void* destination, std::size_t count) noexcept {
    static_cast<void>(memset(destination, 0, count));
}
extern "C" void __aeabi_memcpy(void* destination, const void* source,
                              std::size_t count) noexcept {
    static_cast<void>(memcpy(destination, source, count));
}
extern "C" void __aeabi_memcpy4(void* destination, const void* source,
                               std::size_t count) noexcept {
    static_cast<void>(memcpy(destination, source, count));
}
