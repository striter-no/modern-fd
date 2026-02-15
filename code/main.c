#define _GNU_SOURCE
#include <mfd/streams.h>
#include <stdio.h>

int main(void){
	FILE *f = fopen("./assets/test.txt", "r");
	int fd = dup(fileno(f));
	fclose(f);

	mfd_stream str;
	mfd_str_create(fd, true, true, &str);

	size_t size = 0;
	char   *content = NULL;
	mfd_str_readmax(str, &size, (void **)&content);

	mfd_str_end(&str);

	printf("Just read:\n%s\n", content);
	free(content);
}
