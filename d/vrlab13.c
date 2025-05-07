#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define HANDLE_ERROR(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <infile> <outfile> <offset> <length>\n", argv[0]);
        fprintf(stderr, "       offset is optional - default 0\n");
        fprintf(stderr, "       length is optional - default to file size or EOF\n");
        exit(EXIT_FAILURE);
    }

	const char *infile = argv[1];
	const char *outfile = argv[2];
	off_t offset = (argc >= 4) ? atoi(argv[3]) : 0;
	size_t length;

	int infd = open(infile, O_RDONLY);
	if (infd == -1) perror("open infile");

	struct stat sb;
	if (fstat(infd, &sb) == -1) perror("fstat");

	if (offset >= sb.st_size) {
		fprintf(stderr, "Error: Offset is beyond end of file\n");
		exit(EXIT_FAILURE);
	}
	length = (argc >= 5) ? atoi(argv[4]) : sb.st_size - offset;
	if (offset + length > sb.st_size) {
		length = sb.st_size - offset;
	}

	off_t pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);

	printf("filesize: %ld bytes\n", sb.st_size);
	printf("pa_offset: %ld\n", pa_offset);
	printf("pid: %d pagesize: %ld bytes\n", getpid(), sysconf(_SC_PAGE_SIZE));

	// Map input file
	char *src = mmap(NULL, length + offset - pa_offset, PROT_READ, MAP_PRIVATE, infd, pa_offset);
	if (src == MAP_FAILED) perror("mmap input");

	// Create anonymous mapping
	char *dest = mmap(NULL, length, PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (dest == MAP_FAILED) perror("mmap anonymous");

	// Copy input memory to anonymous memory
	memcpy(dest, src + (offset - pa_offset), length);

	// Open output file
	int outfd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (outfd == -1) perror("open outfile");

	// Write from anonymous memory to output file
	ssize_t written = write(outfd, dest, length);
	if (written != length) {
		if (written == -1) perror("write");
		fprintf(stderr, "Partial write occurred.\n");
		exit(EXIT_FAILURE);
	}

	printf("%ld bytes written to file.\n", written);

	// Cleanup
	munmap(src, length + offset - pa_offset);
	munmap(dest, length);
	close(infd);
	close(outfd);

	return 0;
}