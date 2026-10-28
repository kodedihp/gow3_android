# RE7 CUSA03842 - Peta Import yang Belum Terresolve

Sumber: dump `eboot.bin` CUSA03842, output `--check-imports` runtime lama (270 import),
nama dari `src/import_names.inc`, shadPS4, dynlib.xml, port PS4 lain, dan verifikasi NID libkernel.

## Ringkasan

- Total import terdeteksi: **270**
- Wajib (core/single-player): **66**
- Fitur game (audio 3D): **14**
- Opsional (online/VR/dialog/sosial): **190**
- Sudah dipetakan/diimplementasi: **15**
- Belum diimplementasi: **255**

Catatan: status "wajib" adalah klasifikasi subsystem, bukan bukti bahwa fungsi
tersebut dipanggil sebelum main. Import hanya menjadi blocker saat benar-benar dipanggil.

## Sudah Selesai / Terpetakan

| NID | Library | Symbol | Status |
|---|---|---|---|
| `NWtTN10cJzE` | `libSceLibcInternalExt` | `sceLibcHeapGetTraceInfo` | SELESAI |
| `18B2NS1y9UU` | `libSceRtc` | `sceRtcGetCurrentTick` | SELESAI |
| `TywrFKCoLGY` | `libSceSaveData` | `sceSaveDataInitialize3` | SELESAI |
| `0z45PIH+SNI` | `libSceSaveData` | `sceSaveDataMount2` | SELESAI |
| `bY-PO6JhzhQ` | `libkernel` | `close` | SELESAI |
| `C0f7TJcbfac` | `libkernel` | `sceKernelAvailableDirectMemorySize` | SELESAI |
| `aNz11fnnzi4` | `libkernel` | `sceKernelAvailableFlexibleMemorySize` | SELESAI |
| `7uhBFWRAS60` | `libkernel` | `sceKernelClearEventFlag` | SELESAI |
| `BpFoboUJoZU` | `libkernel` | `sceKernelCreateEventFlag` | SELESAI |
| `8mql9OcQnd4` | `libkernel` | `sceKernelDeleteEventFlag` | SELESAI |
| `WslcK1FQcGI` | `libkernel` | `sceKernelIsNeoMode` | SELESAI |
| `IOnSvHzqu6A` | `libkernel` | `sceKernelSetEventFlag` | SELESAI |
| `Xjoosiw+XPI` | `libkernel` | `sceKernelUuidCreate` | SELESAI |
| `JTvBflhYazQ` | `libkernel` | `sceKernelWaitEventFlag` | SELESAI |
| `Lr8ovHH9l6A` | `libSceMsgDialog` | `sceMsgDialogGetResult` | SELESAI |

## P0 - Wajib, Prioritaskan

| NID | Library | Symbol | Status |
|---|---|---|---|
| `fJgP+wqifno` | `libSceDiscMap` | `sceDiscMap_7C980FFB0AA27E7A` | P0 |
| `WAzWTZm1H+I` | `libSceSaveData` | `sceSaveDataTransferringMount` | P0 |
| `2eDcGHC0YaM` | `libSceZlib` | `sceZlibGetResult` | P0 |
| `TLar1HULv1Q` | `libSceZlib` | `sceZlibInflate` | P0 |
| `m1YErdIXCp4` | `libSceZlib` | `sceZlibInitialize` | P0 |
| `uB8VlDD4e0s` | `libSceZlib` | `sceZlibWaitForDone` | P0 |
| `8nY19bKoiZk` | `libkernel` | `fcntl` | P0 |
| `uWyW3v98sU4` | `libkernel` | `sceKernelCheckReachability` | P0 |
| `0NTHN1NKONI` | `libkernel` | `sceKernelConvertLocaltimeToUtc` | P0 |
| `-o5uEDpN+oY` | `libkernel` | `sceKernelConvertUtcToLocaltime` | P0 |
| `VOx8NGmHXTs` | `libkernel` | `sceKernelGetCpumode` | P0 |
| `4oXYe9Xmk0Q` | `libkernel` | `sceKernelGetGPI` | P0 |
| `RpQJJVKTiFM` | `libkernel` | `sceKernelGetModuleInfoForUnwind` | P0 |
| `fgxnMeTNUtY` | `libkernel` | `sceKernelGetProcessTimeCounter` | P0 |
| `BNowx2l588E` | `libkernel` | `sceKernelGetProcessTimeCounterFrequency` | P0 |
| `-YTW+qXc3CQ` | `libkernel` | `sceKernelInternalMemoryGetModuleSegmentInfo` | P0 |
| `yDBwVAolDgg` | `libkernel` | `sceKernelIsStack` | P0 |
| `VjBtg5Btl94` | `libkernel` | `sceKernelSetFsstParam` | P0 |

