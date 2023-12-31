/*
 * This file describes the internal interface used by the AVC
 * for calling the user-supplied memory allocation, supplemental
 * auditing, and locking routine, as well as incrementing the
 * statistics fields.
 *
 * Author : Eamon Walsh <ewalsh@epoch.ncsc.mil>
 */
#ifndef _SELINUX_AVC_INTERNAL_H_
#define _SELINUX_AVC_INTERNAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <selinux/avc.h>  //STDLIBC_INTEGER
#include <selinux/callbacks.h>  //STDLIBC_INTEGER
#include <selinux/dso.h>  //STDLIBC_INTEGER

/* callback pointers */
extern void *(*avc_func_malloc) (size_t) hidden;
extern void (*avc_func_free) (void *)hidden;

extern void (*avc_func_log) (const char *, ...) __attribute__((__format__(printf,1,2))) hidden;
extern void (*avc_func_audit) (void *, security_class_t, char *, size_t)hidden;

extern int avc_using_threads hidden;
extern int avc_app_main_loop hidden;
extern void *(*avc_func_create_thread) (void (*)(void))hidden;
extern void (*avc_func_stop_thread) (void *)hidden;

extern void *(*avc_func_alloc_lock) (void)hidden;
extern void (*avc_func_get_lock) (void *)hidden;
extern void (*avc_func_release_lock) (void *)hidden;
extern void (*avc_func_free_lock) (void *)hidden;

static inline void set_callbacks(const struct avc_memory_callback *mem_cb,
				 const struct avc_log_callback *log_cb,
				 const struct avc_thread_callback *thread_cb,
				 const struct avc_lock_callback *lock_cb)
{
	if (mem_cb) {
		avc_func_malloc = mem_cb->func_malloc;
		avc_func_free = mem_cb->func_free;
	}
	if (log_cb) {
		avc_func_log = log_cb->func_log;
		avc_func_audit = log_cb->func_audit;
	}
	if (thread_cb) {
		avc_using_threads = 1;
		avc_func_create_thread = thread_cb->func_create_thread;
		avc_func_stop_thread = thread_cb->func_stop_thread;
	}
	if (lock_cb) {
		avc_func_alloc_lock = lock_cb->func_alloc_lock;
		avc_func_get_lock = lock_cb->func_get_lock;
		avc_func_release_lock = lock_cb->func_release_lock;
		avc_func_free_lock = lock_cb->func_free_lock;
	}
}

/* message prefix and enforcing mode*/
#define AVC_PREFIX_SIZE 16
extern char avc_prefix[AVC_PREFIX_SIZE] hidden;
extern int avc_running hidden;
extern int avc_enforcing hidden;
extern int avc_setenforce hidden;

/* user-supplied callback interface for avc */
static inline void *avc_malloc(size_t size)
{
	return avc_func_malloc ? avc_func_malloc(size) : malloc(size);
}

static inline void avc_free(void *ptr)
{
	if (avc_func_free)
		avc_func_free(ptr);
	else
		free(ptr);
}

/* this is a macro in order to use the variadic capability. */
#define avc_log(type, format...) \
  if (avc_func_log) \
    avc_func_log(format); \
  else \
    selinux_log(type, format);

static inline void avc_suppl_audit(void *ptr, security_class_t class,
				   char *buf, size_t len)
{
	if (avc_func_audit)
		avc_func_audit(ptr, class, buf, len);
	else
		selinux_audit(ptr, class, buf, len);
}

static inline void *avc_create_thread(void (*run) (void))
{
	return avc_func_create_thread ? avc_func_create_thread(run) : NULL;
}

static inline void avc_stop_thread(void *thread)
{
	if (avc_func_stop_thread)
		avc_func_stop_thread(thread);
}

static inline void *avc_alloc_lock(void)
{
	return avc_func_alloc_lock ? avc_func_alloc_lock() : NULL;
}

static inline void avc_get_lock(void *lock)
{
	if (avc_func_get_lock)
		avc_func_get_lock(lock);
}

static inline void avc_release_lock(void *lock)
{
	if (avc_func_release_lock)
		avc_func_release_lock(lock);
}

static inline void avc_free_lock(void *lock)
{
	if (avc_func_free_lock)
		avc_func_free_lock(lock);
}

/* statistics helper routines */
#ifdef AVC_CACHE_STATS

#define avc_cache_stats_incr(field) \
  cache_stats.field ++;
#define avc_cache_stats_add(field, num) \
  cache_stats.field += num;

#else

#define avc_cache_stats_incr(field)
#define avc_cache_stats_add(field, num)

#endif

/* logging helper routines */
#define AVC_AUDIT_BUFSIZE 1024

/* again, we need the variadic capability here */
#define log_append(buf,format...) \
  snprintf(buf+strlen(buf), AVC_AUDIT_BUFSIZE-strlen(buf), format)

/* internal callbacks */
int avc_ss_grant(security_id_t ssid, security_id_t tsid,
		 security_class_t tclass, access_vector_t perms,
		 uint32_t seqno) hidden;
int avc_ss_try_revoke(security_id_t ssid, security_id_t tsid,
		      security_class_t tclass,
		      access_vector_t perms, uint32_t seqno,
		      access_vector_t * out_retained) hidden;
int avc_ss_revoke(security_id_t ssid, security_id_t tsid,
		  security_class_t tclass, access_vector_t perms,
		  uint32_t seqno) hidden;
int avc_ss_reset(uint32_t seqno) hidden;
int avc_ss_set_auditallow(security_id_t ssid, security_id_t tsid,
			  security_class_t tclass, access_vector_t perms,
			  uint32_t seqno, uint32_t enable) hidden;
int avc_ss_set_auditdeny(security_id_t ssid, security_id_t tsid,
			 security_class_t tclass, access_vector_t perms,
			 uint32_t seqno, uint32_t enable) hidden;

/* netlink kernel message code */
extern int avc_netlink_trouble hidden;

hidden_proto(avc_av_stats)
    hidden_proto(avc_cleanup)
    hidden_proto(avc_reset)
    hidden_proto(avc_audit)
    hidden_proto(avc_has_perm_noaudit)
#endif				/* _SELINUX_AVC_INTERNAL_H_ */
