#pragma once

#if defined(_MSC_VER)

#include <windows.h>
typedef LONG volatile imdd_atomic_uint;
#define imdd_atomic_store(addr, val)		InterlockedExchange(addr, val)
#define imdd_atomic_load(addr)				InterlockedCompareExchange((imdd_atomic_uint *)addr, 0, 0)
#define imdd_atomic_fetch_add(addr, val)	(InterlockedAdd(addr, val) - val)

#elif defined(__clang__)

#include <stdatomic.h>
typedef atomic_uint imdd_atomic_uint;
#define imdd_atomic_store(addr, val)        atomic_store_explicit(addr, val, memory_order_release)
#define imdd_atomic_load(addr)              atomic_load_explicit(addr, memory_order_acquire)
#define imdd_atomic_fetch_add(addr, val)    atomic_fetch_add_explicit(addr, val, memory_order_relaxed)

#else

typedef uint32_t imdd_atomic_uint;
#define imdd_atomic_store(addr, val)        __atomic_store_n(addr, val, __ATOMIC_RELEASE)
#define imdd_atomic_load(addr)              __atomic_load_n(addr, __ATOMIC_ACQUIRE)
#define imdd_atomic_fetch_add(addr, val)    __atomic_fetch_add(addr, val, __ATOMIC_RELAXED)

#endif