## P1 - Wajib, Tunda Sampai Terbukti Dipanggil (Json2)

| NID | Library | Symbol | Status |
|---|---|---|---|
| `Cxwy7wHq4J0` | `libSceJson2` | `_ZN3sce4Json11Initializer10initializeEPKNS0_13InitParameterE` | P1 |
| `cK6bYHf-Q5E` | `libSceJson2` | `_ZN3sce4Json11InitializerC1Ev` | P1 |
| `RujUxbr3haM` | `libSceJson2` | `_ZN3sce4Json11InitializerD1Ev` | P1 |
| `-hJRce8wn1U` | `libSceJson2` | `_ZN3sce4Json12MemAllocatorC2Ev` | P1 |
| `OcAgPxcq5Vk` | `libSceJson2` | `_ZN3sce4Json12MemAllocatorD2Ev` | P1 |
| `zQtLRTqceMY` | `libSceJson2` | `_ZN3sce4Json5Array9push_backERKNS0_5ValueE` | P1 |
| `JP-PtKMiI1E` | `libSceJson2` | `_ZN3sce4Json5ArrayC1Ev` | P1 |
| `HJ8GpRT1aiw` | `libSceJson2` | `_ZN3sce4Json5ArrayD1Ev` | P1 |
| `Xbl-LYVFNEE` | `libSceJson2` | `_ZN3sce4Json5Value21setNullAccessCallBackEPFRKS1_NS0_9ValueTypeEPS2_PvES6_` | P1 |
| `IKQimvG9Wqs` | `libSceJson2` | `_ZN3sce4Json5Value3setENS0_9ValueTypeE` | P1 |
| `dFCphqnd+a4` | `libSceJson2` | `_ZN3sce4Json5Value3setERKNS0_6ObjectE` | P1 |
| `FIjXN2TkuTs` | `libSceJson2` | `_ZN3sce4Json5Value5clearEv` | P1 |
| `R7FDWtcN6f8` | `libSceJson2` | `_ZN3sce4Json5Value9serializeERNS0_6StringE` | P1 |
| `iZeYfOxtMRg` | `libSceJson2` | `_ZN3sce4Json5ValueC1ERKNS0_5ArrayE` | P1 |
| `3xUXnmUkXfo` | `libSceJson2` | `_ZN3sce4Json5ValueC1ERKNS0_6ObjectE` | P1 |
| `sZIoMRGO+jk` | `libSceJson2` | `_ZN3sce4Json5ValueC1ERKNS0_6StringE` | P1 |
| `0lLK8+kDqmE` | `libSceJson2` | `_ZN3sce4Json5ValueC1El` | P1 |
| `qBMjqyBn3OM` | `libSceJson2` | `_ZN3sce4Json5ValueC1Ev` | P1 |
| `WTtYf+cNnXI` | `libSceJson2` | `_ZN3sce4Json5ValueD1Ev` | P1 |
| `4zrm6VrgIAw` | `libSceJson2` | `_ZN3sce4Json5ValueaSERKS1_` | P1 |
| `OJPTonqdg0I` | `libSceJson2` | `_ZN3sce4Json6ObjectC1Ev` | P1 |
| `5JmzZt8twAo` | `libSceJson2` | `_ZN3sce4Json6ObjectD1Ev` | P1 |
| `ERuf9y0DY84` | `libSceJson2` | `_ZN3sce4Json6ObjectixERKNS0_6StringE` | P1 |
| `S5JxQnoGF3E` | `libSceJson2` | `_ZN3sce4Json6Parser5parseERNS0_5ValueEPKcm` | P1 |
| `9KUZFjI1IxA` | `libSceJson2` | `_ZN3sce4Json6StringC1EPKc` | P1 |
| `qSmqLXXCPas` | `libSceJson2` | `_ZN3sce4Json6StringC1Ev` | P1 |
| `cG1VE2HMl6c` | `libSceJson2` | `_ZN3sce4Json6StringD1Ev` | P1 |
| `DIxvoy7Ngvk` | `libSceJson2` | `_ZNK3sce4Json5Value10getIntegerEv` | P1 |
| `RBw+4NukeGQ` | `libSceJson2` | `_ZNK3sce4Json5Value5countEv` | P1 |
| `epJ6x2LV0kU` | `libSceJson2` | `_ZNK3sce4Json5Value9getStringEv` | P1 |
| `HwDt5lD9Bfo` | `libSceJson2` | `_ZNK3sce4Json5ValueixEPKc` | P1 |
| `XlWbvieLj2M` | `libSceJson2` | `_ZNK3sce4Json5ValueixEm` | P1 |
| `L1KAkYWml-M` | `libSceJson2` | `_ZNK3sce4Json6String5c_strEv` | P1 |
| `EUH+EmT-v9E` | `libSceJson2` | `_ZNK3sce4Json6String6lengthEv` | P1 |

