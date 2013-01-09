#ifndef _XSLTP_THREAD_H_INCLUDED_
#define _XSLTP_THREAD_H_INCLUDED_

#include <pthread.h>

typedef pthread_t XSLTP_TID;

typedef struct {
    pthread_mutex_t mutex;
} xsltp_mutex_t;

typedef struct {
    pthread_cond_t cond;
} xsltp_cond_t;

typedef struct {
    pthread_rwlock_t rwlock;
} xsltp_rwlock_t;

#define XSLTP_THREAD_FREE   1
#define XSLTP_THREAD_BUSY   2
#define XSLTP_THREAD_EXIT   3
#define XSLTP_THREAD_DONE   4

#define XSLTP_THREAD_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER;

xsltp_mutex_t *xsltp_mutex_init(void);
void xsltp_mutex_lock(xsltp_mutex_t *mutex);
void xsltp_mutex_unlock(xsltp_mutex_t *mutex);
void xsltp_mutex_destroy(xsltp_mutex_t *mutex);

xsltp_cond_t *xsltp_cond_init(void);
void xsltp_cond_signal(xsltp_cond_t *cond);
void xsltp_cond_broadcast(xsltp_cond_t *cond);
void xsltp_cond_wait(xsltp_cond_t *cond, xsltp_mutex_t *mutex);
void xsltp_cond_destroy(xsltp_cond_t *cond);

xsltp_rwlock_t *xsltp_rwlock_init(void);
void xsltp_rwlock_destroy(xsltp_rwlock_t *rwlock);
void xsltp_rwlock_rdlock(xsltp_rwlock_t *rwlock);
void xsltp_rwlock_wrlock(xsltp_rwlock_t *rwlock);
void xsltp_rwlock_unlock(xsltp_rwlock_t *rwlock);

#endif /* _XSLTP_THREAD_H_INCLUDED_ */
