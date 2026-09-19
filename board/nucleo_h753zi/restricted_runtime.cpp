#include <cstddef>

namespace {

[[noreturn]] void runtime_trap() noexcept {
    __asm__ volatile("bkpt #0");
    for (;;) {
        __asm__ volatile("wfi");
    }
}

} // namespace

namespace std {
[[noreturn]] void __glibcxx_assert_fail(const char*, int, const char*,
                                        const char*) noexcept {
    runtime_trap();
}
} // namespace std

void operator delete(void*) noexcept {
    runtime_trap();
}

void operator delete(void*, std::size_t) noexcept {
    runtime_trap();
}

extern "C" [[noreturn]] void __cxa_pure_virtual() noexcept {
    runtime_trap();
}