## Fitur Game - Audio 3D

| NID | Library | Symbol | Status |
|---|---|---|---|
| `9tEwE0GV0qo` | `libSceAudio3d` | `sceAudio3dBedWrite` | P1 |
| `Im+jOoa5WAI` | `libSceAudio3d` | `sceAudio3dGetDefaultOpenParameters` | P1 |
| `UmCvjSmuZIw` | `libSceAudio3d` | `sceAudio3dInitialize` | P1 |
| `jO2tec4dJ2M` | `libSceAudio3d` | `sceAudio3dObjectReserve` | P1 |
| `4uyHN9q4ZeU` | `libSceAudio3d` | `sceAudio3dObjectSetAttributes` | P1 |
| `1HXxo-+1qCw` | `libSceAudio3d` | `sceAudio3dObjectUnreserve` | P1 |
| `lw0qrdSjZt8` | `libSceAudio3d` | `sceAudio3dPortAdvance` | P1 |
| `OyVqOeVNtSk` | `libSceAudio3d` | `sceAudio3dPortClose` | P1 |
| `ZOGrxWLgQzE` | `libSceAudio3d` | `sceAudio3dPortFlush` | P1 |
| `YaaDbDwKpFM` | `libSceAudio3d` | `sceAudio3dPortGetQueueLevel` | P1 |
| `XeDDK0xJWQA` | `libSceAudio3d` | `sceAudio3dPortOpen` | P1 |
| `VEVhZ9qd4ZY` | `libSceAudio3d` | `sceAudio3dPortPush` | P1 |
| `Yq9bfUQ0uJg` | `libSceAudio3d` | `sceAudio3dPortSetAttribute` | P1 |
| `WW1TS2iz5yc` | `libSceAudio3d` | `sceAudio3dTerminate` | P1 |

## Opsional - Boleh Stub/Error, Jangan Blokir Boot

Tabel per library:

