#include "arena.hpp"
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <cstring>
int main() {
    perf_event_attr pe; std::memset(&pe, 0, sizeof(pe));
    pe.type = PERF_TYPE_HARDWARE; pe.config = PERF_COUNT_HW_CPU_CYCLES;
    pe.size = sizeof(pe); pe.disabled = 1; pe.exclude_kernel = 1; pe.exclude_hv = 1;
    int fd = syscall(__NR_perf_event_open, &pe, 0, -1, -1, 0);
    if (fd == -1) { std::cout << "Perf not allowed. Run with sudo or sysctl -w kernel.perf_event_paranoid=0" << std::endl; return 1; }
    petronilho::ScalableArena arena(1024ULL * 1024 * 1024);
    ioctl(fd, PERF_EVENT_IOC_RESET, 0); ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
    for (int i = 0; i < 50000000; ++i) { void* p = arena.allocate(64); asm volatile("" : : "g"(p) : "memory"); }
    ioctl(fd, PERF_EVENT_IOC_DISABLE, 0); uint64_t count; read(fd, &count, sizeof(count));
    std::cout << "Cycles measured: " << count << std::endl; close(fd);
    return 0;
}
