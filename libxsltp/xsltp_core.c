#include "xsltp_config.h"
#include "xsltp_core.h"

void *xsltp_malloc(size_t size)
{
    void *p = malloc(size);
    if (p == NULL)
		perror("malloc failed\n");
    return p;
}

void xsltp_free(void *p) {
	free(p);
}

time_t xsltp_last_modify(const char *file_name) {
	struct stat sb;
	if (stat(file_name, &sb) == -1)
		return 0;
	return sb.st_mtime;
}