| Library | Jumlah |
|---|---:|
| `libSceAppContent` | 7 |
| `libSceCamera` | 8 |
| `libSceCommonDialog` | 1 |
| `libSceContentExport` | 5 |
| `libSceErrorDialog` | 5 |
| `libSceGameLiveStreaming` | 3 |
| `libSceHmd` | 22 |
| `libSceHmdSetupDialog` | 7 |
| `libSceHttp` | 8 |
| `libSceIme` | 4 |
| `libSceInvitationDialog` | 6 |
| `libSceMsgDialog` | 2 |
| `libSceNet` | 3 |
| `libSceNetCtl` | 1 |
| `libSceNpCommerce` | 2 |
| `libSceNpManager` | 1 |
| `libSceNpMatching2` | 2 |
| `libSceNpScore` | 6 |
| `libSceNpTus` | 15 |
| `libSceNpWebApi` | 9 |
| `libScePadTracker` | 4 |
| `libSceRemoteplay` | 4 |
| `libSceScreenShot` | 3 |
| `libSceSharePlay` | 4 |
| `libSceSigninDialog` | 4 |
| `libSceSystemService` | 2 |
| `libSceVideoRecording` | 7 |
| `libSceVoice` | 7 |
| `libSceVrServiceDialog` | 7 |
| `libSceVrTracker` | 16 |
| `libSceWebBrowserDialog` | 6 |
| `libkernel` | 9 |

### libSceAppContent

| NID | Symbol |
|---|---|
| `VANhIWcqYak` | `sceAppContentAddcontMount` |
| `3rHWaV-1KC4` | `sceAppContentAddcontUnmount` |
| `CN7EbEV7MFU` | `sceAppContentDownloadDataFormat` |
| `Gl6w5i0JokY` | `sceAppContentDownloadDataGetAvailableSpaceKb` |
| `a5N7lAG0y2Q` | `sceAppContentTemporaryDataFormat` |
| `SaKib2Ug0yI` | `sceAppContentTemporaryDataGetAvailableSpaceKb` |
| `buYbeLOGWmA` | `sceAppContentTemporaryDataMount2` |

### libSceCamera

| NID | Symbol |
|---|---|
| `OMS9LlcrvBo` | `sceCameraClose` |
| `mxgMmR+1Kr0` | `sceCameraGetFrameData` |
| `p6n3Npi3YY4` | `sceCameraIsAttached` |
| `BHn83xrF92E` | `sceCameraOpen` |
| `VQ+5kAqsE2Q` | `sceCameraSetConfig` |
| `wpeyFwJ+UEI` | `sceCameraSetVideoSync` |
| `9EpRYMy7rHU` | `sceCameraStart` |
| `2G2C0nmd++M` | `sceCameraStop` |

### libSceCommonDialog

| NID | Symbol |
|---|---|
| `BQ3tey0JmQM` | `sceCommonDialogIsUsed` |

### libSceContentExport

| NID | Symbol |
|---|---|
| `tb3cZTCl8Ps` | `sceContentExportFinish` |
| `OWCJUmrWH1g` | `sceContentExportFromFile` |
| `FzEWeYnAFlI` | `sceContentExportInit` |
| `FCygF4Ec4so` | `sceContentExportStart` |
| `+KDWny9Y-6k` | `sceContentExportTerm` |

### libSceErrorDialog

| NID | Symbol |
|---|---|
| `ekXHb1kDBl0` | `sceErrorDialogClose` |
| `I88KChlynSs` | `sceErrorDialogInitialize` |
| `M2ZF-ClLhgY` | `sceErrorDialogOpen` |
| `9XAxK2PMwk8` | `sceErrorDialogTerminate` |
| `WWiGuh9XfgQ` | `sceErrorDialogUpdateStatus` |

### libSceGameLiveStreaming

| NID | Symbol |
|---|---|
| `dWM80AX39o4` | `sceGameLiveStreamingEnableLiveStreaming` |
| `kvYEw2lBndk` | `sceGameLiveStreamingInitialize` |
| `9yK6Fk8mKOQ` | `sceGameLiveStreamingTerminate` |

### libSceHmd

