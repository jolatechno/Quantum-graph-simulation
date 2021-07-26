#include <sys/sysinfo.h>

std::pair<long long int, long int> get_mem_usage_and_free_mem() {
	struct sysinfo info;
	if (sysinfo(&info) < 0)
		throw;

    return {
        info.totalram * info.mem_unit,
        info.freeram * info.mem_unit
    };
}