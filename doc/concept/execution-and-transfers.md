# Bare-metal execution and transfer contract

This page defines the detailed contract. For a user-facing explanation and a
complete `main()`/execution-policy/interrupt example, start with
[Integrating mcuhw into a bare-metal application](application-integration.md).

PLN-0006 establishes the execution boundary used by later functional drivers.
It does not own a scheduler, system tick, vector table, or global event loop, and
it does not include MCUOS headers.

## Ownership and storage

Peripheral ownership is represented by a fixed-size ownership table. A claim
returns a move-only lease; duplicate claims return `busy`, and no implicit
reconfiguration or queue is created.

Each transfer operation occupies a stable slot in a fixed-capacity pool. A
move-only `transfer<T>` refers to that slot by index and generation, so moving a
handle never relocates storage observed by an interrupt handler. Exhaustion is
reported explicitly. The pool and execution policy must outlive every handle.
`operation_pool::capacity` reports the compile-time slot count, and
`sizeof(operation_pool<...>)` is the complete static storage cost for a chosen
value/error/policy binding; there is no heap allocation or hidden queue.
Stored value types must be nothrow movable/destructible, and typed error values
must be nothrow copyable, movable, and destructible so publication and
consumption cannot escape a critical section through an exception.

Dropping a terminal handle releases its slot. Destroying a pending handle is a
contract violation: the fake policy records it and a target bare-metal binding
must trap. Callers shall instead wait for completion or call `cancel()`. This is
the selected active-destruction policy for the first release.

## Completion, consumption, and quiescence

An operation has exactly one transition out of `pending`. Success stores the
value before publishing readiness; failure stores its typed cause before
publishing readiness. Timeout and cancellation first reserve the terminal
transition, then invoke the operation's bounded quiesce function outside the
critical section. Completion arriving after that reservation is rejected.

A quiesce function returns either `reusable` or `faulted`. Both results promise
that hardware, DMA, and interrupt handlers can no longer access operation
buffers. `faulted` retires the slot permanently; it does not permit unsafe
reuse. A quiesce implementation unable to make that promise must trap rather
than return. Generation-tagged operation IDs reject duplicate and late
completion, including completion for a later operation that reused a slot.

Inspection is repeatable and non-consuming. `take()` consumes a success value
or terminal error exactly once. Pending and consumed states are explicit and
never masquerade as a successful result.

## Execution-policy boundary

The transfer implementation requires an execution policy providing:

- a wrapping 32-bit monotonic tick count;
- short critical sections protecting pool and notification state;
- ISR-context detection;
- wait preparation, one wait/poll step, and completion notification; and
- an active-transfer-destruction violation hook.

Deadlines use half-range modular ordering. Relative deadlines must not exceed
`2^31 - 1` ticks. At the exact deadline, a completion already published wins;
otherwise timeout reserves the terminal transition.

Waiting follows `prepare_wait -> check readiness/deadline -> wait_once`. A
notification token closes the completion-before-sleep window. No wait holds a
critical section, and waits from ISR context return an explicit error.

The initial bare-metal policy is single-core. Its `wait_once` operation polls
with interrupts enabled; it deliberately does not issue WFI because a portable
race-free sleep handshake requires target composition. Time, interrupt masking,
ISR detection, polling, and notification are supplied by target hooks, so the
core does not take over SysTick, PendSV, vectors, or another environment-owned
facility. SMP use requires a future policy with validated inter-core
synchronization.

The native fake policy exposes deterministic wait stages and fake time for
race, timeout, wrap, cancellation, and late-completion tests. A future RTOS
adapter can implement the same policy surface while keeping scheduler types and
primitives outside drivers; callback and coroutine adapters are not required by
this release.