| NID | Symbol |
|---|---|
| `6biw1XHTSqQ` | `sceHmdClose` |
| `BWY-qKM5hxE` | `sceHmdGet2DEyeOffset` |
| `thDt9upZlp8` | `sceHmdGetDeviceInformation` |
| `NPQwYFqi0bs` | `sceHmdGetFieldOfView` |
| `s-J66ar9g50` | `sceHmdInitialize315` |
| `d2g5Ij7EUzo` | `sceHmdOpen` |
| `ZrV5YIqD09I` | `sceHmdReprojectionFinalize` |
| `OuygGEWkins` | `sceHmdReprojectionInitialize` |
| `TkcANcGM0s8` | `sceHmdReprojectionQueryGarlicBuffAlign` |
| `z0KtN1vqF2E` | `sceHmdReprojectionQueryGarlicBuffSize` |
| `IWybWbR-xvA` | `sceHmdReprojectionQueryOnionBuffAlign` |
| `kLUAkN6a1e8` | `sceHmdReprojectionQueryOnionBuffSize` |
| `6CRWGc-evO4` | `sceHmdReprojectionSetCallback` |
| `E+dPfjeQLHI` | `sceHmdReprojectionSetDisplayBuffers` |
| `LjdLRysHU6Y` | `sceHmdReprojectionSetOutputMinColor` |
| `knyIhlkpLgE` | `sceHmdReprojectionSetUserEventEnd` |
| `dntZTJ7meIU` | `sceHmdReprojectionStart` |
| `q3e8+nEguyE` | `sceHmdReprojectionStart2dVr` |
| `8gH1aLgty5I` | `sceHmdReprojectionStartMultilayer` |
| `vzMEkwBQciM` | `sceHmdReprojectionStop` |
| `iGNNpDDjcwo` | `sceHmdReprojectionUnsetDisplayBuffers` |
| `z-RMILqP6tE` | `sceHmdTerminate` |

### libSceHmdSetupDialog

| NID | Symbol |
|---|---|
| `nmHzU4Gh0xs` | `sceHmdSetupDialogClose` |
| `6lVRHMV5LY0` | `sceHmdSetupDialogGetResult` |
| `J9eBpW1udl4` | `sceHmdSetupDialogGetStatus` |
| `NB1Y2kA2jCY` | `sceHmdSetupDialogInitialize` |
| `NNgiV4T+akU` | `sceHmdSetupDialogOpen` |
| `+z4OJmFreZc` | `sceHmdSetupDialogTerminate` |
| `Ud7j3+RDIBg` | `sceHmdSetupDialogUpdateStatus` |

### libSceHttp

| NID | Symbol |
|---|---|
| `hvG6GfBMXg8` | `sceHttpAbortRequest` |
| `aCYPMSUIaP8` | `sceHttpGetAllResponseHeaders` |
| `0onIrKx9NIE` | `sceHttpGetLastErrno` |
| `hPTXo3bICzI` | `sceHttpParseResponseHeader` |
| `T-mGo9f3Pu4` | `sceHttpSetAutoRedirect` |
| `XNUoD2B9a6A` | `sceHttpSetCookieEnabled` |
| `JBN6N-EY+3M` | `sceHttpsGetSslError` |
| `DK+GoXCNT04` | `sceHttpsLoadCert` |

### libSceIme

| NID | Symbol |
|---|---|
| `PMVehSlfZ94` | `sceImeKeyboardClose` |
| `VkqLPArfFdc` | `sceImeKeyboardGetInfo` |
| `eaFXjfJv3xs` | `sceImeKeyboardOpen` |
| `-4GCfYdNF1s` | `sceImeUpdate` |

### libSceInvitationDialog

