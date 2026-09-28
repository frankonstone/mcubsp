# mcuhw — Chat Handoff Document

## Project goal

Build a modern C++23 hardware abstraction/library for microcontrollers, with:

- compile-time hardware descriptions
- heavy use of `constexpr` / `consteval`
- type-safe MMIO registers and bitfields
- MCU-specific hardware metadata separated from reusable drivers
- no preprocessor-heavy HAL design
- compile-time validation of pin routing, clocks, DMA, timer channels, etc.
- very small runtime overhead
- optional async operation without forcing the whole application into `async/await`

The initial concrete target is:

- **Arduino UNO R4 Minima**
- MCU: **Renesas RA4M1**

Other MCUs discussed as future targets/reference points:

- STM32H753
- ESP32-P4

---

## High-level architecture

The preferred architecture is layered:

```text
MCU description        Peripheral driver        Application / Board
─────────────────      ──────────────────       ───────────────────
constexpr metadata  -> generic driver       -> concrete pins/instances
registers
clocks
IRQs
DMA
pins / routing
capabilities
```

Important principle:

> The common abstraction should live at the functional level, not at the register-layout level.

Different MCUs should not be forced into the same fake register model.

---

## Core design principles

### 1. Register model

Avoid overlaying vendor C structs directly on peripheral memory.

Preferred style:

```cpp
namespace hw {

enum class access {
    read_only,
    write_only,
    read_write,
    write_one_to_clear
};

template<
    std::uintptr_t Address,
    typename T = std::uint32_t,
    access Access = access::read_write
>
struct reg {
    using value_type = T;

    static constexpr auto address = Address;
    static constexpr auto access_mode = Access;

    [[nodiscard]]
    static inline T read() noexcept
        requires (Access != access::write_only)
    {
        return *reinterpret_cast<volatile T*>(Address);
    }

    static inline void write(T value) noexcept
        requires (Access != access::read_only)
    {
        *reinterpret_cast<volatile T*>(Address) = value;
    }
};

template<unsigned Pos, unsigned Width = 1>
struct field {
    static constexpr unsigned position = Pos;
    static constexpr unsigned width = Width;

    static constexpr std::uint32_t mask =
        ((std::uint32_t{1} << Width) - 1) << Pos;

    static consteval std::uint32_t encode(std::uint32_t value)
    {
        return (value << Pos) & mask;
    }
};

}
```

---

### 2. Peripheral descriptions

Peripheral instances should be compile-time metadata types.

Example shape:

```cpp
enum class peripheral_kind {
    gpio,
    uart,
    spi,
    i2c,
    timer,
    adc,
    dma,
    usb,
    ethernet
};

template<
    peripheral_kind Kind,
    std::uintptr_t Base,
    auto Irq,
    auto Clock
>
struct peripheral {
    static constexpr auto kind = Kind;
    static constexpr auto base = Base;
    static constexpr auto irq = Irq;
    static constexpr auto clock = Clock;
};
```

The concrete MCU description contains addresses, IRQs, clock relations and capabilities.

The driver should not contain MCU addresses.

---

### 3. Capabilities

Prefer concept/capabilities over a "largest common denominator" HAL.

Examples:

```cpp
supports_dma<T>
supports_fifo<T>
supports_hw_flow_control<T>
supports_clock_stretching<T>
supports_dead_time<T>
supports_complementary_outputs<T>
supports_center_aligned_pwm<T>
```

Then:

```cpp
if constexpr (supports_dma<Hw>) {
    ...
}
```

---

## Clock model

Clock handling should be an independent subsystem driven by compile-time traits.

Do not make drivers hard-code clock register behavior.

Example:

```cpp
template<typename Peripheral>
struct clock_traits;
```

For RA4M1 specifically, the model must support Renesas' module-stop system and peripheral clocks.

Conceptually:

```cpp
clock::enable<Peripheral>();
clock::disable<Peripheral>();
reset::release<Peripheral>();
```

The MCU description determines how that maps to RA4M1 registers.

---

## Pin model

Pins should be types.

Example:

```cpp
template<unsigned Port, unsigned Pin>
struct pin {
    static constexpr auto port = Port;
    static constexpr auto number = Pin;
};
```

Routing should be compile-time checked.

Example:

```cpp
template<typename Peripheral, auto Signal, typename Pin>
concept routable =
    requires {
        pin_route<Peripheral, Signal, Pin>::value;
    };
```

Invalid peripheral/pin combinations should fail at compile time.

For RA4M1, this needs to model PFS / pin-function selection.

---

# PWM design

The desired application-facing API is compact.

Simple PWM:

```cpp
using pwm = mcuhw::pwm<
    mcuhw::mcu::ra4m1::gpt0,
    mcuhw::channel<1>,
    mcuhw::pin::p105
>;

constexpr pwm::config cfg{
    .frequency = 20_kHz,
    .duty_cycle = 0.25
};

int main()
{
    pwm::init<cfg>();
    pwm::start();
}
```

For grouped/motor PWM:

