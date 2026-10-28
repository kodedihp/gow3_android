#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <sys/types.h>

#if defined(__x86_64__)
#define ORBIS_ABI __attribute__((sysv_abi))
#else
#define ORBIS_ABI
#endif

extern "C" {
    void* re7_hle_resolve_symbol(const char* name);
    void* re7_hle_resolve_nid(const char* nid);
    void* re7_hle_resolve_hash64(uint64_t hash);
    void  re7_hle_update_pad(uint32_t buttons, uint8_t lx, uint8_t ly, uint8_t rx, uint8_t ry);
}

namespace Orbis::RE7 {

struct SceKernelVirtualQueryInfo {
    void*    start;
    void*    end;
    off_t    offset;
    int32_t  protection;
    int32_t  memoryType;
    uint8_t  isFlexibleMemory : 1;
    uint8_t  isDirectMemory   : 1;
    uint8_t  isStack          : 1;
    uint8_t  isPooledMemory   : 1;
    uint8_t  isCommitted      : 1;
    char     name[32];
};

struct SceKernelEvent {
    uintptr_t ident;
    int16_t   filter;
    uint16_t  flags;
    uint32_t  fflags;
    intptr_t  data;
    void*     udata;
};

struct ScePlayGoProgress {
    uint64_t progressSize;
    uint64_t totalSize;
};

struct ScePlayGoToDo {
    uint16_t chunkId;
    uint8_t  locus;
    uint8_t  reserved;
};

struct SceSaveDataMount2 {
    int32_t     userId;
    int32_t     pad1;
    const char* dirName;
    uint64_t    blocks;
    uint32_t    mountMode;
    uint8_t     reserved[36];
};

struct SceSaveDataMountResult {
    char     mountPoint[16];
    uint64_t requiredBlocks;
    uint32_t unused;
    uint32_t mountStatus;
    uint8_t  reserved[28];
};

struct ScePadData {
    uint64_t timestamp;
    uint32_t buttons;
    uint8_t  leftStickX;
    uint8_t  leftStickY;
    uint8_t  rightStickX;
    uint8_t  rightStickY;
    uint8_t  analogButtons[2];
    uint16_t padding;
    float    orientation[4];
    float    acceleration[3];
    float    angularVelocity[3];
    uint8_t  touchData[24];
    bool     connected;
    uint64_t connectedCount;
    uint8_t  extensionUnitData[16];
};

struct SceUserServiceEvent {
    int32_t eventType;
    int32_t userId;
};

void  InitializeRE7Subsystem();
void* ResolveBySymbolName(const char* name);
void* ResolveByNID(const char* nid11);
void* ResolveByHash64(uint64_t nidHash);
void  UpdateGamepadState(uint32_t buttons, uint8_t lx, uint8_t ly, uint8_t rx, uint8_t ry);

// Overload otomatis agar kompatibel dengan berbagai tipe parameter resolver di gow3_android
inline void* ResolveAny(const char* s) { return re7_hle_resolve_symbol(s); }
inline void* ResolveAny(const std::string& s) { return re7_hle_resolve_symbol(s.c_str()); }
inline void* ResolveAny(std::string_view s) {
    std::string tmp(s);
    return re7_hle_resolve_symbol(tmp.c_str());
}

} // namespace Orbis::RE7