| NID | Symbol |
|---|---|
| `WWtCL5lzi7Y` | `sceInvitationDialogClose` |
| `8XKR6wa64iQ` | `sceInvitationDialogGetResult` |
| `XvA5KS56wcs` | `sceInvitationDialogInitialize` |
| `0zU0G+wiVLA` | `sceInvitationDialogOpen` |
| `B6HVJtDYxEE` | `sceInvitationDialogTerminate` |
| `9+g9iOq+7kg` | `sceInvitationDialogUpdateStatus` |

### libSceMsgDialog

| NID | Symbol |
|---|---|
| `Gc5k1qcK4fs` | `sceMsgDialogProgressBarInc` |

### libSceNet

| NID | Symbol |
|---|---|
| `6Oc0bLsIYe0` | `sceNetGetMacAddress` |
| `hLuXdjHnhiI` | `sceNetGetSockInfo` |
| `J5i3hiLJMPk` | `sceNetResolverGetError` |

### libSceNetCtl

| NID | Symbol |
|---|---|
| `0cBgduPRR+M` | `sceNetCtlGetResult` |

### libSceNpCommerce

| NID | Symbol |
|---|---|
| `NU3ckGHMFXo` | `sceNpCommerceDialogClose` |
| `r42bWcQbtZY` | `sceNpCommerceDialogGetResult` |

### libSceNpManager

| NID | Symbol |
|---|---|
| `Ghz9iWDUtC4` | `sceNpGetAccountCountry` |

### libSceNpMatching2

| NID | Symbol |
|---|---|
| `26vWrPAWJfM` | `sceNpMatching2GetRoomDataExternalList` |
| `uBESzz4CQws` | `sceNpMatching2RegisterRoomMessageCallback` |

### libSceNpScore

| NID | Symbol |
|---|---|
| `JjOFRVPdQWc` | `sceNpScoreGetGameDataAsync` |
| `Rd27dqUFZV8` | `sceNpScoreGetRankingByNpIdAsync` |
| `rShmqXHwoQE` | `sceNpScoreGetRankingByRangeAsync` |
| `m1DfNRstkSQ` | `sceNpScorePollAsync` |
| `1gL5PwYzrrw` | `sceNpScoreRecordGameDataAsync` |
| `ANJssPz3mY0` | `sceNpScoreRecordScoreAsync` |

### libSceNpTus

| NID | Symbol |
|---|---|
| `sRVb2Cf0GHg` | `sceNpTssCreateNpTitleCtx` |
| `DS2yu3Sjj1o` | `sceNpTssGetDataAsync` |
| `2eq1bMwgZYo` | `sceNpTusAbortRequest` |
| `Q2UmHdK04c8` | `sceNpTusAddAndGetVariableAsync` |
| `lliK9T6ylJg` | `sceNpTusAddAndGetVariableVUserAsync` |
| `BIkMmUfNKWM` | `sceNpTusCreateNpTitleCtx` |
| `3bh2aBvvmvM` | `sceNpTusCreateRequest` |
| `OCozl1ZtxRY` | `sceNpTusDeleteMultiSlotDataAsync` |
| `H3uq7x0sZOI` | `sceNpTusDeleteNpTitleCtx` |
| `CcIH40dYS88` | `sceNpTusDeleteRequest` |
| `uHtKS5V1T5k` | `sceNpTusGetDataAsync` |
| `t7b6dmpQNiI` | `sceNpTusPollAsync` |
| `G68xdfQuiyU` | `sceNpTusSetDataAsync` |
| `5J9GGMludxY` | `sceNpTusSetMultiSlotVariableAsync` |
| `CJAxTxQdwHM` | `sceNpTusSetMultiSlotVariableVUserAsync` |

### libSceNpWebApi

