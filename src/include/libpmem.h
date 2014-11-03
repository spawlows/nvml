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
 * libpmem.h -- definitions of libpmem entry points
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
 * See libpmem(3) for details.
 */

#ifndef	LIBPMEM_H
#define	LIBPMEM_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/uio.h>

/*
 * basic PMEM flush-to-durability support...
 */
void *pmem_map(int fd);
int pmem_is_pmem(void *addr, size_t len);
void pmem_persist(void *addr, size_t len, int flags);
int pmem_persist_msync(int is_pmem, void *addr, size_t len);
void pmem_flush(void *addr, size_t len, int flags);
void pmem_fence(void);
void pmem_drain(void);

/*
 * managing overall library behavior...
 */

/*
 * PMEM_MAJOR_VERSION and PMEM_MINOR_VERSION provide the current
 * version of the libpmem API as provided by this header file.
 * Applications can verify that the version available at run-time
 * is compatible with the version used at compile-time by passing
 * these defines to pmem_check_version().
 */
#define	PMEM_MAJOR_VERSION 1
#define	PMEM_MINOR_VERSION 0
const char *pmem_check_version(
		unsigned major_required,
		unsigned minor_required);

/*
 * Passing NULL to pmem_set_funcs() tells libpmem to continue to use
 * the default for that function.  The replacement functions must
 * not make calls back into libpmem.
 *
 * The print_func is called by libpmem based on the environment
 * variable PMEM_LOG_LEVEL:
 * 	0 or unset: print_func is only called for pmem_pool_stats_print()
 * 	1:          additional details are logged when errors are returned
 * 	2:          basic operations (allocations/frees) are logged
 * 	3:          produce very verbose tracing of function calls in libpmem
 * 	4:          also log obscure stuff used to debug the library itself
 *
 * The default print_func prints to stderr.  Applications can override this
 * by setting the environment variable PMEM_LOG_FILE, or by supplying a
 * replacement print function.
 */
void pmem_set_funcs(
		void *(*malloc_func)(size_t size),
		void (*free_func)(void *ptr),
		void *(*realloc_func)(void *ptr, size_t size),
		char *(*strdup_func)(const char *s),
		void (*print_func)(const char *s),
		void (*persist_func)(void *addr, size_t len, int flags));


#ifdef __cplusplus
}
#endif
#endif	/* libpmem.h */
