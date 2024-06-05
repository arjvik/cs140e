#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libunix.h"

// allocate buffer, read entire file into it, return it.   
// buffer is zero padded to a multiple of 4.
//
//  - <size> = exact nbytes of file.
//  - for allocation: round up allocated size to 4-byte multiple, pad
//    buffer with 0s. 
//
// fatal error: open/read of <name> fails.
//   - make sure to check all system calls for errors.
//   - make sure to close the file descriptor (this will
//     matter for later labs).
// 
void *read_file(unsigned *size, const char *name) {
    FILE *f = fopen(name, "rb");
    if (!f) panic("failed to open file %s", name);
    struct stat st;
    fstat(fileno(f), &st);
    *size = st.st_size;
    char *const buf = calloc(pi_roundup(*size, 4), sizeof(buf[0]));
    assert(buf);
    fread(buf, sizeof(*buf), *size, f);
    fclose(f);
    return buf;
}
