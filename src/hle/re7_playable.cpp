#include "re7_playable.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <atomic>
#include <mutex>
#include <vector>
#include <unordered_map>

namespace Orbis::RE7 {

static const uint8_t ORBIS_SALT[16] = {
    0x51, 0x8D, 0x64, 0xA6, 0x35, 0xDE, 0xD8, 0xC1,
    0xE6, 0xB0, 0x39, 0xB1, 0xC3, 0xE5, 0x52, 0x30
};

static inline uint32_t Rol32(uint32_t v, int b) { return (v << b) | (v >> (32 - b)); }

static void ComputeSHA1(const uint8_t* data, size_t len, uint8_t out[20]) {
    uint32_t h0 = 0x67452301, h1 = 0xEFCDAB89, h2 = 0x98BADCFE, h3 = 0x10325476, h4 = 0xC3D2E1F0;
    size_t newLen = len + 1;
    while ((newLen % 64) != 56) newLen++;
    std::vector<uint8_t> msg(newLen + 8, 0);
    std::memcpy(msg.data(), data, len);
    msg[len] = 0x80;
    uint64_t bits = static_cast<uint64_t>(len) * 8;
    for (int i = 0; i < 8; ++i) msg[newLen + i] = (bits >> (56 - i * 8)) & 0xFF;

    for (size_t offset = 0; offset < msg.size(); offset += 64) {
        uint32_t w[80];
        for (int i = 0; i < 16; ++i) {
            w[i] = (uint32_t(msg[offset + i*4]) << 24) | (uint32_t(msg[offset + i*4 + 1]) << 16) |
                   (uint32_t(msg[offset + i*4 + 2]) << 8) | uint32_t(msg[offset + i*4 + 3]);
        }
        for (int i = 16; i < 80; ++i) w[i] = Rol32(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);
        uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;
        for (int i = 0; i < 80; ++i) {
            uint32_t f, k;
            if (i < 20)      { f = (b & c) | ((~b) & d); k = 0x5A827999; }
            else if (i < 40) { f = b ^ c ^ d;            k = 0x6ED9EBA1; }
            else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
            else             { f = b ^ c ^ d;            k = 0xCA62C1D6; }
            uint32_t temp = Rol32(a, 5) + f + e + k + w[i];
            e = d; d = c; c = Rol32(b, 30); b = a; a = temp;
        }
        h0 += a; h1 += b; h2 += c; h3 += d; h4 += e;
    }
    uint32_t hs[5] = {h0, h1, h2, h3, h4};
    for (int i = 0; i < 5; ++i) {
        out[i*4]   = (hs[i] >> 24) & 0xFF;
        out[i*4+1] = (hs[i] >> 16) & 0xFF;
        out[i*4+2] = (hs[i] >> 8)  & 0xFF;
        out[i*4+3] = hs[i] & 0xFF;
    }
}

static void ComputeOrbisNID(const char* symName, std::string& outNid, uint64_t& outHash64) {
    size_t nLen = std::strlen(symName);
    std::vector<uint8_t> buf(nLen + 16);
    std::memcpy(buf.data(), symName, nLen);
    std::memcpy(buf.data() + nLen, ORBIS_SALT, 16);

    uint8_t digest[20];
    ComputeSHA1(buf.data(), buf.size(), digest);

    uint8_t rev[9] = {
        digest[7], digest[6], digest[5], digest[4],
        digest[3], digest[2], digest[1], digest[0], 0
    };
    std::memcpy(&outHash64, digest, sizeof(uint64_t));

    static const char* B64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-";
    char nid[12];
    int outIdx = 0;
    for (int i = 0; i < 9 && outIdx < 11; i += 3) {
        uint32_t v = (uint32_t(rev[i]) << 16) | (uint32_t(rev[i+1]) << 8) | uint32_t(rev[i+2]);
        nid[outIdx++] = B64[(v >> 18) & 0x3F];
        if (outIdx < 11) nid[outIdx++] = B64[(v >> 12) & 0x3F];
        if (outIdx < 11) nid[outIdx++] = B64[(v >> 6)  & 0x3F];
        if (outIdx < 11) nid[outIdx++] = B64[v & 0x3F];
    }
    nid[11] = '\0';
    outNid = nid;
}

struct VmaRange {
    uintptr_t start;
    uintptr_t end;
    off_t     offset;
    int32_t   prot;
    int32_t   memType;
    bool      isFlex;
    bool      isDirect;
    char      name[32];
};

static std::mutex g_vmaMutex;
static std::vector<VmaRange> g_vmaList;
static std::atomic<uintptr_t> g_virtCursor{0x200000000ULL};
static std::atomic<off_t> g_directMemPhysCursor{0x100000};
static constexpr size_t RE7_DIRECT_MEM_SIZE = 5ULL * 1024 * 1024 * 1024;

static inline uintptr_t AlignUp(uintptr_t v, size_t align) {
    size_t mask = (align > 0) ? (align - 1) : 0xFFFF;
    return (v + mask) & ~mask;
}

static void RecordVma(uintptr_t start, size_t len, off_t off, int32_t prot, bool isFlex, bool isDirect, const char* name) {
    std::lock_guard<std::mutex> lk(g_vmaMutex);
    VmaRange r{};
    r.start = start;
    r.end = start + len;
    r.offset = off;
    r.prot = prot ? prot : 0x33;
    r.memType = isDirect ? 3 : 0;
    r.isFlex = isFlex;
    r.isDirect = isDirect;
    std::snprintf(r.name, sizeof(r.name), "%s", name ? name : "RE7_MEM");
    g_vmaList.push_back(r);
}

size_t ORBIS_ABI sceKernelGetDirectMemorySize() { return RE7_DIRECT_MEM_SIZE; }

int32_t ORBIS_ABI sceKernelAllocateDirectMemory(off_t searchStart, off_t searchEnd, size_t len, size_t alignment, int32_t memoryType, off_t* physAddrOut) {
    if (!physAddrOut || len == 0) return -22;
    size_t align = (alignment >= 0x4000) ? alignment : 0x10000;
    off_t base = g_directMemPhysCursor.fetch_add(static_cast<off_t>(len + align));
    base = static_cast<off_t>(AlignUp(static_cast<uintptr_t>(base), align));
    if (searchStart > 0 && base < searchStart) base = static_cast<off_t>(AlignUp(static_cast<uintptr_t>(searchStart), align));
    (void)searchEnd; (void)memoryType;
    *physAddrOut = base;
    return 0;
}

int32_t ORBIS_ABI sceKernelMapNamedDirectMemory(void** addrInOut, size_t len, int32_t prot, int32_t flags, off_t directMemoryStart, size_t alignment, const char* name) {
    if (!addrInOut || len == 0) return -22;
    size_t align = (alignment >= 0x4000) ? alignment : 0x10000;
    uintptr_t req = reinterpret_cast<uintptr_t>(*addrInOut);
    int mmapFlags = MAP_PRIVATE | MAP_ANONYMOUS;
    if (req >= 0x100000000ULL && (flags & 0x10 || flags & 0x20)) {
        mmapFlags |= MAP_FIXED;
    } else if (req < 0x100000000ULL) {
        req = AlignUp(g_virtCursor.fetch_add(len + align), align);
    }
    int hostProt = PROT_READ | PROT_WRITE;
    if (prot & 0x04) hostProt |= PROT_EXEC;

    void* mapped = ::mmap(reinterpret_cast<void*>(req), len, hostProt, mmapFlags, -1, 0);
    if (mapped == MAP_FAILED) {
        mapped = ::mmap(nullptr, len, hostProt, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (mapped == MAP_FAILED) return -12;
    }
    *addrInOut = mapped;
    RecordVma(reinterpret_cast<uintptr_t>(mapped), len, directMemoryStart, prot, false, true, name ? name : "RE7_Direct");
    return 0;
}

int32_t ORBIS_ABI sceKernelMapDirectMemory(void** addrInOut, size_t len, int32_t prot, int32_t flags, off_t directMemoryStart, size_t alignment) {
    return sceKernelMapNamedDirectMemory(addrInOut, len, prot, flags, directMemoryStart, alignment, "RE7_Direct");
}

int32_t ORBIS_ABI sceKernelReserveVirtualRange(void** addrInOut, size_t len, int32_t flags, size_t alignment) {
    if (!addrInOut || len == 0) return -22;
    size_t align = (alignment >= 0x4000) ? alignment : 0x10000;
    uintptr_t req = reinterpret_cast<uintptr_t>(*addrInOut);
    if (req < 0x100000000ULL) {
        req = AlignUp(g_virtCursor.fetch_add(len + align), align);
    } else {
        req = AlignUp(req, align);
    }
    (void)flags;
    void* res = ::mmap(reinterpret_cast<void*>(req), len, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (res == MAP_FAILED) return -12;
    *addrInOut = res;
    RecordVma(reinterpret_cast<uintptr_t>(res), len, 0, 0, true, false, "RE7_Reserved");
    return 0;
}

int32_t ORBIS_ABI sceKernelMapNamedFlexibleMemory(void** addrInOut, size_t len, int32_t prot, int32_t flags, const char* name) {
    if (!addrInOut || len == 0) return -22;
    uintptr_t target = reinterpret_cast<uintptr_t>(*addrInOut);
    int mmapFlags = MAP_PRIVATE | MAP_ANONYMOUS;
    if (target >= 0x100000000ULL && (flags & 0x10 || flags & 0x20)) {
        mmapFlags |= MAP_FIXED;
    } else if (target < 0x100000000ULL) {
        target = AlignUp(g_virtCursor.fetch_add(len + 0x10000), 0x10000);
    }
    int hostProt = PROT_READ | PROT_WRITE;
    if (prot & 0x04) hostProt |= PROT_EXEC;

    void* mapped = ::mmap(reinterpret_cast<void*>(target), len, hostProt, mmapFlags, -1, 0);
    if (mapped == MAP_FAILED) {
        mapped = ::mmap(nullptr, len, hostProt, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (mapped == MAP_FAILED) return -12;
    }
    *addrInOut = mapped;
    RecordVma(reinterpret_cast<uintptr_t>(mapped), len, 0, prot, true, false, name ? name : "RE7_Flex");
    return 0;
}

int32_t ORBIS_ABI sceKernelMapFlexibleMemory(void** addrInOut, size_t len, int32_t prot, int32_t flags) {
    return sceKernelMapNamedFlexibleMemory(addrInOut, len, prot, flags, "RE7_Flex");
}

int32_t ORBIS_ABI sceKernelVirtualQuery(const void* addr, int32_t flags, SceKernelVirtualQueryInfo* info, size_t infoSize) {
    if (!info || infoSize < sizeof(SceKernelVirtualQueryInfo)) return -22;
    uintptr_t ptr = reinterpret_cast<uintptr_t>(addr);
    std::lock_guard<std::mutex> lk(g_vmaMutex);
    for (auto it = g_vmaList.rbegin(); it != g_vmaList.rend(); ++it) {
        if (ptr >= it->start && ptr < it->end) {
            std::memset(info, 0, sizeof(*info));
            info->start = reinterpret_cast<void*>(it->start);
            info->end = reinterpret_cast<void*>(it->end);
            info->offset = it->offset;
            info->protection = it->prot;
            info->memoryType = it->memType;
            info->isFlexibleMemory = it->isFlex ? 1 : 0;
            info->isDirectMemory = it->isDirect ? 1 : 0;
            info->isCommitted = 1;
            std::memcpy(info->name, it->name, 32);
            return 0;
        }
    }
    if (flags & 1) {
        std::memset(info, 0, sizeof(*info));
        uintptr_t base = ptr & ~0xFFFFULL;
        info->start = reinterpret_cast<void*>(base);
        info->end = reinterpret_cast<void*>(base + 0x10000);
        info->protection = 0x03;
        info->isFlexibleMemory = 1;
        info->isCommitted = 1;
        std::snprintf(info->name, 32, "RE7_Fallback");
        return 0;
    }
    return -13;
}

struct OrbisEqueue {
    std::mutex mtx;
    std::vector<SceKernelEvent> triggeredEvents;
};

int32_t ORBIS_ABI sceKernelCreateEqueue(OrbisEqueue** outEq, const char* name) {
    (void)name;
    if (!outEq) return -22;
    *outEq = new OrbisEqueue();
    return 0;
}

int32_t ORBIS_ABI sceKernelDeleteEqueue(OrbisEqueue* eq) {
    delete eq;
    return 0;
}

int32_t ORBIS_ABI sceKernelAddUserEvent(OrbisEqueue* eq, int32_t id) {
    (void)eq; (void)id;
    return 0;
}

int32_t ORBIS_ABI sceKernelTriggerUserEvent(OrbisEqueue* eq, int32_t id, void* udata) {
    if (!eq) return -22;
    std::lock_guard<std::mutex> lk(eq->mtx);
    SceKernelEvent ev{};
    ev.ident = static_cast<uintptr_t>(id);
    ev.filter = -6;
    ev.udata = udata;
    eq->triggeredEvents.push_back(ev);
    return 0;
}

int32_t ORBIS_ABI sceKernelWaitEqueue(OrbisEqueue* eq, SceKernelEvent* evOut, int32_t num, int32_t* outNum, uint32_t* timo) {
    if (!eq || !evOut || num <= 0) return -22;
    {
        std::lock_guard<std::mutex> lk(eq->mtx);
        if (!eq->triggeredEvents.empty()) {
            int32_t count = 0;
            while (!eq->triggeredEvents.empty() && count < num) {
                evOut[count++] = eq->triggeredEvents.front();
                eq->triggeredEvents.erase(eq->triggeredEvents.begin());
            }
            if (outNum) *outNum = count;
            return 0;
        }
    }
    ::usleep((timo && *timo < 2000) ? *timo : 2000);
    std::memset(&evOut[0], 0, sizeof(SceKernelEvent));
    evOut[0].ident = 1;
    evOut[0].filter = -1;
    if (outNum) *outNum = 1;
    return 0;
}

static std::string TranslateOrbisPath(const char* orbisPath) {
    if (!orbisPath) return "";
    std::string p(orbisPath);
    const char* gameDir = std::getenv("RE7_GAME_DIR");
    if (!gameDir) gameDir = "/sdcard/PS4/CUSA03962";
    const char* saveDir = std::getenv("RE7_SAVE_DIR");
    if (!saveDir) saveDir = "/sdcard/PS4/CUSA03962_save";

    if (p.rfind("/app0", 0) == 0) {
        return std::string(gameDir) + p.substr(5);
    } else if (p.rfind("/savedata0", 0) == 0 || p.rfind("/download0", 0) == 0) {
        ::mkdir(saveDir, 0777);
        return std::string(saveDir) + p.substr(10);
    } else if (p.rfind("/temp0", 0) == 0) {
        const char* tmp = "/data/local/tmp";
        return std::string(tmp) + p.substr(6);
    }
    return p;
}

int32_t ORBIS_ABI sceKernelOpen(const char* path, int32_t flags, uint16_t mode) {
    std::string hostPath = TranslateOrbisPath(path);
    int hostFlags = O_RDONLY;
    if (flags & 0x0001) hostFlags = O_WRONLY;
    if (flags & 0x0002) hostFlags = O_RDWR;
    if (flags & 0x0200) hostFlags |= O_CREAT;
    if (flags & 0x0400) hostFlags |= O_TRUNC;
    int fd = ::open(hostPath.c_str(), hostFlags, mode ? mode : 0666);
    return (fd < 0) ? -2 : fd;
}

// Pada ARM64 (__LP64__), off_t di Android NDK sudah 64-bit secara native
ssize_t ORBIS_ABI sceKernelPread(int32_t fd, void* buf, size_t nbytes, int64_t offset) {
    return ::pread(fd, buf, nbytes, static_cast<off_t>(offset));
}

ssize_t ORBIS_ABI sceKernelRead(int32_t fd, void* buf, size_t nbytes) {
    return ::read(fd, buf, nbytes);
}

int64_t ORBIS_ABI sceKernelLseek(int32_t fd, int64_t offset, int32_t whence) {
    return static_cast<int64_t>(::lseek(fd, static_cast<off_t>(offset), whence));
}

int32_t ORBIS_ABI scePlayGoInitialize(const void*) { return 0; }
int32_t ORBIS_ABI scePlayGoTerminate() { return 0; }
int32_t ORBIS_ABI scePlayGoOpen(int32_t* outHandle, const void*) {
    if (outHandle) *outHandle = 1;
    return 0;
}
int32_t ORBIS_ABI scePlayGoClose(int32_t) { return 0; }
int32_t ORBIS_ABI scePlayGoGetLocus(int32_t, const uint16_t*, uint32_t count, int8_t* outLoci) {
    if (outLoci && count > 0) std::memset(outLoci, 3, count);
    return 0;
}
int32_t ORBIS_ABI scePlayGoGetProgress(int32_t, const uint16_t*, uint32_t, ScePlayGoProgress* outProg) {
    if (outProg) {
        outProg->totalSize = 24ULL * 1024 * 1024 * 1024;
        outProg->progressSize = outProg->totalSize;
    }
    return 0;
}
int32_t ORBIS_ABI scePlayGoGetToDoList(int32_t, ScePlayGoToDo*, uint32_t, uint32_t* outCount) {
    if (outCount) *outCount = 0;
    return 0;
}

int32_t ORBIS_ABI sceSaveDataInitialize3(const void*) { return 0; }
int32_t ORBIS_ABI sceSaveDataMount2(const SceSaveDataMount2*, SceSaveDataMountResult* res) {
    if (res) {
        std::memset(res, 0, sizeof(*res));
        std::snprintf(res->mountPoint, sizeof(res->mountPoint), "/savedata0");
        res->mountStatus = 1;
    }
    const char* saveDir = std::getenv("RE7_SAVE_DIR");
    ::mkdir(saveDir ? saveDir : "/sdcard/PS4/CUSA03962_save", 0777);
    return 0;
}
int32_t ORBIS_ABI sceSaveDataUmount(const void*) { return 0; }

static std::atomic<bool> g_userLoginSent{false};
int32_t ORBIS_ABI sceUserServiceInitialize(const void*) { return 0; }
int32_t ORBIS_ABI sceUserServiceGetInitialUser(int32_t* userId) {
    if (userId) *userId = 1;
    return 0;
}
int32_t ORBIS_ABI sceUserServiceGetEvent(SceUserServiceEvent* ev) {
    if (!ev) return -22;
    bool expected = false;
    if (g_userLoginSent.compare_exchange_strong(expected, true)) {
        ev->eventType = 0;
        ev->userId = 1;
        return 0;
    }
    return -2146435065;
}

static ScePadData g_padState{};
static std::mutex g_padMtx;

void UpdateGamepadState(uint32_t buttons, uint8_t lx, uint8_t ly, uint8_t rx, uint8_t ry) {
    std::lock_guard<std::mutex> lk(g_padMtx);
    g_padState.buttons = buttons;
    g_padState.leftStickX = lx;
    g_padState.leftStickY = ly;
    g_padState.rightStickX = rx;
    g_padState.rightStickY = ry;
}

int32_t ORBIS_ABI scePadInit() {
    std::lock_guard<std::mutex> lk(g_padMtx);
    std::memset(&g_padState, 0, sizeof(g_padState));
    g_padState.leftStickX = 128; g_padState.leftStickY = 128;
    g_padState.rightStickX = 128; g_padState.rightStickY = 128;
    g_padState.orientation[3] = 1.0f;
    g_padState.connected = true;
    g_padState.connectedCount = 1;
    return 0;
}
int32_t ORBIS_ABI scePadOpen(int32_t, int32_t, int32_t, const void*) { return 1; }
int32_t ORBIS_ABI scePadReadState(int32_t, ScePadData* data) {
    if (!data) return -22;
    std::lock_guard<std::mutex> lk(g_padMtx);
    struct timespec ts;
    ::clock_gettime(CLOCK_MONOTONIC, &ts);
    g_padState.timestamp = static_cast<uint64_t>(ts.tv_sec) * 1000000ULL + (ts.tv_nsec / 1000ULL);
    *data = g_padState;
    return 0;
}
int32_t ORBIS_ABI scePadRead(int32_t h, ScePadData* data, int32_t count) {
    if (count <= 0 || !data) return 0;
    scePadReadState(h, &data[0]);
    return 1;
}

static std::atomic<uint32_t> g_ajmBatchId{1};
int32_t ORBIS_ABI sceAjmInitialize(int64_t, uint32_t* ctx) { if (ctx) *ctx = 1; return 0; }
int32_t ORBIS_ABI sceAjmModuleRegister(uint32_t, uint32_t, int64_t) { return 0; }
int32_t ORBIS_ABI sceAjmInstanceCreate(uint32_t, uint32_t, uint32_t, uint32_t* inst) {
    static std::atomic<uint32_t> id{10};
    if (inst) *inst = id.fetch_add(1);
    return 0;
}
int32_t ORBIS_ABI sceAjmBatchJobRunSplit(void* buf, size_t sz, const void*, size_t, uint32_t* batchId) {
    if (buf && sz > 0) std::memset(buf, 0, sz);
    if (batchId) *batchId = g_ajmBatchId.fetch_add(1);
    return 0;
}
int32_t ORBIS_ABI sceAjmBatchWait(uint32_t, uint32_t, uint32_t, void* res) {
    if (res) std::memset(res, 0, 64);
    return 0;
}

static uint32_t* g_computeReadPtrs[16] = {nullptr};
int32_t ORBIS_ABI sceGnmMapComputeQueue(uint32_t* vqOut, uint32_t pipeId, uint32_t qId, void*, uint32_t, void* readPtr) {
    uint32_t vq = (((pipeId & 3) << 2) | (qId & 3)) + 1;
    g_computeReadPtrs[vq & 15] = reinterpret_cast<uint32_t*>(readPtr);
    if (g_computeReadPtrs[vq & 15]) *g_computeReadPtrs[vq & 15] = 0;
    if (vqOut) *vqOut = vq;
    return 0;
}
int32_t ORBIS_ABI sceGnmDingDong(uint32_t vq, uint32_t nextOffsetDw) {
    if (g_computeReadPtrs[vq & 15]) *g_computeReadPtrs[vq & 15] = nextOffsetDw;
    return 0;
}
int32_t ORBIS_ABI sceGnmDingDongForWorkload(uint32_t vq, uint32_t nextOffsetDw, uint32_t) {
    return sceGnmDingDong(vq, nextOffsetDw);
}

static std::unordered_map<std::string, void*> g_symMap;
static std::unordered_map<std::string, void*> g_nidMap;
static std::unordered_map<uint64_t, void*>    g_hashMap;
static std::once_flag g_initOnce;

static void RegisterHook(const char* name, void* fnPtr) {
    g_symMap[name] = fnPtr;
    std::string nid;
    uint64_t hash64 = 0;
    ComputeOrbisNID(name, nid, hash64);
    g_nidMap[nid] = fnPtr;
    g_hashMap[hash64] = fnPtr;
}

#define REG_RE7(fn) RegisterHook(#fn, reinterpret_cast<void*>(&fn))

void InitializeRE7Subsystem() {
    std::call_once(g_initOnce, []() {
        REG_RE7(sceKernelGetDirectMemorySize);
        REG_RE7(sceKernelAllocateDirectMemory);
        REG_RE7(sceKernelMapDirectMemory);
        REG_RE7(sceKernelMapNamedDirectMemory);
        REG_RE7(sceKernelReserveVirtualRange);
        REG_RE7(sceKernelMapFlexibleMemory);
        REG_RE7(sceKernelMapNamedFlexibleMemory);
        REG_RE7(sceKernelVirtualQuery);
        REG_RE7(sceKernelCreateEqueue);
        REG_RE7(sceKernelDeleteEqueue);
        REG_RE7(sceKernelAddUserEvent);
        REG_RE7(sceKernelTriggerUserEvent);
        REG_RE7(sceKernelWaitEqueue);
        REG_RE7(sceKernelOpen);
        REG_RE7(sceKernelPread);
        REG_RE7(sceKernelRead);
        REG_RE7(sceKernelLseek);
        REG_RE7(scePlayGoInitialize);
        REG_RE7(scePlayGoTerminate);
        REG_RE7(scePlayGoOpen);
        REG_RE7(scePlayGoClose);
        REG_RE7(scePlayGoGetLocus);
        REG_RE7(scePlayGoGetProgress);
        REG_RE7(scePlayGoGetToDoList);
        REG_RE7(sceSaveDataInitialize3);
        REG_RE7(sceSaveDataMount2);
        REG_RE7(sceSaveDataUmount);
        REG_RE7(sceUserServiceInitialize);
        REG_RE7(sceUserServiceGetInitialUser);
        REG_RE7(sceUserServiceGetEvent);
        REG_RE7(scePadInit);
        REG_RE7(scePadOpen);
        REG_RE7(scePadReadState);
        REG_RE7(scePadRead);
        REG_RE7(sceAjmInitialize);
        REG_RE7(sceAjmModuleRegister);
        REG_RE7(sceAjmInstanceCreate);
        REG_RE7(sceAjmBatchJobRunSplit);
        REG_RE7(sceAjmBatchWait);
        REG_RE7(sceGnmMapComputeQueue);
        REG_RE7(sceGnmDingDong);
        REG_RE7(sceGnmDingDongForWorkload);
        scePadInit();
        std::fprintf(stderr, "[Orbis::RE7] Playable HLE Layer Initialized (%zu NIDs)\n", g_nidMap.size());
    });
}

void* ResolveBySymbolName(const char* name) {
    if (!name) return nullptr;
    InitializeRE7Subsystem();
    auto it = g_symMap.find(name);
    if (it != g_symMap.end()) return it->second;
    auto itNid = g_nidMap.find(name);
    if (itNid != g_nidMap.end()) return itNid->second;
    return nullptr;
}

void* ResolveByNID(const char* nid11) {
    if (!nid11) return nullptr;
    InitializeRE7Subsystem();
    std::string key(nid11, ::strnlen(nid11, 11));
    auto it = g_nidMap.find(key);
    return (it != g_nidMap.end()) ? it->second : nullptr;
}

void* ResolveByHash64(uint64_t nidHash) {
    InitializeRE7Subsystem();
    auto it = g_hashMap.find(nidHash);
    return (it != g_hashMap.end()) ? it->second : nullptr;
}

__attribute__((constructor))
static void AutoBootRE7() {
    InitializeRE7Subsystem();
}

} // namespace Orbis::RE7

extern "C" {
    void* re7_hle_resolve_symbol(const char* name) { return Orbis::RE7::ResolveBySymbolName(name); }
    void* re7_hle_resolve_nid(const char* nid)     { return Orbis::RE7::ResolveByNID(nid); }
    void* re7_hle_resolve_hash64(uint64_t hash)    { return Orbis::RE7::ResolveByHash64(hash); }
    void  re7_hle_update_pad(uint32_t buttons, uint8_t lx, uint8_t ly, uint8_t rx, uint8_t ry) {
        Orbis::RE7::UpdateGamepadState(buttons, lx, ly, rx, ry);
    }
}
