#include <benchmark/benchmark.h>

#ifdef __linux__
#include "PerfEvent.hpp"
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#endif

class PerfEventBenchamrkWrapper {
public:
    PerfEventBenchamrkWrapper(benchmark::State& state) {
#ifdef __linux__
        test_if_support_perf_event();

        if (!perf_event_supported)  return;
        state_ = &state;
        e_ = new PerfEvent();
        e_->startCounters();
#else
        (void)state; // unused
#endif
    }

    ~PerfEventBenchamrkWrapper() {
#ifdef __linux__
        if (!perf_event_supported) return;

        auto &e = *e_;
        e.stopCounters();
        auto &state = *state_;
        int n = state.iterations();
        state.counters["cycles"] = e.getCounter("cycles") / n;
        state.counters["instructions"] = e.getCounter("instructions") / n;
        state.counters["L1-misses"] = e.getCounter("L1-misses") / n;
        state.counters["LLC-misses"] = e.getCounter("LLC-misses") / n;
        state.counters["branch-misses"] = e.getCounter("branch-misses") / n;
        state.counters["task-clock"] = e.getCounter("task-clock") / n;
        state.counters["IPC"] = e.getCounter("instructions") / e.getCounter("cycles");
        state.counters["CPUs"] = e.getCounter("task-clock") / (e.getDuration()*1e9);
        state.counters["GHz"] = e.getCounter("cycles") / e.getCounter("task-clock");

        delete e_;
#endif
    }

    void test_if_support_perf_event() {
#ifdef __linux__
        perf_event_attr pe;
        int fd;
        uint64_t value;

        // setup pe
        memset(&pe, 0, sizeof(struct perf_event_attr));
        pe.type = PERF_TYPE_HARDWARE;
        pe.size = sizeof(struct perf_event_attr);
        pe.config = PERF_COUNT_HW_CPU_CYCLES;
        pe.disabled = true;
        pe.inherit = 1;
        pe.inherit_stat = 0;
        pe.exclude_kernel = false;
        pe.exclude_hv = false;
        //pe.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;

        // open
        fd = syscall(__NR_perf_event_open, &pe, 0, -1, -1, 0);

        // read
        perf_event_supported = read(fd, &value, sizeof(uint64_t)) == sizeof(uint64_t);

        // close
        close(fd);
#endif
    }

    bool perf_event_supported = false;    

#ifdef __linux__
private:
    PerfEvent *e_;
    benchmark::State *state_;
#endif
};