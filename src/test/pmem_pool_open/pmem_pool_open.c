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
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * pmem_pool_open.c -- unit test for pool_openping different types of pmem pools
 *
 * usage: pmem_pool_open <type>:file ...
 *
 * each file is oppened with:
 * 	pmemobj_pool_open() if the filename starts with t:
 * 	pmemblk_pool_open() if the filename starts with b:
 * 	pmemlog_pool_open() if the filename starts with l:
 * and the handle returned is dereferenced to force a SEGV.
 */

#include "unittest.h"

#define	CHECK_BYTES 4096	/* bytes to compare before/after open call */

sigjmp_buf Jmp;

/*
 * signal_handler -- called on SIGSEGV
 */
void
signal_handler(int sig)
{
	OUT("signal: %s", strsignal(sig));

	siglongjmp(Jmp, 1);
}

int
main(int argc, char *argv[])
{
	START(argc, argv, "pmem_pool_open");

	if (argc < 2)
		FATAL("usage: %s <type>:file ...", argv[0]);

	/* arrange to catch SEGV */
	struct sigvec v = { 0 };
	v.sv_handler = signal_handler;
	SIGVEC(SIGSEGV, &v, NULL);

	/* open each file argument with the given pool type */
	for (int arg = 1; arg < argc; arg++) {
		if (strchr("obl", argv[arg][0]) == NULL || argv[arg][1] != ':')
			FATAL("type must be o: or b: or l:");

		const char *path = &argv[arg][2];
		int fd = OPEN(path, O_RDWR);

		char before[CHECK_BYTES];
		char after[CHECK_BYTES];

		READ(fd, before, CHECK_BYTES);
		CLOSE(fd);

		void *handle = NULL;
		switch (argv[arg][0]) {
		case 'o':
			handle = pmemobj_pool_open(path);
			break;

		case 'b':
			handle = pmemblk_pool_open(path, 4096);
			break;

		case 'l':
			handle = pmemlog_pool_open(path);
			break;

		default:
			FATAL(NULL);	/* can't happen */
		}

		fd = OPEN(path, O_RDWR);
		LSEEK(fd, (off_t)0, SEEK_SET);

		if (READ(fd, after, CHECK_BYTES) == CHECK_BYTES) {
			if (memcmp(before, after, CHECK_BYTES))
				OUT("%s: first %d bytes changed during "
					"pool_open", argv[arg], CHECK_BYTES);
			else
				OUT("%s: first %d bytes unchanged during "
					"pool_open", argv[arg], CHECK_BYTES);
		}

		CLOSE(fd);

		if (handle == NULL) {
			switch (argv[arg][0]) {
			case 'o':
				OUT("!pmemobj_pool_open");
				break;

			case 'b':
				OUT("!pmemblk_pool_open");
				break;

			case 'l':
				OUT("!pmemlog_pool_open");
				break;

			default:
				FATAL(NULL);	/* can't happen */
			}
		} else if (!sigsetjmp(Jmp, 1)) {
			/* try to deref the opaque handle */
			char x = *(char *)handle;

			OUT("x = %c", x);	/* shouldn't get here */
		} else {
			/* back from signal handler, close the pool */
			switch (argv[arg][0]) {
			case 'o':
				pmemobj_pool_close(handle);
				break;

			case 'b':
				pmemblk_pool_close(handle);
				break;

			case 'l':
				pmemlog_pool_close(handle);
				break;

			default:
				FATAL(NULL);	/* can't happen */
			}
		}
	}

	DONE(NULL);
}
