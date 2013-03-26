#include "xsltp_config.h"
#include "xsltp_core.h"

#ifdef HAVE_THREADS

xsltp_mutex_t *xsltp_mutex_init(void) {
	xsltp_mutex_t *mutex;

    if ((mutex = xsltp_malloc(sizeof(xsltp_mutex_t))) == NULL) {
		return NULL;
	}

	if (pthread_mutex_init(&mutex->mutex, NULL) != 0) {
		perror("Mutex init failed");
		return NULL;
	}

	return mutex;
}

void xsltp_mutex_destroy(xsltp_mutex_t *mutex) {
    if (mutex != NULL) {
        if (pthread_mutex_destroy(&mutex->mutex) != 0) {
            perror("Mutex destroy failed");
        }
        xsltp_free(mutex);
    }
}

void xsltp_mutex_lock(xsltp_mutex_t *mutex) {
	if (pthread_mutex_lock(&mutex->mutex) != 0) {
		perror("Mutex lock failed");
	}
}

void xsltp_mutex_unlock(xsltp_mutex_t *mutex) {
	if (pthread_mutex_unlock(&mutex->mutex) != 0) {
		perror("Mutex unlock failed");
	}
}

xsltp_cond_t *xsltp_cond_init(void) {
	xsltp_cond_t *cond;

    if ((cond = xsltp_malloc(sizeof(xsltp_cond_t))) == NULL) {
		return NULL;
	}

	if (pthread_cond_init(&cond->cond, NULL) != 0) {
		perror("Condition init failed");
		return NULL;
	}

	return cond;
}

void xsltp_cond_destroy(xsltp_cond_t *cond) {
    if (cond != NULL) {
        if (pthread_cond_destroy(&cond->cond) != 0) {
            perror("Condition destroy failed");
        }
        xsltp_free(cond);
    }
}

void xsltp_cond_signal(xsltp_cond_t *cond) {
	if (pthread_cond_signal(&cond->cond) != 0) {
		perror("Condition signaling failed");
	}
}

void xsltp_cond_broadcast(xsltp_cond_t *cond) {
	if (pthread_cond_broadcast(&cond->cond) != 0) {
		perror("Condition broadcast failed");
	}
}

void xsltp_cond_wait(xsltp_cond_t *cond, xsltp_mutex_t *mutex) {
	if (pthread_cond_wait(&cond->cond, &mutex->mutex) != 0) {
		perror("Condition wait failed");
	}
}

xsltp_rwlock_t *xsltp_rwlock_init(void) {
	xsltp_rwlock_t *rwlock;

    if ((rwlock = xsltp_malloc(sizeof(xsltp_rwlock_t))) == NULL) {
		return NULL;
	}

	if (pthread_rwlock_init(&rwlock->rwlock, NULL) != 0) {
		perror("RWLock init failed");
		return NULL;
	}

	return rwlock;
}

void xsltp_rwlock_destroy(xsltp_rwlock_t *rwlock) {
	if (pthread_rwlock_destroy(&rwlock->rwlock) != 0) {
		perror("RWLock destroy failed");
	}
	xsltp_free(rwlock);
}

void xsltp_rwlock_rdlock(xsltp_rwlock_t *rwlock) {
	if (pthread_rwlock_rdlock(&rwlock->rwlock) != 0) {
		perror("Read lock failed");
	}
}

void xsltp_rwlock_wrlock(xsltp_rwlock_t *rwlock) {
	if (pthread_rwlock_wrlock(&rwlock->rwlock) != 0) {
		perror("Write lock failed");
	}
}
void xsltp_rwlock_unlock(xsltp_rwlock_t *rwlock) {
	if (pthread_rwlock_unlock(&rwlock->rwlock) != 0) {
		perror("RWLock unlock failed");
	}
}

#endif /* HAVE_THREADS */
