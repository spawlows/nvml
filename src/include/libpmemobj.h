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
 * libpmemobj.h -- definitions of libpmem entry points
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
 * See libpmemobj(3) for details.
 */

#ifndef	LIBPMEMOBJ_H
#define	LIBPMEMOBJ_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/uio.h>
#include <setjmp.h>

/*
 * opaque types internal to libpmemobj
 */
typedef struct pmemobjpool PMEMobjpool;

/*
 * support for memory allocation and transactions in PMEM...
 */
#define	PMEMOBJ_MIN_POOL ((size_t)(1024 * 1024 * 2)) /* min pool size: 2MB */

/* path can be "/file/one:/file/two" to force mirrored operation */
PMEMobjpool *pmemobj_pool_open(const char *path);
PMEMobjpool *pmemobj_pool_open_mirrored(const char *path1, const char *path2);
void pmemobj_pool_close(PMEMobjpool *pop);
int pmemobj_pool_check(const char *path);
int pmemobj_pool_check_mirrored(const char *path1, const char *path2);

/*
 * Object IDs used with pmemobj...
 */
typedef struct pmemoid {
	uint64_t pool;
	uint64_t off;
} PMEMoid;

/*
 * transaction ID
 */
typedef uintptr_t PMEMtid;

/*
 * PMEMmutex is a pthread_mutex_t designed to live in a pmem-resident
 * data structure.  Unlike the rest of the things in pmem, this is a
 * volatile lock so any persistent state is ignored and the lock
 * re-initializes itself to a fresh, DRAM-resident lock each time
 * the program is run.
 */
typedef struct pmemmutex {
	uint64_t runid;		/* matches if pthread_mutexp is initialized */
	pthread_mutex_t *pthread_mutexp;
} PMEMmutex;

typedef struct pmemrwlock {
	uint64_t runid;		/* matches if pthread_rwlockp is initialized */
	pthread_rwlock_t *pthread_rwlockp;
} PMEMrwlock;

typedef struct pmemcond {
	uint64_t runid;		/* matches if pthread_condp is initialized */
	pthread_cond_t *pthread_condp;
} PMEMcond;

int pmemobj_mutex_init(PMEMmutex *mutexp);
int pmemobj_mutex_lock(PMEMmutex *mutexp);
int pmemobj_mutex_trylock(PMEMmutex *mutexp);
int pmemobj_mutex_unlock(PMEMmutex *mutexp);

int pmemobj_rwlock_init(PMEMrwlock *rwlockp);
int pmemobj_rwlock_rdlock(PMEMrwlock *rwlockp);
int pmemobj_rwlock_wrlock(PMEMrwlock *rwlockp);
int pmemobj_rwlock_timedrdlock(PMEMrwlock *restrict rwlockp,
		const struct timespec *restrict abs_timeout);
int pmemobj_rwlock_timedwrlock(PMEMrwlock *restrict rwlockp,
		const struct timespec *restrict abs_timeout);
int pmemobj_rwlock_tryrdlock(PMEMrwlock *rwlockp);
int pmemobj_rwlock_trywrlock(PMEMrwlock *rwlockp);
int pmemobj_rwlock_unlock(PMEMrwlock *rwlockp);

int pmemobj_cond_init(PMEMcond *condp);
int pmemobj_cond_broadcast(PMEMcond *condp);
int pmemobj_cond_signal(PMEMcond *condp);
int pmemobj_cond_timedwait(PMEMcond *restrict condp,
		PMEMmutex *restrict mutexp,
		const struct timespec *restrict abstime);
int pmemobj_cond_wait(PMEMcond *condp,
		PMEMmutex *restrict mutexp);

void *pmemobj_root_direct(PMEMobjpool *pop, size_t size);
int pmemobj_root_resize(PMEMobjpool *pop, size_t size);

PMEMtid pmemobj_tx_begin(PMEMobjpool *pop, jmp_buf env);
PMEMtid pmemobj_tx_begin_lock(PMEMobjpool *pop,
		jmp_buf env, PMEMmutex *mutexp);
PMEMtid pmemobj_tx_begin_wrlock(PMEMobjpool *pop,
		jmp_buf env, PMEMrwlock *rwlockp);
int pmemobj_tx_commit(void);
int pmemobj_tx_commit_tid(PMEMtid tid);
int pmemobj_tx_commit_multi(PMEMtid tid, ...);
int pmemobj_tx_commit_multiv(PMEMtid tids[]);
int pmemobj_tx_abort(int errnum);
int pmemobj_tx_abort_tid(PMEMtid tid, int errnum);

PMEMoid pmemobj_alloc(size_t size);
PMEMoid pmemobj_zalloc(size_t size);
PMEMoid pmemobj_realloc(PMEMoid oid, size_t size);
PMEMoid pmemobj_aligned_alloc(size_t alignment, size_t size);
PMEMoid pmemobj_strdup(const char *s);
int pmemobj_free(PMEMoid oid);

size_t pmemobj_size(PMEMoid oid);	/* no lock/tx required */

PMEMoid pmemobj_alloc_tid(PMEMtid tid, size_t size);
PMEMoid pmemobj_zalloc_tid(PMEMtid tid, size_t size);
PMEMoid pmemobj_realloc_tid(PMEMtid tid, PMEMoid oid, size_t size);
PMEMoid pmemobj_aligned_alloc_tid(PMEMtid tid, size_t alignment, size_t size);
PMEMoid pmemobj_strdup_tid(PMEMtid tid, const char *s);
int pmemobj_free_tid(PMEMtid tid, PMEMoid oid);

void *pmemobj_direct(PMEMoid oid);
void *pmemobj_direct_ntx(PMEMoid oid);

int pmemobj_nulloid(PMEMoid oid);

int pmemobj_memcpy(void *dstp, void *srcp, size_t size);
int pmemobj_memcpy_tid(PMEMtid tid, void *dstp, void *srcp, size_t size);

#define	PMEMOBJ_SET(lhs, rhs)\
	pmemobj_memcpy((void *)&(lhs), (void *)&(rhs), sizeof (lhs))
#define	PMEMOBJ_SET_TID(tid, lhs, rhs)\
	pmemobj_memcpy_tid(tid, (void *)&(lhs), (void *)&(rhs), sizeof (lhs))


#ifdef __cplusplus
}
#endif
#endif	/* libpmemobj.h */