```cpp
using phase_u = mcuhw::pwm<
    mcu::ra4m1::gpt0,
    channel<1>,
    pin::...
>;

using phase_v = mcuhw::pwm<
    mcu::ra4m1::gpt0,
    channel<2>,
    pin::...
>;

using phase_w = mcuhw::pwm<
    mcu::ra4m1::gpt0,
    channel<3>,
    pin::...
>;
```

Potential higher-level grouping:

```cpp
using motor_pwm = mcuhw::pwm_group<
    phase_u,
    phase_v,
    phase_w
>;
```

Possible configuration:

```cpp
constexpr motor_pwm::config cfg{
    .frequency = 20_kHz,
    .alignment = pwm_alignment::center,
    .dead_time = 500_ns
};
```

Compile-time solving is desired:

```cpp
consteval auto solve_pwm(
    units::frequency timer_clock,
    units::frequency requested,
    double duty,
    unsigned counter_bits
) -> timer_solution;
```

That should calculate:

- prescaler
- period
- compare value
- actual frequency
- optional dead-time encoding

The solver should ideally disappear entirely from the final firmware binary.

The RA4M1 GPT is a good fit because it has PWM modes, buffering, synchronization, capture/compare, and related timer functionality.

---

# I2C design

The preferred layering is:

```text
I2C peripheral description
        ↓
I2C backend
        ↓
I2C bus
        ↓
i2c_device<Bus, Address>
        ↓
sensor / EEPROM / device driver
```

Example bus:

```cpp
using bus = mcuhw::i2c<
    mcu::ra4m1::iic0,
    pin::scl,
    pin::sda
>;

constexpr bus::config cfg{
    .frequency = 400_kHz
};

int main()
{
    bus::init<cfg>();
}
```

Concrete I2C device:

```cpp
using eeprom = mcuhw::i2c_device<
    bus,
    mcuhw::i2c_address<0x50>
>;
```

Register-oriented use:

```cpp
using sensor = mcuhw::i2c_device<
    bus,
    mcuhw::i2c_address<0x76>
>;

auto id = sensor::read_register<std::uint8_t>(0xD0);
```

Combined write + repeated-start + read:

```cpp
std::array<std::byte, 6> buffer{};

sensor::write_read(
    std::array{std::byte{0xF7}},
    buffer
);
```

Important design point:

Use real 7-bit I2C addresses, not already-shifted values.

Potential types:

```cpp
i2c_address7<0x50>
i2c_address10<0x2AA>
```

Compile-time validation:

```cpp
static_assert(Address::value < 0x80);
```

I2C timing configuration should also be compile-time solvable where possible:

```cpp
consteval auto solve_i2c_timing(
    units::frequency peripheral_clock,
    units::frequency bus_frequency,
    i2c_mode mode
) -> i2c_timing;
```

For RA4M1 this will need to map onto its IIC timing/bit-rate registers.

---

# Async design — important conclusion

Initially coroutines (`task<T>` + `co_await`) were considered.

Example:

```cpp
mcuhw::task<void> read_sensor()
{
    std::array<std::byte, 6> raw{};

    co_await sensor_dev::async_read_registers(
        0xF7,
        raw
    );

    auto data = decode_measurement(raw);
    process(data);
}
```

However, the downside was identified:

> Async tends to propagate through the whole call graph.

Once a function uses:

```cpp
co_await something();
```

its caller often also becomes a coroutine, and so on.

That is **not** desired as the fundamental architecture for `mcuhw`.

## Preferred solution: `transfer<T>`

The central async primitive should be a transfer object.

Example:

```cpp
auto transfer = sensor::begin_read();
```

The same transfer can be consumed in several ways.

### Blocking

```cpp
auto measurement = sensor::begin_read().wait();
```

### Polling

```cpp
auto transfer = sensor::begin_read();

while (!transfer.ready()) {
    do_other_work();
}

auto measurement = transfer.get();
```

### Coroutine adapter

Optional:

```cpp
auto measurement = co_await sensor::begin_read();
```

Therefore the architecture is:

```text
                    ┌── blocking / waiting
                    │
Hardware -> transfer
                    │
                    ├── polling / event / callback
                    │
                    └── coroutine adapter (optional)
```

Coroutines are syntactic sugar, not the foundation.

This is currently considered the preferred design direction.

---

## Proposed transfer API

Sketch:

```cpp
template<typename T>
class transfer {
public:
    [[nodiscard]]
    bool ready() const noexcept;

    [[nodiscard]]
    bool error() const noexcept;

    T get();

    T wait()
    {
        while (!ready()) {
            // possibly WFI
        }

        return get();
    }

    // optional:
    auto operator co_await();
};
```

For transfers without a return value:

```cpp
transfer<void>
```

For I2C:

```cpp
std::array<std::byte, 6> buffer{};

auto t = bus::begin_read(
    i2c_address<0x76>,
    buffer
);

while (!t.ready()) {
    control_loop();
}

if (t.error()) {
    handle_error();
} else {
    process(buffer);
}
```

---

# Device driver example

A higher-level sensor driver should not know anything about the MCU.

Example:

