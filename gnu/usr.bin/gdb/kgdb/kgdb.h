/*
 * Copyright (c) 2004 Marcel Moolenaar
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD: src/gnu/usr.bin/gdb/kgdb/kgdb.h,v 1.3 2005/09/10 18:25:53 marcel Exp $
 * $DragonFly: src/gnu/usr.bin/gdb/kgdb/kgdb.h,v 1.4 2008/01/14 21:36:38 corecode Exp $
 */

#ifndef _KGDB_H_
#define	_KGDB_H_

struct thread_info;

extern kvm_t *kvm;

struct kthr {
	struct kthr	*next;
	uintptr_t	paddr;
	uintptr_t	kaddr;
	uintptr_t	kstack;
	uintptr_t	pcb;
	int		tid;
	int		pid;
};

extern struct kthr *curkthr;

uintptr_t lookup(const char *);

void kgdb_target(void);
void kgdb_trgt_fetch_registers(struct regcache *, int);
void kgdb_trgt_store_registers(struct regcache *, int);

extern const struct frame_unwind kgdb_trgt_trapframe_unwind;

struct kthr *kgdb_thr_first(void);
struct kthr *kgdb_thr_init(void);
struct kthr *kgdb_thr_lookup_tid(int);
struct kthr *kgdb_thr_lookup_pid(int);
struct kthr *kgdb_thr_lookup_paddr(uintptr_t);
struct kthr *kgdb_thr_lookup_taddr(uintptr_t);
struct kthr *kgdb_thr_next(struct kthr *);
struct kthr *kgdb_thr_select(struct kthr *);
char        *kgdb_thr_extra_thread_info(int);

#endif /* _KGDB_H_ */
