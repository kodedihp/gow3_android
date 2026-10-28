# Rencana Eksekusi RE7 PS4 (CUSA03842)

Tujuan: menjalankan RE7 sampai menu dan gameplay single-player memakai pendekatan repo
(`src/runtime_*` + fallback `src/hle/re7_playable.cpp`), dengan referensi shadPS4,
AnyPS5, dynlib, dan port PS4 lain. Bukan port ulang seluruh emulator.

Peta import lengkap: `docs/re7_import_map.md` dan `docs/re7_import_map.csv`.

## Prinsip

- Import yang belum resolve hanya menjadi blocker saat benar-benar dipanggil.
- `WAJIB` = prioritas boot/single-player; `FITUR` = audio 3D; `OPSIONAL` = online,
  VR, dialog, sosial, media.
- Jangan memberi sukses palsu pada operasi yang punya handle/state. Init/term boleh
  sukses no-op; operasiNyata harus mengembalikan error yang jelas.
- Setiap batch harus bisa dibangun dan diuji lewat CI tanpa memasang APK.

## Status Awal

- 270 import missing pada dump lama.
- 15 sudah dipetakan/diimplementasi: heap trace, 5 event flag, 2 query memory,
  UUID, Neo mode, SaveData init/mount, Rtc tick, MsgDialog result, close.
- 18 P0 sudah diimplementasikan di working tree dan 18 NID-nya sudah dipetakan;
  belum dibuktikan build CI.
- Sisa: 34 Json2 P1 + 14 fitur Audio3d + 190 opsional.
- Perubahan wiring belum dibangun ulang lewat CI.

## Fase 0 - Baseline dan Loop Uji

Tujuan: memastikan perubahan wiring saat ini benar sebelum menambah fungsi.

1. Commit/push wiring yang ada (setelah disetujui), lalu jalankan CI Android.
2. Untuk loop import, jangan build APK penuh. Gunakan workflow
   `re7-runtime-fast` yang hanya menghasilkan `out/bionic` (probe + shared libs):
   - `tools/android/build_bionic_fast.sh` memakai ulang `libgow3cpu.so`,
     `libgow3gpu.so`, dan `libatrac9.a`; hanya `gow3-probe` yang dikompilasi ulang.
   - Cache dasar di-key oleh `cpu/**`, `gpu/**`, dan `third_party/`, bukan `src/**`,
     jadi perubahan `src/runtime*` atau `src/hle/re7_playable.cpp` tetap cache-hit.
   - Jangan pakai fast path bila `cpu/**`, `gpu/**`, atau `LibAtrac9/**` berubah;
     workflow otomatis jatuh ke full build untuk path tersebut.
3. Unduh artifact `re7-runtime-<sha>`, lalu jalankan baseline memakai
   `gow3-probe` + `libgow3cpu.so` + `libgow3gpu.so` + `libc++_shared.so` +
   `libSDL3.so`:
   - `--check-imports` untuk melihat daftar missing terbaru.
   - run biasa dengan `--timeout 60` dan `--app0`/`--user`.
4. Simpan log per iterasi (SHA, tanggal, import pertama, caller offset).
5. Kriteria keluar: heap trace tidak lagi menjadi STOP pertama.

Build APK penuh tetap dipakai hanya saat runtime sudah stabil atau saat menguji
perubahan launcher/packaging.

## Fase 1 - P0 Boot Contract

Tujuan: menyelesaikan 18 import P0. Tiga di antaranya hanya butuh mapping karena
implementasinya sudah ada.

### 1A. libkernel yang sudah ada, tinggal mapping

Tambahkan NID ke `src/import_names.inc`:

- `fgxnMeTNUtY#libkernel` -> `sceKernelGetProcessTimeCounter`
- `BNowx2l588E#libkernel` -> `sceKernelGetProcessTimeCounterFrequency`
- `uWyW3v98sU4#libkernel` -> `sceKernelCheckReachability`

Verifikasi: `--check-imports` tidak lagi melaporkan ketiganya.

### 1B. libkernel baru

Implementasi di `src/runtime_kernel.c` / `src/runtime_file.c`:

- `fcntl`: wrapper `fcntl(fd, cmd, arg)` dengan konversi errno guest.
- `sceKernelConvertUtcToLocaltime` dan `sceKernelConvertLocaltimeToUtc`:
  ikuti signature shadPS4/AnyPS5; gunakan `gmtime_r`/`localtime_r` + offset timezone.
- `sceKernelGetCpumode`: mulai dari 0 (base mode); opsional baca atribut 6/7 CPU
  dari SFO seperti shadPS4.
- `sceKernelGetGPI`: return 0 (non-devkit).
- `sceKernelSetFsstParam`: no-op sukses, log sekali.
- `sceKernelIsStack`: cek rentang guest stack yang terdaftar; isi `start/end`
  bila cocok. Untuk alamat mapped non-stack return 0 dengan start/end nol
  (shadPS4); alamat tidak mapped return EACCES.
- `sceKernelGetModuleInfoForUnwind`: isi `st_size`, nama `eboot.bin`, seg0.
  `.eh_frame` masih nol karena image belum mengekspos data unwind.
- `sceKernelInternalMemoryGetModuleSegmentInfo`: isi alamat + ukuran segmen
  executable utama dari `probe.c`.

Referensi:
- `gpu/shadps4/core/libraries/kernel/process.cpp` dan `time.cpp`.
- `AnyPS5/core/libs/prx/libkernel/Module/src/ModuleInfo.cpp`.
- `AnyPS5/core/libs/prx/libkernel/Time/Time.cpp`.