```cpp
template<typename BusDevice>
class sensor_driver {
public:
    static auto read()
    {
        std::array<std::byte, 6> raw{};

        BusDevice::read_registers(
            0xF7,
            raw
        );

        return decode(raw);
    }

    static auto begin_read()
    {
        // initiate async transfer and return transfer<measurement>
    }
};
```

Concrete binding:

```cpp
using bus = mcuhw::i2c<
    mcu::ra4m1::iic0,
    pin::...,
    pin::...
>;

using sensor_device =
    mcuhw::i2c_device<
        bus,
        i2c_address<0x76>
    >;

using sensor =
    drivers::sensor_driver<sensor_device>;
```

---

# Suggested project layout

```text
mcuhw/
│
├── include/mcuhw/
│   ├── core/
│   │   ├── register.hpp
│   │   ├── field.hpp
│   │   ├── concepts.hpp
│   │   ├── meta.hpp
│   │   ├── units.hpp
│   │   └── transfer.hpp
│   │
│   ├── arch/
│   │   ├── arm/
│   │   │   ├── cortex_m4/
│   │   │   └── cortex_m7/
│   │   └── riscv/
│   │
│   ├── mcu/
│   │   ├── ra4m1/
│   │   │   ├── device.hpp
│   │   │   ├── memory.hpp
│   │   │   ├── interrupts.hpp
│   │   │   ├── clocks.hpp
│   │   │   ├── pins.hpp
│   │   │   ├── dma.hpp
│   │   │   ├── iic.hpp
│   │   │   ├── sci.hpp
│   │   │   ├── spi.hpp
│   │   │   └── gpt.hpp
│   │   │
│   │   ├── stm32h753/
│   │   └── esp32p4/
│   │
│   └── drivers/
│       ├── gpio.hpp
│       ├── uart.hpp
│       ├── spi.hpp
│       ├── i2c.hpp
│       ├── pwm.hpp
│       ├── timer.hpp
│       ├── adc.hpp
│       └── dma.hpp
│
├── example/
├── test/
└── CMakeLists.txt
```

Important separation:

```text
mcu/*
```

contains hardware description only.

```text
drivers/*
```

contains generic driver logic and no hard-coded MCU addresses.

---

# Immediate target

Start with **Arduino UNO R4 Minima / Renesas RA4M1**.

Recommended first implementation sequence:

1. `core/register.hpp`
2. `core/field.hpp`
3. basic units (`Hz`, `kHz`, `MHz`, `ns`, `us`, etc.)
4. RA4M1 memory/register addressing
5. RA4M1 clock/module-stop description
6. pin/PFS model
7. GPIO
8. GPT/PWM
9. IIC/I2C
10. `transfer<T>` for interrupt/DMA-backed operations
11. optional coroutine adapter later

---

# Source documents available in the original chat

The original chat had these manuals attached:

1. **Renesas RA4M1 User's Manual: Hardware**
   - primary reference for the Arduino UNO R4 Minima target
   - includes:
     - clock generation
     - module stop
     - ICU
     - DMA / DTC
     - ELC
     - I/O ports / PFS
     - GPT
     - AGT
     - IIC
     - SPI
     - SCI
     - USB
     - etc.

2. **STM32H753xI datasheet**

3. **STM32F7/H7 Cortex-M7 Programming Manual PM0253**

4. **ESP32-P4 Technical Reference Manual**

The RA4M1 manual should be treated as the authoritative starting point for the next implementation phase.

---

# Current preferred application style

The application API should remain compact and declarative.

PWM:

```cpp
using pwm = mcuhw::pwm<
    mcu::ra4m1::gpt0,
    channel<1>,
    pin::...
>;

constexpr pwm::config cfg{
    .frequency = 20_kHz,
    .duty_cycle = 0.25
};

int main()
{
    pwm::init<cfg>();
    pwm::start();
}
```

I2C:

```cpp
using bus = mcuhw::i2c<
    mcu::ra4m1::iic0,
    pin::...,
    pin::...
>;

using sensor_dev = mcuhw::i2c_device<
    bus,
    i2c_address<0x76>
>;

int main()
{
    bus::init<{
        .frequency = 400_kHz
    }>();

    auto measurement = sensor_dev::read(...);
}
```

Async when needed:

```cpp
auto transfer = sensor::begin_read();

while (!transfer.ready()) {
    do_other_work();
}

auto measurement = transfer.get();
process(measurement);
```

Optional coroutine use should still remain possible later:

```cpp
auto measurement = co_await sensor::begin_read();
```

But this should never be mandatory for ordinary application code.

---

## Current design decision summary

The most important decisions so far are:

- Modern **C++23**
- compile-time descriptions wherever possible
- `constexpr` / `consteval` for clocks, pin routing, timing calculations and validation
- type-safe registers/bitfields
- no generic C-style HAL with huge peripheral structs
- no vendor-specific addresses in generic drivers
- use concepts and capabilities
- compile-time invalid-pin detection
- compile-time PWM/I2C timing solving
- device drivers independent of MCU
- RA4M1 first
- Arduino UNO R4 Minima first board
- **`transfer<T>` is the preferred async primitive**
- coroutines are an optional adapter only

