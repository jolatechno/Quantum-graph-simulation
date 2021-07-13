#include <sys/sysinfo.h>
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int parseLine(char* line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

std::pair<float, float> get_mem_usage_and_free_mem() {
	struct sysinfo info;
  
	if (sysinfo(&info) < 0)
		throw;

	/* read memory usage of this program 
	kind of shitty !! */	
	FILE* file = fopen("/proc/self/status", "r");
    int ram_usage = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmRSS:", 6) == 0){
            ram_usage = parseLine(line);
            break;
        }
    }
    fclose(file);

    return {1000 * (float)ram_usage / (float)info.totalram,
    	(float)info.freeram / (float)info.totalram};
}