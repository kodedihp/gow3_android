# START — Checkpoint RE7 Android

Terakhir diperbarui: **2026-10-28 05:55 UTC**

## Aturan sinkronisasi

- File ini adalah checkpoint status kerja di folder repo ini.
- Selalu samakan isi checkpoint dengan `git status`, commit terakhir, dan hasil CI terbaru.
- Jangan menandai fase selesai sebelum build/CI benar-benar membuktikannya.
- Jangan mengubah status “belum di-commit” menjadi “sudah” tanpa commit/push nyata.
- Sebelum melanjutkan pekerjaan, baca checkpoint ini lalu cek `git status` dan `git log -1`.

## Identitas kerja

- Repo: `/data/data/com.termux/files/home/gow3_android`
- Branch: `feat/re7-playable`
- Commit kode terakhir: `19d2e2e feat(re7): implement P0 boot imports and fast runtime CI` (sudah di-push ke `origin/feat/re7-playable`)
- Target: menjalankan **Resident Evil 7 PS4 CUSA03842** di Android.
- Pendekatan: runtime repo (`src/runtime_*`) + fallback HLE RE7 (`src/hle/re7_playable.cpp`), memakai referensi shadPS4, AnyPS5, dynlib, dan port PS4 lain.

## Status sinkronisasi saat ini

- Perubahan P0, checkpoint, peta import, dan workflow sudah **di-commit dan di-push** (`19d2e2e`).
- CI runtime `re7-runtime-fast` run `40640566995` sedang berjalan (trigger push `19d2e2e`).
- Submodule (`third_party/FEX`, `gpu/third_party/*`) tetap kotor di working tree dan **tidak** ikut di-commit; itu perubahan konten lokal, bukan pointer submodule.
- Perubahan `START.MD` ini di-commit terpisah sebagai checkpoint; status CI di dalamnya adalah snapshot 05:55 UTC dan boleh berubah setelah run selesai.

## Yang sudah selesai

- Rebrand package/namespace ke `com.resident.evil7.bionic`; source Kotlin/Java tetap `com.gow3.android`.
- CI APK lama pernah sukses dan menghasilkan APK signed, tetapi APK tersebut belum diuji menjalankan RE7.
- Dump/link `CUSA03842` berhasil: 891 import, 680.238 relocation, 60 module.
- Blocker runtime lama: `sceLibcHeapGetTraceInfo` (`NWtTN10cJzE#libSceLibcInternalExt`) di caller `libc.prx +0x5bd7e`.
- Implementasi heap trace + `REG_RE7` sudah ditambahkan di `src/hle/re7_playable.cpp`.
- `src/runtime_eventflag.c` sudah dibuat: 128 event flag, pattern 64-bit, wait AND/OR, clear-on-wake, timeout; unit test host lulus.
- `src/runtime_memory.c`: query available direct/flexible memory.
- `src/runtime_kernel.c`: UUID, Neo mode, process time counter, frequency, konversi waktu, IsStack, module info, GetCpumode/GetGPI/SetFsstParam.
- `src/runtime_file.c`: `fcntl` FreeBSD (DUPFD/GETFL/SETFL) memakai tabel FD guest.
- `src/runtime_savedata.c`: `sceSaveDataTransferringMount` -> NOT_FOUND.
- `src/runtime_services.c`: alias `sceDiscMap_7C980FFB0AA27E7A`.
- `src/runtime_zlib.c`: zlib init/inflate/wait/result, termasuk finalize.
- `src/import_names.inc`: 15 mapping lama + 18 mapping P0.
- `src/probe.c`: mempublikasikan main image/segmen dan stack guest main thread.
- `src/runtime.c`: resolver zlib + deklarasi C untuk HLE RE7 (header C++ tidak lagi di-include dari C).
- Wiring resolver event flag dan fallback RE7 sudah ada.
- `tools/android/build_bionic.sh`: compile C++ RE7 terpisah, link `-lc++_shared`.
- `android-launcher/app/src/main/cpp/CMakeLists.txt`: RE7 dihapus dari `gow3present`.
- Peta import lengkap: `docs/re7_import_map.md`, `docs/re7_import_map.csv`.
- Rencana eksekusi: `docs/re7_execution_plan.md`.
- Fast build sudah dibuat:
  - `tools/android/build_bionic_fast.sh`
  - `.github/workflows/re7-runtime.yml`
  - reuse `libgow3cpu.so`, `libgow3gpu.so`, `libatrac9.a`; hanya rebuild `gow3-probe`.
  - `bash -n`, `git diff --check`, dan syntax check API 30 lulus; build CI sedang berjalan.

## Status fase

