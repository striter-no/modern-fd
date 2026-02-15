# MFD - Modern FD

Small utility project for handling tasks, related to file descriptors (fds)

## Functionality

Provides several types of FD structures:

- `r_fd`: 
  - raw fd, contains `fd` (from kernel), `writable` and `readable` properties
- `g_fd`: 
  - "general" fd, created from `pipe2()`, contains 2 `r_fd` (*ro and wo*)
- `p_fd`: 
  - programmable fd, just buffer of bytes with `used` property

Provides neat functions for working with fds:

- `mfd_write`: Generic macro for all of 3 types fds to **write** bytes
- `mfd_read`: Generic macro for all of 3 types fds to **read** bytes

- `mfd_redirect`: reads from `src` fd and writes data to `dest` fd
- `mfd_available`: returns number of bytes, available for reading in fd
- `mfd_set_flags`: sets given flags for r_fd
- `mfd_set_flags`: returns flags of r_fd

Also provies **streams**, structure for dealing with fds easier. They can be used for convenient reading and writing:

- `mfd_str_readexact`: reads exactly N given bytes
- `mfd_str_readmax`: reads until no data available
- `mfd_str_writeexact`: writes exactly N given bytes

## Example


```c
#define _GNU_SOURCE
#include <mfd/streams.h>
#include <stdio.h>

int main(void){
	FILE *f = fopen("./assets/test.txt", "r");
	int fd = dup(fileno(f));
	fclose(f);

	mfd_stream str;

    // And readable and writeable
	mfd_str_create(fd, true, true, &str);

    // Reading all content of the file
	size_t size = 0;
	char   *content = NULL;
	mfd_str_readmax(str, &size, (void **)&content);

    // Closing stream
	mfd_str_end(&str);

	printf("Just read:\n%s\n", content);
	free(content);
}
```