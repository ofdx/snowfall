/*
	encoder.c
	mperron (2021)

	Reads from the file specified in the first argument and writes out a
	comma-separated list of bytes in hex value with 0x prepended. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ_BLOCK_SIZE 256

int main(int argc, char **argv){
	char buffer[READ_BLOCK_SIZE];
	unsigned char *data = NULL;
	size_t n = 0, r;

	if(argc < 2){
		fprintf(stderr, "Usage:\n\t%s <file>\n", *argv);
		return 0;
	}

	FILE *source = fopen(argv[1], "r");

	if(!source){
		fprintf(stderr, "File not found: %s\n", argv[1]);
		return 1;
	}

	while((r = fread(buffer, sizeof(char), READ_BLOCK_SIZE, source))){
		n += r;
		data = realloc(data, n * sizeof(char));

		if(!data){
			fprintf(stderr, "Failed to realloc input holder!\n");
			return 2;
		}

		memcpy(data + n - r, buffer, r);
	}

	fclose(source);

	for(int i = 0; i < n; i++)
		printf(
			"0x%02x%s%s",
			*(data++),
			(i == (n - 1) ? "" : ", "),
			(((i + 1) % 16) ? "" : "\n")
		);

	return 0;
}