### Fase 0 — Baseline / loop uji

- Fast-path build sudah dibuat dan sudah di-push.
- CI runtime pertama sudah berjalan: run `40640566995` (`in_progress` per 05:55 UTC).
- Artifact `re7-runtime-<sha>` belum diunduh.
- Baseline `--check-imports` dan run `--timeout 60` belum dijalankan untuk kode terbaru.

### Fase 1 — P0

**Implementasi selesai, sudah di-commit (`19d2e2e`); build CI sedang berjalan.** Syntax check API 30 lulus, host unit test zlib/fcntl/time/module-info lulus.

18 P0:

1. `sceKernelGetProcessTimeCounter` — mapping NID + implementasi.
2. `sceKernelGetProcessTimeCounterFrequency` — mapping NID + implementasi.
3. `sceKernelCheckReachability` — mapping NID + implementasi file.
4. `fcntl` — implementasi tabel FD guest + mapping.
5. `sceKernelConvertLocaltimeToUtc` — implementasi.
6. `sceKernelConvertUtcToLocaltime` — implementasi.
7. `sceKernelGetCpumode` — return 0.
8. `sceKernelGetGPI` — return 0.
9. `sceKernelGetModuleInfoForUnwind` — main image/seg0; `.eh_frame` masih nol.
10. `sceKernelInternalMemoryGetModuleSegmentInfo` — segmen executable main image.
11. `sceKernelIsStack` — registry guest stack + fallback host stack.
12. `sceKernelSetFsstParam` — no-op 0.
13. `sceSaveDataTransferringMount` — zero result + NOT_FOUND.
14. `sceZlibInitialize` — sekali init, work memory diabaikan dengan aman.
15. `sceZlibInflate` — sinkron + request ID/queue.
16. `sceZlibWaitForDone` — null timeout menunggu, timeout mikrodetik.
17. `sceZlibGetResult` — validasi output + lookup ID.
18. `sceDiscMap_7C980FFB0AA27E7A` (`fJgP+wqifno`) — alias DiscMap existing.

### Fase 2–5

- Fase 2 boot loop: belum.
- Fase 3 menu/gameplay: belum.
- Fase 4 Audio3d/opsional: belum.
- Fase 5 validasi/stabilitas: belum.

## Detail desain P0 yang sudah dikumpulkan

### fcntl

- Import: `8nY19bKoiZk#libkernel` -> `fcntl`.
- Gunakan table FD guest di `src/runtime_file.c`, bukan langsung FD host.
- Command guest FreeBSD:
  - `0 F_DUPFD`
  - `1 F_GETFD`
  - `2 F_SETFD`
  - `3 F_GETFL`
  - `4 F_SETFL`
  - `17 F_DUPFD_CLOEXEC`
- Guest status flags: nonblock `0x4`, append `0x8`, sync `0x80`.
- Kembalikan `-1` + `*runtime_errno()` untuk POSIX; `sceKernel*` memakai error `0x8002xxxx`.

### Konversi waktu kernel

- Signature:
  - `sceKernelConvertLocaltimeToUtc(int64 local, int64 reserved, int64 *utc, KernelTimesec *tz, int32 *dst)`
  - `sceKernelConvertUtcToLocaltime(int64 utc, int64 *local, KernelTimesec *st, int32 *dst)`
- `KernelTimesec`: `{ int64 t; uint32 west_sec; uint32 dst_sec; }` (16 byte).
- Ikuti pendekatan AnyPS5: `tm_gmtoff`, pisahkan standard offset dan daylight, cari daylight di sekitar bulan terkait.
- Return `0`; output null boleh dilewati.

### GetCpumode / GetGPI / SetFsstParam

- `sceKernelGetCpumode()` -> `0`.
- `sceKernelGetGPI()` -> `0` (uint64).
- `sceKernelSetFsstParam()` -> `0` no-op sukses.

### IsStack

- Guest stack didaftarkan oleh `runtime_thread.c`:
  - `GuestThread.guest_stack` + `guest_stack_size`
  - main thread memakai stack guest di `src/probe.c`
- Helper lookup sudah ada di `runtime_thread.c`, lalu `sceKernelIsStack(addr, &start, &end)`:
  - jika addr di dalam stack: isi start/end, return `0`;
  - jika addr mapped tapi bukan stack: return `0` dengan start/end nol (mengikuti shadPS4);
  - jika addr tidak mapped: return `EACCES` guest.
- Di Windows, stack JIT bisa tidak dialokasikan; host fallback hanya di Linux/Android.

### Module info