### 1C. SaveData, Zlib, DiscMap

- `sceSaveDataTransferringMount`:
  - Ikuti AnyPS5: bila tidak ada transfer, return `NOT_FOUND` dan mount point kosong.
  - Jangan mount direktori biasa secara diam-diam tanpa transfer state.
- `libSceZlib`:
  - `src/runtime_zlib.c` sudah dibuat dan ikut glob `runtime*.c`.
  - `Initialize`/`Finalize` mengelola satu sesi; `Inflate` sinkron tetapi tetap
    membuat request ID dan antrean completed; `WaitForDone`/`GetResult` sesuai
    kontrak AnyPS5.
  - `-lz` ditambahkan di build Android; Windows memakai miniz lewat CMake.
- `libSceDiscMap` NID `fJgP+wqifno`:
  - Petakan ke implementasi DiscMap yang sudah ada di `runtime_services.c`
    (`sceDiscMap_8A828CAEE7EDD5E9`) bila signature cocok.
  - Bila tidak, buat wrapper dengan return `DISC_MAP_NO_BITMAP`; game terinstal
    penuh sehingga tidak ada bitmap disc.

### Kriteria keluar Fase 1

- Semua 18 P0 memiliki mapping.
- `--check-imports` menunjukkan P0 tidak ada yang missing.
- Unit test event flag, time conversion, fcntl, dan zlib lulus.
- Build CI sukses.

## Fase 2 - Dynamic Boot Loop

Tujuan: melewati inisialisasi libc dan mencapai game entry.

1. Jalankan runtime dengan `--timeout 60`.
2. Setiap `STOP: first unsupported PS4 import`:
   - cocokkan NID dengan `docs/re7_import_map.csv`;
   - jika P0, implementasi benar;
   - jika P1, cek apakah benar-benar dipanggil; jika ya, implementasi minimal;
   - jika opsional, buat stub per family (lihat Fase 4), jangan sukses palsu.
3. Ulangi sampai log menunjukkan `Original guest entry instructions executed`
   atau game entry berjalan.
4. Setelah entry, uji `--cpu-only` dulu untuk memisahkan masalah CPU/import dari GPU.
5. Baru aktifkan GPU dan cek shader/pipeline.

Kriteria keluar:
- Tidak ada STOP import sebelum entry.
- Tidak ada crash TLS/relocation.
- Setidaknya logo/title screen tercapai.

## Fase 3 - Menu dan Gameplay Awal

- Fokus masalah setelah import: TLS, heap, file Fios, thread, sinkronisasi,
  dan inisialisasi GPU.
- Cek path `app0`/`user` dan patch Fios sudah benar untuk CUSA03842.
- Uji:
  - boot 3x berturut-turut;
  - masuk menu;
  - new game;
  - 10-30 menit gameplay;
  - save/load;
  - ganti setting grafis/restart.

Kriteria keluar:
- Menu utama stabil.
- New game masuk gameplay tanpa STOP import.
- Save dan load dasar bekerja.

## Fase 4 - Fitur dan Stub Opsional

### Audio3d

- 14 import. Implementasi setelah boot; prioritaskan `Initialize`, `PortOpen`,
  `PortPush`, `BedWrite`, `PortAdvance`, `Terminate`.
- Bila positional audio belum siap, hubungkan ke jalur audio stereo yang ada
  dan log degradasi; jangan blokir boot.

### Kebijakan opsional

- Dialog (Msg/Error/Common/Invitation/WebBrowser/Signin/Commerce/VrService):
  initialize/terminate sukses no-op; open/update menghasilkan status closed;
  get result menghasilkan canceled.
- VR/HMD/VrTracker/HmdSetup: init/term sukses; open/start mengembalikan
  `not connected`/`not supported`.
- Network/Http/Net/NetCtl: return offline/not connected; jangan menggantung.
- PSN (NpManager/Score/Tus/WebApi/Matching2): return not signed in.
- Screenshot/Recording/GameLiveStreaming/Remoteplay/SharePlay/ContentExport:
  return not supported, log sekali.
- AppContent/Ime/Camera/PadTracker/Voice: init/term no-op; operasi nyata error.

## Fase 5 - Validasi dan Rilis

- Jalankan `--check-imports`; target: tidak ada P0 missing.
- Jalankan 30 menit; pastikan tidak ada STOP import baru.
- Cek log: thread, memory, save, file, audio, GPU.
- Ukur performa awal (FPS/frame time) dan catat regresi.
- Perbarui `docs/re7_import_map.md` dan `CHANGELOG.md`.

## Definition of Done

- RE7 CUSA03842 boot sampai menu di Android.
- New game bisa dimainkan tanpa unresolved import.
- Save/load dasar berfungsi.
- Modal `--check-imports` bersih untuk P0.
- Tidak ada stub sukses palsu pada operasi yang membutuhkan handle/state.
- Semua perubahan dibangun lewat CI dan diuji dengan runtime hasil APK.

## Risiko Utama

- `Json2` 34 simbol C++ memakai ABI/mangled class; jangan implementasi asal.
  Tunda sampai terbukti dipanggil; bila dipanggil, adaptasi dari AnyPS5/nlohmann.
- `GetModuleInfoForUnwind` bisa memblokir exception/backtrace libc bila salah.
- Zlib async: game mungkin mengharapkan request ID dan status tertentu.
- Optional service yang dipanggil lebih awal dari perkiraan bisa menjadi blocker
  berikutnya; karena itu loop Fase 2 harus berbasis log, bukan asumsi.
