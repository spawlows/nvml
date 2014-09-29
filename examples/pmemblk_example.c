/*
 * Copyright (c) 2014, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY LOG OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libpmem.h>

/* size of each element in the PMEM pool (bytes) */
#define	ELEMENT_SIZE ((size_t)1024)

int
main(int argc, char *argv[])
{
	int fd;
	PMEMblk *pbp;
	size_t nelements;
	char buf[ELEMENT_SIZE];

	/* create file on PMEM-aware file system */
	if ((fd = open("/my/pmem-aware/fs/myfile",
					O_CREAT|O_RDWR, 0666)) < 0) {
		perror("open");
		exit(1);
	}

	/* pre-allocate 2GB of persistent memory */
	if ((errno = posix_fallocate(fd, (off_t)0,
					(size_t)1024 * 1024 * 1024 * 2)) != 0) {
		perror("posix_fallocate");
		exit(1);
	}

	/* create an array of atomically writable elements */
	if ((pbp = pmemblk_map(fd, ELEMENT_SIZE)) == NULL) {
		perror("pmemblk_map");
		exit(1);
	}

	/* how many elements fit into the PMEM pool? */
	nelements = pmemblk_nblock(pbp);
	printf("file holds %zu elements\n", nelements);

	/* store a block at index 5 */
	strcpy(buf, "hello, world");
	if (pmemblk_write(pbp, buf, 5) < 0) {
		perror("pmemblk_write");
		exit(1);
	}

	/* read the block at index 10 (reads as zeros initially) */
	if (pmemblk_read(pbp, buf, 10) < 0) {
		perror("pmemblk_write");
		exit(1);
	}

	/* zero out the block at index 5 */
	if (pmemblk_set_zero(pbp, 5) < 0) {
		perror("pmemblk_set_zero");
		exit(1);
	}

	/* ... */

	pmemblk_unmap(pbp);
	close(fd);
}