- Layout `ModuleInfoForUnwind` (256-byte name):
  `st_size`, `name[256]`, `eh_frame_hdr_addr`, `eh_frame_addr`, `eh_frame_size`, `seg0_addr`, `seg0_size`.
- `sceKernelInternalMemoryGetModuleSegmentInfo`:
  `{ uint64 address; uint64 size; }` — isi alamat/ukuran segmen executable/libkernel.
- Jangan mengembalikan pointer sampah; jika belum ada data, isi nol yang valid dan return sesuai referensi.

### SaveDataTransferringMount

- Signature: `(const SaveDataTransferringMount *mount, SaveDataMountResult *result)`.
- Tidak ada transfer PS4 di perangkat ini.
- Zero-kan `result`, return `ERR_NOT_FOUND` (`0x809F0008`), termasuk jika argumen null.
- Jangan mount direktori biasa diam-diam.

### Zlib

- Signature:
  - `sceZlibInitialize(const void *buffer, uint64 length)`
  - `sceZlibInflate(const void *src, uint32 srcLen, void *dst, uint32 dstLen, uint64 *requestId)`
  - `sceZlibWaitForDone(uint64 *requestId, uint32 *timeout)`
  - `sceZlibGetResult(uint64 requestId, uint32 *dstLen, int32 *status)`
- Error:
  - `NOT_FOUND 0x81120002`
  - `INVALID_ARGUMENT 0x81120016`
  - `NO_SPACE 0x8112001C`
  - `TIMED_OUT 0x81120027`
  - `NOT_INITIALIZED 0x81120032`
  - `ALREADY_INITIALIZED 0x81120033`
  - `FATAL 0x811200FF`
- Aturan penting:
  - init hanya sekali; finalize belum wajib untuk P0 tapi sebaiknya ditambahkan.
  - dst length harus kelipatan 2048 dan <= 65536; dst alignment 2048.
  - Inflate boleh sinkron; tetap buat request ID dan antrean completed.
  - `WaitForDone` null timeout = tunggu; timeout mikrodetik.
  - `GetResult` null output = INVALID_ARGUMENT; ID tidak ada = NOT_FOUND.
- Gunakan zlib host; tambah `-lz` pada build Android/Windows sesuai availability.

### DiscMap

- NID: `fJgP+wqifno` -> `sceDiscMap_7C980FFB0AA27E7A`.
- Implementasi DiscMap existing: `src/runtime_services.c` fungsi `sceDiscMap_8A828CAEE7EDD5E9`.
- Karena game terinstal penuh, return `DISC_MAP_NO_BITMAP` (`0x81100004`) bila signature cocok.
- Jika signature berbeda, tambahkan wrapper yang memakai konstanta yang sama.

## Fast build

```bash
# hanya probe + shared libs, bukan APK
bash tools/android/build_bionic_fast.sh
```

Trigger aktual: **push ke `feat/re7-playable`** yang menyentuh path workflow. `gh workflow run re7-runtime-fast.yml` gagal 404 karena workflow belum ada di default branch (`android`), jadi gunakan push atau commit pemicu.

Pantau run:

```bash
gh run list --branch feat/re7-playable --limit 3
gh run watch <run-id> --interval 20 --exit-status
```

Artifact: `re7-runtime-<sha>` berisi `out/bionic/bin/gow3-probe`, `kgsl-chip-id`, dan `*.so`.

Jangan gunakan fast path bila `cpu/**`, `gpu/**`, atau `third_party/LibAtrac9/**` berubah; workflow akan memilih full build untuk path tersebut.

## Batasan lingkungan

- Tidak ada NDK lokal.
- Tidak ada ADB/root.
- APK baru belum terpasang.
- Build/run harus lewat CI lalu artifact diuji.
- `clang --target=aarch64-linux-android30` dipakai untuk syntax check; API 24 gagal untuk `getrandom`/`memfd_create`.
- CI cache runtime harus di-key dengan benar agar perubahan `src/**` tidak melewatkan rebuild.

## Langkah berikutnya

1. Tunggu run `40640566995`; jika gagal, ambil log job dan perbaiki build.
2. Unduh artifact `re7-runtime-19d2e2e` setelah run sukses.
3. Jalankan `gow3-probe --check-imports`, catat import yang benar-benar dipanggil.
4. Jalankan `gow3-probe --timeout 60`, catat STOP pertama.
5. ~~Implementasi P0, mapping NID, `-lz`, syntax check, host test.~~ Sudah selesai (`ZTEST_OK`, `FTEST_OK`, `KTEST_OK`).
6. Lanjut loop Fase 2 sampai entry/menu; jangan mengejar 270 import sekaligus.
7. Commit/push checkpoint ini setelah status CI berubah.
