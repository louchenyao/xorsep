#pragma once

static inline void prefetch0(const volatile void *p) {
    asm volatile("prefetcht0 %[p]" : : [p] "m"(*(const volatile char *)p));
}

static inline void prefetch1(const volatile void *p) {
    asm volatile("prefetcht1 %[p]" : : [p] "m"(*(const volatile char *)p));
}

static inline void prefetch2(const volatile void *p) {
    asm volatile("prefetcht2 %[p]" : : [p] "m"(*(const volatile char *)p));
}

static inline void prefetch_non_temporal(const volatile void *p) {
    asm volatile("prefetchnta %[p]" : : [p] "m"(*(const volatile char *)p));
}