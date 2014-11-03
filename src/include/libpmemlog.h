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
 * libpmemlog.h -- definitions of libpmem entry points
 *
 * This library provides support for programming with Persistent Memory (PMEM).
 *
 * The libpmem entry points are divided below into these categories:
 *	- basic PMEM flush-to-durability support
 *	- support for memory allocation and transactions in PMEM
 *	- support for arrays of atomically-writable blocks
 *	- support for PMEM-resident log files
 *	- managing overall library behavior
 *
 * See libpmemlog(3) for details.
 */

#ifndef	LIBPMEMLOG_H
#define	LIBPMEMLOG_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/uio.h>

/*
 * opaque types internal to libpmemlog
 */
typedef struct pmemlog PMEMlogpool;

/*
 * support for PMEM-resident log files...
 */
#define	PMEMLOG_MIN_POOL ((size_t)(1024 * 1024 * 2)) /* min pool size: 2MB */

PMEMlogpool *pmemlog_pool_open(const char *path);
void pmemlog_pool_close(PMEMlogpool *plp);
int pmemlog_pool_check(const char *path);

size_t pmemlog_nbyte(PMEMlogpool *plp);
int pmemlog_append(PMEMlogpool *plp, const void *buf, size_t count);
int pmemlog_appendv(PMEMlogpool *plp, const struct iovec *iov, int iovcnt);
off_t pmemlog_tell(PMEMlogpool *plp);
void pmemlog_rewind(PMEMlogpool *plp);
void pmemlog_walk(PMEMlogpool *plp, size_t chunksize,
	int (*process_chunk)(const void *buf, size_t len, void *arg),
	void *arg);


#ifdef __cplusplus
}
#endif
#endif	/* libpmemlog.h */