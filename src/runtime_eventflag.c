/* libkernel event flags. A flag is a 64-bit pattern with AND/OR wait modes and
 * optional clear-on-wake, as exposed by the PS4 kernel. */
#define _GNU_SOURCE
#include "runtime.h"
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define ERR(n) ((int32_t)(UINT32_C(0x80020000)|(n)))
#define MAX_EVENT_FLAGS 128
#define ATTR_SINGLE 0x10
#define ATTR_MULTI 0x20
#define WAIT_AND 0x01
#define WAIT_OR 0x02
#define WAIT_CLEAR_ALL 0x10
#define WAIT_CLEAR_PAT 0x20

typedef struct {
    int used, initialized;
    uint32_t attr;
    uint64_t pattern;
    char name[32];
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} EventFlag;

static EventFlag flags[MAX_EVENT_FLAGS];
static pthread_mutex_t flags_lock = PTHREAD_MUTEX_INITIALIZER;

static EventFlag *lookup(int32_t id) {
    if (id <= 0 || id > MAX_EVENT_FLAGS) return NULL;
    EventFlag *flag = &flags[id - 1];
    return flag->used ? flag : NULL;
}

static ABI int32_t event_flag_create(const char *name, uint32_t attr, uint64_t pattern, const void *option) {
    (void)option;
    if ((attr & ATTR_SINGLE) && (attr & ATTR_MULTI)) return ERR(EINVAL);
    pthread_mutex_lock(&flags_lock);
    for (int i = 0; i < MAX_EVENT_FLAGS; ++i) {
        EventFlag *flag = &flags[i];
        if (flag->used) continue;
        if (!flag->initialized) {
            if (pthread_mutex_init(&flag->mutex, NULL) ||
                pthread_cond_init(&flag->cond, NULL)) {
                pthread_mutex_unlock(&flags_lock);
                return ERR(ENOMEM);
            }
            flag->initialized = 1;
        }
        flag->used = 1;
        flag->attr = attr;
        flag->pattern = pattern;
        memset(flag->name, 0, sizeof(flag->name));
        if (name) {
            strncpy(flag->name, name, sizeof(flag->name) - 1);
        }
        pthread_mutex_unlock(&flags_lock);
        return i + 1;
    }
    pthread_mutex_unlock(&flags_lock);
    return ERR(ENOMEM);
}

static ABI int32_t event_flag_delete(int32_t id) {
    pthread_mutex_lock(&flags_lock);
    EventFlag *flag = lookup(id);
    if (!flag) {
        pthread_mutex_unlock(&flags_lock);
        return ERR(ESRCH);
    }
    flag->used = 0;
    flag->pattern = 0;
    pthread_mutex_unlock(&flags_lock);
    return 0;
}

static ABI int32_t event_flag_set(int32_t id, uint64_t bits) {
    EventFlag *flag = lookup(id);
    if (!flag) return ERR(ESRCH);
    pthread_mutex_lock(&flag->mutex);
    flag->pattern |= bits;
    pthread_cond_broadcast(&flag->cond);
    pthread_mutex_unlock(&flag->mutex);
    return 0;
}

static ABI int32_t event_flag_clear(int32_t id, uint64_t bits) {
    EventFlag *flag = lookup(id);
    if (!flag) return ERR(ESRCH);
    pthread_mutex_lock(&flag->mutex);
    flag->pattern &= ~bits;
    pthread_mutex_unlock(&flag->mutex);
    return 0;
}

static int wait_ready(uint64_t pattern, uint64_t bits, uint32_t mode) {
    if (mode & WAIT_AND) return (pattern & bits) == bits;
    if (mode & WAIT_OR) return (pattern & bits) != 0;
    return 0;
}

static void wait_deadline(struct timespec *deadline, uint32_t microseconds) {
    clock_gettime(CLOCK_REALTIME, deadline);
    deadline->tv_sec += microseconds / 1000000;
    deadline->tv_nsec += (long)(microseconds % 1000000) * 1000;
    if (deadline->tv_nsec >= 1000000000) {
        ++deadline->tv_sec;
        deadline->tv_nsec -= 1000000000;
    }
}

static ABI int32_t event_flag_wait(int32_t id, uint64_t bits, uint32_t mode,
                                   uint64_t *result, uint32_t *timeout) {
    if (!(mode & (WAIT_AND | WAIT_OR)) || ((mode & WAIT_AND) && (mode & WAIT_OR)))
        return ERR(EINVAL);
    EventFlag *flag = lookup(id);
    if (!flag) return ERR(ESRCH);

    struct timespec deadline;
    if (timeout) wait_deadline(&deadline, *timeout);

    pthread_mutex_lock(&flag->mutex);
    while (!wait_ready(flag->pattern, bits, mode)) {
        if (timeout) {
            int rc = pthread_cond_timedwait(&flag->cond, &flag->mutex, &deadline);
            if (rc == ETIMEDOUT) {
                pthread_mutex_unlock(&flag->mutex);
                *timeout = 0;
                return ERR(ETIMEDOUT);
            }
            if (rc) {
                pthread_mutex_unlock(&flag->mutex);
                return ERR(EINVAL);
            }
        } else {
            pthread_cond_wait(&flag->cond, &flag->mutex);
        }
    }
    if (result) *result = flag->pattern;
    if (mode & WAIT_CLEAR_ALL) flag->pattern = 0;
    else if (mode & WAIT_CLEAR_PAT) flag->pattern &= ~bits;
    pthread_mutex_unlock(&flag->mutex);
    return 0;
}

static const RuntimeExport exports[] = {
    {"sceKernelCreateEventFlag", event_flag_create},
    {"sceKernelDeleteEventFlag", event_flag_delete},
    {"sceKernelSetEventFlag", event_flag_set},
    {"sceKernelClearEventFlag", event_flag_clear},
    {"sceKernelWaitEventFlag", event_flag_wait},
};

uintptr_t runtime_eventflag_resolve(const char *name) {
    return RUNTIME_LOOKUP(exports, name);
}