| NID | Symbol |
|---|---|
| `gVNNyxf-1Sg` | `sceNpWebApiCheckTimeout` |
| `M2BUB+DNEGE` | `sceNpWebApiCreateExtdPushEventFilter` |
| `79M-JqvvGo0` | `sceNpWebApiCreateHandle` |
| `pfaJtb7SQ80` | `sceNpWebApiDeleteExtdPushEventFilter` |
| `5Mn7TYwpl30` | `sceNpWebApiDeleteHandle` |
| `vrM02A5Gy1M` | `sceNpWebApiRegisterExtdPushEventCallback` |
| `KjNeZ-29ysQ` | `sceNpWebApiSendRequest2` |
| `qWcbJkBj1Lg` | `sceNpWebApiSetRequestTimeout` |
| `PqCY25FMzPs` | `sceNpWebApiUnregisterExtdPushEventCallback` |

### libScePadTracker

| NID | Symbol |
|---|---|
| `8bK-4Vxjr8E` | `scePadTrackerGetWorkingMemorySize` |
| `G1-RyPlIrIY` | `scePadTrackerInit` |
| `PqlxOq+k02o` | `scePadTrackerReadState` |
| `cri6g3qvCtc` | `scePadTrackerUpdate` |

### libSceRemoteplay

| NID | Symbol |
|---|---|
| `xQeIryTX7dY` | `sceRemoteplayApprove` |
| `k1SwgkMSOM8` | `sceRemoteplayInitialize` |
| `mrNh78tBpmg` | `sceRemoteplayProhibit` |
| `BOwybKVa3Do` | `sceRemoteplayTerminate` |

### libSceScreenShot

| NID | Symbol |
|---|---|
| `tIYf0W5VTi8` | `sceScreenShotDisable` |
| `2xxUtuC-RzE` | `sceScreenShotEnable` |
| `73WQ4Jj0nJI` | `sceScreenShotSetOverlayImageWithOrigin` |

### libSceSharePlay

| NID | Symbol |
|---|---|
| `OOrLKB0bSDs` | `sceSharePlayGetCurrentConnectionInfo` |
| `isruqthpYcw` | `sceSharePlayInitialize` |
| `co2NCj--pnc` | `sceSharePlaySetProhibition` |
| `UaLjloJinow` | `sceSharePlayTerminate` |

### libSceSigninDialog

| NID | Symbol |
|---|---|
| `mlYGfmqE3fQ` | `sceSigninDialogInitialize` |
| `JlpJVoRWv7U` | `sceSigninDialogOpen` |
| `LXlmS6PvJdU` | `sceSigninDialogTerminate` |
| `Bw31liTFT3A` | `sceSigninDialogUpdateStatus` |

### libSceSystemService

| NID | Symbol |
|---|---|
| `JoBqSQt1yyA` | `sceSystemServiceLoadExec` |
| `3s8cHiCBKBE` | `sceSystemServiceReportAbnormalTermination` |

### libSceVideoRecording

| NID | Symbol |
|---|---|
| `KHvkPQJDMLk` | `sceVideoRecordingClose` |
| `fZJQzFK4Gv4` | `sceVideoRecordingGetStatus` |
| `s28dalBwp2g` | `sceVideoRecordingOpen2` |
| `bGYkY6q3bIw` | `sceVideoRecordingQueryMemSize2` |
| `Fc8qxlKINYQ` | `sceVideoRecordingSetInfo` |
| `tWoe9IlGAhs` | `sceVideoRecordingStart` |
| `OOFxrMY+mfI` | `sceVideoRecordingStop` |

### libSceVoice

| NID | Symbol |
|---|---|
| `Pc4z1QjForU` | `sceVoiceGetMuteFlag` |
| `elcxZTEfHZM` | `sceVoiceGetPortAttr` |
| `jjkCjneOYSs` | `sceVoiceGetVolume` |
| `x0slGBQW+wY` | `sceVoicePausePort` |
| `gAgN+HkiEzY` | `sceVoiceResumePort` |
| `gwUynkEgNFY` | `sceVoiceSetMuteFlag` |
| `QBFoAIjJoXQ` | `sceVoiceSetVolume` |

### libSceVrServiceDialog

