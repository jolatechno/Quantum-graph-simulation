#include <sys/sysinfo.h>

unsigned long int get_free_mem_size() {
	struct sysinfo info;
  
	if (sysinfo(&info) < 0)
		return 0;
	
	return info.totalram * info.mem_unit;
}