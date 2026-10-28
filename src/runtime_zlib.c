/* libSceZlib: one-shot zlib inflate with the PS4 request plumbing.
 * The guest expects asynchronous requests, but decoding is done synchronously
 * and the completed request is queued until sceZlibWaitForDone. */
#define _GNU_SOURCE
#include "runtime.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#ifdef _WIN32
#include "miniz.h"
typedef mz_stream ZStream;
#define z_init(s) mz_inflateInit(s)
#define z_inflate(s,f) mz_inflate(s,f)
#define z_end(s) mz_inflateEnd(s)
#define Z_OK MZ_OK
#define Z_STREAM_END MZ_STREAM_END
#define Z_BUF_ERROR MZ_BUF_ERROR
#define Z_FINISH MZ_FINISH
#else
#include <zlib.h>
typedef z_stream ZStream;
#define z_init(s) inflateInit(s)
#define z_inflate(s,f) inflate(s,f)
#define z_end(s) inflateEnd(s)
#endif

#define ERR_NOT_FOUND ((int32_t)0x81120002)
#define ERR_INVALID_ARGUMENT ((int32_t)0x81120016)
#define ERR_NO_SPACE ((int32_t)0x8112001C)
#define ERR_TIMED_OUT ((int32_t)0x81120027)
#define ERR_NOT_INITIALIZED ((int32_t)0x81120032)
#define ERR_ALREADY_INITIALIZED ((int32_t)0x81120033)

#define MAX_RESULTS 4096
#define MAX_COMPLETED 4096

typedef struct { int used; uint64_t id; uint32_t produced; int32_t status; } Result;

static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t completed_cond=PTHREAD_COND_INITIALIZER;
static int initialized;
static uint64_t next_id=1;
static Result results[MAX_RESULTS];
static size_t result_next;
static uint64_t completed[MAX_COMPLETED];
static size_t completed_head, completed_count;