| NID | Symbol |
|---|---|
| `hBH2ABP7IeY` | `sceVrServiceDialogClose` |
| `cYnBkgm8I0c` | `sceVrServiceDialogGetResult` |
| `kUavKmsczkY` | `sceVrServiceDialogGetStatus` |
| `hYFXG8FWThI` | `sceVrServiceDialogInitialize` |
| `60-cjn5Dn0Q` | `sceVrServiceDialogOpen` |
| `M4xKWUytNMo` | `sceVrServiceDialogTerminate` |
| `RmRtBJpoHlA` | `sceVrServiceDialogUpdateStatus` |

### libSceVrTracker

| NID | Symbol |
|---|---|
| `5IFOAYv-62g` | `sceVrTrackerCpuProcess` |
| `zvyKP0Z3UvU` | `sceVrTrackerGetPlayAreaWarningInfo` |
| `76OBvrrQXUc` | `sceVrTrackerGetResult` |
| `XoeWzXlrnMw` | `sceVrTrackerGetTime` |
| `TVegDMLaBB8` | `sceVrTrackerGpuSubmit` |
| `gkGuO9dd57M` | `sceVrTrackerGpuWait` |
| `ARhgpXvwoR0` | `sceVrTrackerGpuWaitAndCpuProcess` |
| `QkRl7pART9M` | `sceVrTrackerInit` |
| `K7yhYrsIBPc` | `sceVrTrackerQueryMemory` |
| `EUCaQtXXYNI` | `sceVrTrackerRecalibrate` |
| `sIh8GwcevaQ` | `sceVrTrackerRegisterDevice` |
| `E0P0sN-wy+4` | `sceVrTrackerResetOrientationRelative` |
| `qBjnR0HtMYI` | `sceVrTrackerSetDurationUntilStatusNotTracking` |
| `IBv4P3q1pQ0` | `sceVrTrackerTerm` |
| `Q8skQqEwn5c` | `sceVrTrackerUnregisterDevice` |
| `9fvHMUbsom4` | `sceVrTrackerUpdateMotionSensorData` |

### libSceWebBrowserDialog

| NID | Symbol |
|---|---|
| `PSK+Eik919Q` | `sceWebBrowserDialogClose` |
| `vCaW0fgVQmc` | `sceWebBrowserDialogGetResult` |
| `jqb7HntFQFc` | `sceWebBrowserDialogInitialize` |
| `FraP7debcdg` | `sceWebBrowserDialogOpen` |
| `ocHtyBwHfys` | `sceWebBrowserDialogTerminate` |
| `h1dR-t5ISgg` | `sceWebBrowserDialogUpdateStatus` |

### libkernel

| NID | Symbol |
|---|---|
| `3e+4Iv7IJ8U` | `accept` |
| `KuOmgKoqCdY` | `bind` |
| `XVL8So3QJUk` | `connect` |
| `pxnCmagrtao` | `listen` |
| `Ez8xjo9UF4E` | `recv` |
| `T8fER+tIGgk` | `select` |
| `fZOeZIOEmLw` | `send` |
| `fFxGkxF2bVo` | `setsockopt` |
| `TU-d9PfIHPM` | `socket` |

## Urutan Implementasi yang Disarankan

1. P0 libkernel: process time counter/frequency, konversi waktu, UUID/Neo sudah,
   event flag sudah, memory query sudah; lanjut `GetGPI`, `GetCpumode`, `IsStack`,
   `SetFsstParam`, `CheckReachability`, `GetModuleInfoForUnwind`,
   `InternalMemoryGetModuleSegmentInfo`, `fcntl`.
2. P0 SaveData/Zlib/DiscMap: `sceSaveDataTransferringMount`, 4 fungsi Zlib, DiscMap.
3. Jalankan ulang runtime; baru implementasi Json2 jika benar-benar terpanggil.
4. Fitur Audio3d setelah boot; opsional cukup stub/error yang jelas.

## File

- `docs/re7_import_map.csv` - data lengkap 270 baris (NID, library, symbol, kategori, prioritas).