static ABI int32_t zlib_initialize(const void *buffer,uint64_t length) {
    int32_t result=0;
    pthread_mutex_lock(&lock);
    if (initialized) result=ERR_ALREADY_INITIALIZED;
    else if ((buffer==NULL)!=(length==0)) result=ERR_INVALID_ARGUMENT;
    else {
        if (buffer) printf("Runtime: zlib work memory (%llu bytes) is managed internally\n",
                           (unsigned long long)length);
        memset(results,0,sizeof(results));
        result_next=0; completed_head=completed_count=0;
        initialized=1;
    }
    pthread_mutex_unlock(&lock);
    return result;
}
static ABI int32_t zlib_finalize(void) {
    pthread_mutex_lock(&lock);
    int32_t result=initialized ? 0 : ERR_NOT_INITIALIZED;
    if (initialized) {
        initialized=0;
        memset(results,0,sizeof(results));
        completed_head=completed_count=0;
        pthread_cond_broadcast(&completed_cond);
    }
    pthread_mutex_unlock(&lock);
    return result;
}
static void store_result(uint64_t id,uint32_t produced,int32_t status) {
    Result *slot=&results[result_next++ % MAX_RESULTS];
    *slot=(Result){.used=1,.id=id,.produced=produced,.status=status};
    if (completed_count==MAX_COMPLETED) {
        completed_head=(completed_head+1)%MAX_COMPLETED;
        --completed_count;
    }
    completed[(completed_head+completed_count)%MAX_COMPLETED]=id;
    ++completed_count;
    pthread_cond_signal(&completed_cond);
}
static ABI int32_t zlib_inflate(const void *source,uint32_t source_length,void *destination,
                                uint32_t destination_length,uint64_t *request_id) {
    pthread_mutex_lock(&lock);
    if (!initialized) { pthread_mutex_unlock(&lock); return ERR_NOT_INITIALIZED; }
    if (!source || !source_length || !destination || !destination_length || !request_id ||
        destination_length>65536 || destination_length%2048 ||
        ((uintptr_t)destination&2047)) {
        pthread_mutex_unlock(&lock);
        return ERR_INVALID_ARGUMENT;
    }
    if (next_id==0) next_id=1;
    ZStream stream;
    memset(&stream,0,sizeof(stream));
    stream.next_in=(unsigned char *)(uintptr_t)source;
    stream.avail_in=source_length;
    stream.next_out=(unsigned char *)destination;
    stream.avail_out=destination_length;
    int32_t status;
    uint32_t produced=0;
    int started=z_init(&stream)==Z_OK;
    if (!started) status=(int32_t)0x811200FF; /* FATAL */
    else {
        int decoded=z_inflate(&stream,Z_FINISH);
        produced=(uint32_t)stream.total_out;
        status=decoded==Z_STREAM_END ? 0 :
            (decoded==Z_BUF_ERROR && stream.avail_out==0 && stream.avail_in!=0 ? ERR_NO_SPACE : (int32_t)0x811200FF);
        z_end(&stream);
    }
    uint64_t id=next_id++;
    if (next_id==0) next_id=1;
    store_result(id,produced,status);
    *request_id=id;
    pthread_mutex_unlock(&lock);
    return 0;
}
static ABI int32_t zlib_wait_for_done(uint64_t *request_id,uint32_t *timeout) {
    pthread_mutex_lock(&lock);
    if (!initialized) { pthread_mutex_unlock(&lock); return ERR_NOT_INITIALIZED; }
    if (!request_id) { pthread_mutex_unlock(&lock); return ERR_INVALID_ARGUMENT; }
    int32_t result=0;
    if (timeout) {
        struct timespec deadline;
        clock_gettime(CLOCK_REALTIME,&deadline);
        deadline.tv_sec+=(time_t)(*timeout/1000000u);
        deadline.tv_nsec+=(long)(*timeout%1000000u)*1000;
        if (deadline.tv_nsec>=1000000000) {
            deadline.tv_sec+=deadline.tv_nsec/1000000000;
            deadline.tv_nsec%=1000000000;
        }
        while (initialized && completed_count==0) {
            int e=pthread_cond_timedwait(&completed_cond,&lock,&deadline);
            if (e==ETIMEDOUT) { result=ERR_TIMED_OUT; break; }
        }
    } else {
        while (initialized && completed_count==0) pthread_cond_wait(&completed_cond,&lock);
    }
    if (!initialized) result=ERR_NOT_INITIALIZED;
    else if (!result && completed_count) {
        *request_id=completed[completed_head];
        completed_head=(completed_head+1)%MAX_COMPLETED;
        --completed_count;
    }
    pthread_mutex_unlock(&lock);
    return result;
}
static ABI int32_t zlib_get_result(uint64_t request_id,uint32_t *destination_length,int32_t *status) {
    pthread_mutex_lock(&lock);
    if (!initialized) { pthread_mutex_unlock(&lock); return ERR_NOT_INITIALIZED; }
    if (!destination_length || !status) { pthread_mutex_unlock(&lock); return ERR_INVALID_ARGUMENT; }
    int32_t result=ERR_NOT_FOUND;
    for (size_t i=0;i<MAX_RESULTS;++i) {
        if (!results[i].used || results[i].id!=request_id) continue;
        *destination_length=results[i].produced;
        *status=results[i].status;
        result=0;
        break;
    }
    pthread_mutex_unlock(&lock);
    return result;
}

static const RuntimeExport exports[]={
    {"sceZlibInitialize",zlib_initialize}, {"sceZlibFinalize",zlib_finalize},
    {"sceZlibInflate",zlib_inflate}, {"sceZlibWaitForDone",zlib_wait_for_done},
    {"sceZlibGetResult",zlib_get_result},
};
uintptr_t runtime_zlib_resolve(const char *name) { return RUNTIME_LOOKUP(exports,name); }
