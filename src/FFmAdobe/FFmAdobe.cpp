/*******************************************************************/
/*                                                                 */
/*                      ADOBE CONFIDENTIAL                         */
/*                   _ _ _ _ _ _ _ _ _ _ _ _ _                     */
/*                                                                 */
/* Copyright 1992-2008 Adobe Systems Incorporated                  */
/* All Rights Reserved.                                            */
/*                                                                 */
/* NOTICE:  All information contained herein is, and remains the   */
/* property of Adobe Systems Incorporated and its suppliers, if    */
/* any.  The intellectual and technical concepts contained         */
/* herein are proprietary to Adobe Systems Incorporated and its    */
/* suppliers and may be covered by U.S. and Foreign Patents,       */
/* patents in process, and are protected by trade secret or        */
/* copyright law.  Dissemination of this information or            */
/* reproduction of this material is strictly forbidden unless      */
/* prior written permission is obtained from Adobe Systems         */
/* Incorporated.                                                   */
/*                                                                 */
/*******************************************************************/
/*
        Revision History

        Version		Change
   Engineer	Date
        =======		======
   ========	====== 1.0			created
   eks			10/11/1999 1.1			Added note on supporting
   multiple			bbb			6/14/2000 audioRate in
   compGetAudioIndFormat 1.2			Converted to C++, imposed coding
   guidelines	bbb			10/22/2001 fixed a supervision logic bug
   or two... 1.3			Updated for Adobe Premiere 6.5
   bbb			5/21/2002 1.4			Fixed work area export
   zal			1/20/2003 1.5			Updated for Adobe
   Premiere Pro 1.0			zal			2/28/2003 1.6
   Fixed row padding problem					zal
   8/11/2003 2.0			Added audio support for Premiere Pro,
   zal			1/6/2004 arbitrary audio sample rates, multi-channel
   audio, pixel aspect ratio, and fields; code cleanup 2.5
   Updated for Adobe Premiere Pro 2.0,			zal
   3/10/2006 code cleanup 3.0			High-bit video support (v410)
   zal			6/20/2006 4.0			Ported to new export API
   zal			3/3/2008
*/

#include "FFmAdobe.h"
#include "FFmAdobe_Params.h"

#include <windows.h>
#include <shlobj.h>
#include <string>
#include <vector>

DllExport PREMPLUGENTRY xSDKExport(csSDK_int32 selector,
                                   exportStdParms *stdParmsP, void *param1,
                                   void *param2) {
  prMALError result = exportReturn_Unsupported;

  switch (selector) {
  case exSelStartup:
    result =
        exSDKStartup(stdParmsP, reinterpret_cast<exExporterInfoRec *>(param1));
    break;

  case exSelBeginInstance:
    result = exSDKBeginInstance(
        stdParmsP, reinterpret_cast<exExporterInstanceRec *>(param1));
    break;

  case exSelEndInstance:
    result = exSDKEndInstance(
        stdParmsP, reinterpret_cast<exExporterInstanceRec *>(param1));
    break;

  case exSelGenerateDefaultParams:
    result = exSDKGenerateDefaultParams(
        stdParmsP, reinterpret_cast<exGenerateDefaultParamRec *>(param1));
    break;

  case exSelPostProcessParams:
    result = exSDKPostProcessParams(
        stdParmsP, reinterpret_cast<exPostProcessParamsRec *>(param1));
    break;

  case exSelGetParamSummary:
    result = exSDKGetParamSummary(
        stdParmsP, reinterpret_cast<exParamSummaryRec *>(param1));
    break;

  case exSelQueryOutputSettings:
    result = exSDKQueryOutputSettings(
        stdParmsP, reinterpret_cast<exQueryOutputSettingsRec *>(param1));
    break;

  case exSelQueryExportFileExtension:
    result = exSDKFileExtension(
        stdParmsP, reinterpret_cast<exQueryExportFileExtensionRec *>(param1));
    break;

  case exSelParamButton:
    result = exSDKParamButton(stdParmsP,
                              reinterpret_cast<exParamButtonRec *>(param1));
    break;

  case exSelValidateParamChanged:
    result = exSDKValidateParamChanged(
        stdParmsP, reinterpret_cast<exParamChangedRec *>(param1));
    break;

  case exSelValidateOutputSettings:
    result = malNoError;
    break;

  case exSelExport:
    result = exSDKExport(stdParmsP, reinterpret_cast<exDoExportRec *>(param1));
    break;
  }
  return result;
}

prMALError exSDKStartup(exportStdParms *stdParmsP,
                        exExporterInfoRec *infoRecP) {
  prMALError result = malNoError;

  infoRecP->fileType = '3FUI';
  copyConvertStringLiteralIntoUTF16(L"FFmAdobe (26w21d)", infoRecP->fileTypeName);
  copyConvertStringLiteralIntoUTF16(L"mp4", infoRecP->fileTypeDefaultExtension);

  infoRecP->classID = 'FFEX';
  infoRecP->exportReqIndex = 0; // Set this to exportReturn_IterateExporter to
                                // support multiple file types
  infoRecP->wantsNoProgressBar =
      kPrFalse; // Let Premiere provide the progress bar
  infoRecP->hideInUI = kPrFalse;
  infoRecP->doesNotSupportAudioOnly = kPrFalse; // Sure we support audio-only
  infoRecP->canConformToMatchParams = kPrTrue;
  infoRecP->canExportVideo = kPrTrue; // Can compile Video, enables the Video
                                      // checkbox in File > Export > Movie
  infoRecP->canExportAudio = kPrTrue; // Can compile Audio, enables the Audio
                                      // checkbox in File > Export > Movie

  // Tell Premiere which headers the exporter was compiled with
  infoRecP->interfaceVersion = EXPORTMOD_VERSION;

  return result;
}

prMALError exSDKBeginInstance(exportStdParms *stdParmsP,
                              exExporterInstanceRec *instanceRecP) {
  prMALError result = malNoError;
  SPErr spError = kSPNoError;
  ExportSettings *mySettings;
  PrSDKMemoryManagerSuite *memorySuite;
  csSDK_int32 exportSettingsSize = sizeof(ExportSettings);
  SPBasicSuite *spBasic = stdParmsP->getSPBasicSuite();
  if (spBasic != NULL) {
    spError = spBasic->AcquireSuite(
        kPrSDKMemoryManagerSuite, kPrSDKMemoryManagerSuiteVersion,
        const_cast<const void **>(reinterpret_cast<void **>(&memorySuite)));
    mySettings = reinterpret_cast<ExportSettings *>(
        memorySuite->NewPtrClear(exportSettingsSize));

    if (mySettings) {
      mySettings->spBasic = spBasic;
      mySettings->memorySuite = memorySuite;
      spError = spBasic->AcquireSuite(
          kPrSDKExportParamSuite, kPrSDKExportParamSuiteVersion,
          const_cast<const void **>(
              reinterpret_cast<void **>(&(mySettings->exportParamSuite))));
      spError = spBasic->AcquireSuite(
          kPrSDKExportProgressSuite, kPrSDKExportProgressSuiteVersion,
          const_cast<const void **>(
              reinterpret_cast<void **>(&(mySettings->exportProgressSuite))));
      spError = spBasic->AcquireSuite(
          kPrSDKExportFileSuite, kPrSDKExportFileSuiteVersion,
          const_cast<const void **>(
              reinterpret_cast<void **>(&(mySettings->exportFileSuite))));
      spError = spBasic->AcquireSuite(
          kPrSDKExportInfoSuite, kPrSDKExportInfoSuiteVersion,
          const_cast<const void **>(
              reinterpret_cast<void **>(&(mySettings->exportInfoSuite))));
      spError = spBasic->AcquireSuite(
          kPrSDKErrorSuite, kPrSDKErrorSuiteVersion3,
          const_cast<const void **>(
              reinterpret_cast<void **>(&(mySettings->errorSuite))));
      spError = spBasic->AcquireSuite(
          kPrSDKClipRenderSuite, kPrSDKClipRenderSuiteVersion,
          const_cast<const void **>(
              reinterpret_cast<void **>(&(mySettings->clipRenderSuite))));
      spError = spBasic->AcquireSuite(
          kPrSDKMarkerSuite, kPrSDKMarkerSuiteVersion,
          const_cast<const void **>(
              reinterpret_cast<void **>(&(mySettings->markerSuite))));
      spError = spBasic->AcquireSuite(
          kPrSDKPPixSuite, kPrSDKPPixSuiteVersion,
          const_cast<const void **>(
              reinterpret_cast<void **>(&(mySettings->ppixSuite))));
      spError = spBasic->AcquireSuite(
          kPrSDKSequenceAudioSuite, kPrSDKSequenceAudioSuiteVersion1,
          const_cast<const void **>(
              reinterpret_cast<void **>(&(mySettings->sequenceAudioSuite))));
      spError = spBasic->AcquireSuite(
          kPrSDKSequenceRenderSuite, kPrSDKSequenceRenderSuiteVersion,
          const_cast<const void **>(
              reinterpret_cast<void **>(&(mySettings->sequenceRenderSuite))));
      spError = spBasic->AcquireSuite(
          kPrSDKTimeSuite, kPrSDKTimeSuiteVersion,
          const_cast<const void **>(
              reinterpret_cast<void **>(&(mySettings->timeSuite))));
      spError = spBasic->AcquireSuite(
          kPrSDKWindowSuite, kPrSDKWindowSuiteVersion,
          const_cast<const void **>(
              reinterpret_cast<void **>(&(mySettings->windowSuite))));
      spError = spBasic->AcquireSuite(
          kPrSDKApplicationSettingsSuite, kPrSDKApplicationSettingsSuiteVersion,
          const_cast<const void **>(
              reinterpret_cast<void **>(&(mySettings->appSettingsSuite))));
    }

    mySettings->SDKFileRec.width = 0;
    mySettings->SDKFileRec.height = 0;

    instanceRecP->privateData = reinterpret_cast<void *>(mySettings);
  } else {
    result = exportReturn_ErrMemory;
  }
  return result;
}

prMALError exSDKEndInstance(exportStdParms *stdParmsP,
                            exExporterInstanceRec *instanceRecP) {
  prMALError result = malNoError;
  ExportSettings *lRec =
      reinterpret_cast<ExportSettings *>(instanceRecP->privateData);
  SPBasicSuite *spBasic = stdParmsP->getSPBasicSuite();
  PrSDKMemoryManagerSuite *memorySuite;
  if (spBasic != NULL && lRec != NULL) {
    if (lRec->exportParamSuite) {
      result = spBasic->ReleaseSuite(kPrSDKExportParamSuite,
                                     kPrSDKExportParamSuiteVersion);
    }
    if (lRec->exportProgressSuite) {
      result = spBasic->ReleaseSuite(kPrSDKExportProgressSuite,
                                     kPrSDKExportProgressSuiteVersion);
    }
    if (lRec->exportFileSuite) {
      result = spBasic->ReleaseSuite(kPrSDKExportFileSuite,
                                     kPrSDKExportFileSuiteVersion);
    }
    if (lRec->exportInfoSuite) {
      result = spBasic->ReleaseSuite(kPrSDKExportInfoSuite,
                                     kPrSDKExportInfoSuiteVersion);
    }
    if (lRec->errorSuite) {
      result =
          spBasic->ReleaseSuite(kPrSDKErrorSuite, kPrSDKErrorSuiteVersion3);
    }
    if (lRec->clipRenderSuite) {
      result = spBasic->ReleaseSuite(kPrSDKClipRenderSuite,
                                     kPrSDKClipRenderSuiteVersion);
    }
    if (lRec->markerSuite) {
      result =
          spBasic->ReleaseSuite(kPrSDKMarkerSuite, kPrSDKMarkerSuiteVersion);
    }
    if (lRec->ppixSuite) {
      result = spBasic->ReleaseSuite(kPrSDKPPixSuite, kPrSDKPPixSuiteVersion);
    }
    if (lRec->sequenceAudioSuite) {
      result = spBasic->ReleaseSuite(kPrSDKSequenceAudioSuite,
                                     kPrSDKSequenceAudioSuiteVersion1);
    }
    if (lRec->sequenceRenderSuite) {
      result = spBasic->ReleaseSuite(kPrSDKSequenceRenderSuite,
                                     kPrSDKSequenceRenderSuiteVersion);
    }
    if (lRec->timeSuite) {
      result = spBasic->ReleaseSuite(kPrSDKTimeSuite, kPrSDKTimeSuiteVersion);
    }
    if (lRec->windowSuite) {
      result =
          spBasic->ReleaseSuite(kPrSDKWindowSuite, kPrSDKWindowSuiteVersion);
    }
    if (lRec->appSettingsSuite) {
      result = spBasic->ReleaseSuite(kPrSDKApplicationSettingsSuite,
                                     kPrSDKApplicationSettingsSuiteVersion);
    }
    if (lRec->memorySuite) {
      memorySuite = lRec->memorySuite;
      memorySuite->PrDisposePtr(reinterpret_cast<PrMemoryPtr>(lRec));
      result = spBasic->ReleaseSuite(kPrSDKMemoryManagerSuite,
                                     kPrSDKMemoryManagerSuiteVersion);
    }
  }

  return result;
}

// This selector is necessary so that the AME UI can provide a preview
// The bitrate value is used to provide the Estimated File Size
prMALError exSDKQueryOutputSettings(exportStdParms *stdParmsP,
                                    exQueryOutputSettingsRec *outputSettingsP) {
  prMALError result = malNoError;
  csSDK_uint32 exID = outputSettingsP->exporterPluginID;
  exParamValues width, height, frameRate, pixelAspectRatio, fieldType, codec,
      sampleRate, channelType;
  ExportSettings *privateData =
      reinterpret_cast<ExportSettings *>(outputSettingsP->privateData);
  PrSDKExportParamSuite *paramSuite = privateData->exportParamSuite;
  csSDK_int32 mgroupIndex = 0;
  float fps = 0.0f;

  if (outputSettingsP->inExportVideo) {
    paramSuite->GetParamValue(exID, mgroupIndex, ADBEVideoWidth, &width);
    outputSettingsP->outVideoWidth = width.value.intValue;
    paramSuite->GetParamValue(exID, mgroupIndex, ADBEVideoHeight, &height);
    outputSettingsP->outVideoHeight = height.value.intValue;
    paramSuite->GetParamValue(exID, mgroupIndex, ADBEVideoFPS, &frameRate);
    outputSettingsP->outVideoFrameRate = frameRate.value.timeValue;
    paramSuite->GetParamValue(exID, mgroupIndex, ADBEVideoAspect,
                              &pixelAspectRatio);
    outputSettingsP->outVideoAspectNum =
        pixelAspectRatio.value.ratioValue.numerator;
    outputSettingsP->outVideoAspectDen =
        pixelAspectRatio.value.ratioValue.denominator;
    paramSuite->GetParamValue(exID, mgroupIndex, ADBEVideoFieldType,
                              &fieldType);
    outputSettingsP->outVideoFieldType = fieldType.value.intValue;
  }
  if (outputSettingsP->inExportAudio) {
    paramSuite->GetParamValue(exID, mgroupIndex, ADBEAudioRatePerSecond,
                              &sampleRate);
    outputSettingsP->outAudioSampleRate = sampleRate.value.floatValue;
    outputSettingsP->outAudioSampleType = kPrAudioSampleType_32BitFloat;
    paramSuite->GetParamValue(exID, mgroupIndex, ADBEAudioNumChannels,
                              &channelType);
    outputSettingsP->outAudioChannelType =
        (PrAudioChannelType)channelType.value.intValue;
  }

  // Calculate bitrate
  PrTime ticksPerSecond = 0;
  csSDK_uint32 videoBitrate = 0, audioBitrate = 0;
  if (outputSettingsP->inExportVideo) {
    privateData->timeSuite->GetTicksPerSecond(&ticksPerSecond);
    fps = static_cast<float>(ticksPerSecond) / frameRate.value.timeValue;
    paramSuite->GetParamValue(exID, mgroupIndex, ADBEVideoCodec, &codec);
    videoBitrate = static_cast<csSDK_uint32>(
        width.value.intValue * height.value.intValue *
        GetPixelFormatSize(codec.value.intValue) * fps);
  }
  if (outputSettingsP->inExportAudio) {
    audioBitrate = static_cast<csSDK_uint32>(
        sampleRate.value.floatValue * 4 *
        GetNumberOfAudioChannels(outputSettingsP->outAudioChannelType));
  }
  outputSettingsP->outBitratePerSecond = videoBitrate + audioBitrate;

  // New in CS5 - return outBitratePerSecond in kbps
  outputSettingsP->outBitratePerSecond =
      outputSettingsP->outBitratePerSecond * 8 / 1000;

  return result;
}

// If an exporter supports various file extensions, it would specify which one
// to use here
prMALError
exSDKFileExtension(exportStdParms *stdParmsP,
                   exQueryExportFileExtensionRec *exportFileExtensionRecP) {
  prMALError result = malNoError;
  copyConvertStringLiteralIntoUTF16(L"mp4",
                                    exportFileExtensionRecP->outFileExtension);
  return result;
}

prMALError RenderAndWriteAllVideo(exDoExportRec *exportInfoP, float progress,
                                  float videoProgress, PrTime *exportDuration) {
  // Stub 闂?not used in FFmpeg pipe mode
  return malNoError;
}

// Helper: wide path to std::wstring (no conversion needed, just copy)
static std::wstring PrPathToWString(const prUTF16Char* wstr, csSDK_int32 len) {
  if (!wstr || len <= 0) return L"";
  return std::wstring(wstr, wstr + len - 1);
}

// Watchdog thread: unblocks ConnectNamedPipe if FFmpeg exits early
struct WatchdogCtx {
  HANDLE       hProcess;
  std::wstring videoPipe;
  std::wstring audioPipe;
};
static DWORD WINAPI WatchdogThread(LPVOID param) {
  WatchdogCtx* ctx = reinterpret_cast<WatchdogCtx*>(param);
  WaitForSingleObject(ctx->hProcess, INFINITE);
  HANDLE hc = CreateFileW(ctx->videoPipe.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (hc != INVALID_HANDLE_VALUE) CloseHandle(hc);
  if (!ctx->audioPipe.empty()) {
    hc = CreateFileW(ctx->audioPipe.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hc != INVALID_HANDLE_VALUE) CloseHandle(hc);
  }
  delete ctx;
  return 0;
}

// ==== Pre-export configuration validation ====
// Runs a fast dummy encode (0.1s of lavfi input) to verify that the user's
// encoder args and filters are actually supported by the local FFmpeg build.
// Returns 0 if config is valid, non-zero if FFmpeg rejects it.
static int ValidateFFmpegConfig(
    const std::wstring& ffmpegExe,
    const std::wstring& vEncArgs,
    const std::wstring& aEncArgs,
    const std::wstring& vFilters,
    const std::wstring& aFilters,
    const std::wstring& extraInputArgs,
    const std::wstring& wW,
    const std::wstring& wH,
    bool hasAudio)
{
  // Build dummy command using lavfi sources to avoid needing real input
  std::wstring validCmd = ffmpegExe + L" -y";
  if (!extraInputArgs.empty()) validCmd += L" " + extraInputArgs;
  validCmd += L" -f lavfi -i color=c=black:s=" + wW + L"x" + wH + L":d=0.1";
  if (hasAudio)
    validCmd += L" -f lavfi -i anullsrc=r=48000:cl=stereo:d=0.1";

  // Apply video filter if present
  std::wstring vf = L"vflip";
  if (!vFilters.empty()) vf += L"," + vFilters;
  validCmd += L" -vf \"" + vf + L"\" " + vEncArgs;

  if (hasAudio) {
    std::wstring af;
    if (!aFilters.empty()) af = L" -af \"" + aFilters + L"\"";
    validCmd += af + L" " + aEncArgs;
  }
  validCmd += L" -f null -";  // discard output

  // Write validation log path
  wchar_t ad[MAX_PATH] = {};
  SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, ad);
  std::wstring valLogPath = std::wstring(ad) + L"\\FFmAdobe\\ffmpeg_validation.log";

  SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
  HANDLE hLog = CreateFileW(valLogPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
                            &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  PROCESS_INFORMATION pi; STARTUPINFOW si;
  ZeroMemory(&pi, sizeof(pi)); ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  if (hLog != INVALID_HANDLE_VALUE) {
    si.hStdError = hLog; si.hStdOutput = hLog;
    si.dwFlags |= STARTF_USESTDHANDLES;
  }

  std::vector<wchar_t> cmdBuf(validCmd.begin(), validCmd.end());
  cmdBuf.push_back(0);

  if (!CreateProcessW(NULL, cmdBuf.data(), NULL, NULL, TRUE,
                      CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
    if (hLog != INVALID_HANDLE_VALUE) CloseHandle(hLog);
    return -1;  // Can't even start FFmpeg
  }
  if (hLog != INVALID_HANDLE_VALUE) CloseHandle(hLog);

  WaitForSingleObject(pi.hProcess, 30000);  // 30s timeout
  DWORD exitCode = 1;
  GetExitCodeProcess(pi.hProcess, &exitCode);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  return (int)exitCode;
}

// Audio writer thread context
struct AudioThreadContext {
  ExportSettings*     settings;
  csSDK_uint32        exID;
  PrTime              startTime;
  PrTime              exportDuration;
  PrTime              ticksPerSecond;
  PrTime              ticksPerFrame;
  int                 channelTypeInt;
  float               audioSampleRate;
  int                 numAudioChannels;
  HANDLE              hAudioPipe;
  volatile bool       aborted;
};

static DWORD WINAPI AudioWriterThread(LPVOID lpParam) {
  AudioThreadContext* ctx = reinterpret_cast<AudioThreadContext*>(lpParam);

  csSDK_uint32 audioRenderID = 0;
  ctx->settings->sequenceAudioSuite->MakeAudioRenderer(
      ctx->exID, ctx->startTime,
      (PrAudioChannelType)ctx->channelTypeInt,
      kPrAudioSampleType_32BitFloat,
      ctx->audioSampleRate, &audioRenderID);

  csSDK_int32 maxBlip = 0;
  ctx->settings->sequenceAudioSuite->GetMaxBlip(
      audioRenderID, ctx->ticksPerFrame, &maxBlip);
  if (maxBlip <= 0) maxBlip = 4096;

  PrAudioSample totalSamples = (PrAudioSample)(
      (double)ctx->exportDuration / (double)ctx->ticksPerSecond * ctx->audioSampleRate);

  std::vector<float> interleavedBuf(maxBlip * ctx->numAudioChannels);
  std::vector<float*> chBufs(ctx->numAudioChannels);
  std::vector<std::vector<float>> chData(ctx->numAudioChannels, std::vector<float>(maxBlip));
  for (int c = 0; c < ctx->numAudioChannels; c++)
    chBufs[c] = chData[c].data();

  // Wait for FFmpeg to connect to our named pipe
  ConnectNamedPipe(ctx->hAudioPipe, NULL);

  PrAudioSample remaining = totalSamples;
  while (remaining > 0 && !ctx->aborted) {
    csSDK_uint32 toGet = (csSDK_uint32)(remaining < (PrAudioSample)maxBlip ? remaining : maxBlip);
    ctx->settings->sequenceAudioSuite->GetAudio(audioRenderID, toGet, chBufs.data(), kPrFalse);

    // Interleave channels
    for (csSDK_uint32 s = 0; s < toGet; s++)
      for (int c = 0; c < ctx->numAudioChannels; c++)
        interleavedBuf[s * ctx->numAudioChannels + c] = chBufs[c][s];

    DWORD written = 0;
    if (!WriteFile(ctx->hAudioPipe, interleavedBuf.data(),
                   toGet * ctx->numAudioChannels * sizeof(float), &written, NULL))
      break;
    remaining -= toGet;
  }

  FlushFileBuffers(ctx->hAudioPipe);
  DisconnectNamedPipe(ctx->hAudioPipe);

  ctx->settings->sequenceAudioSuite->ReleaseAudioRenderer(ctx->exID, audioRenderID);
  return 0;
}

// The main export function - named pipes to a single ffmpeg process, no intermediate files
prMALError exSDKExport(exportStdParms *stdParmsP, exDoExportRec *exportInfoP) {
  prMALError result = malNoError;
  csSDK_uint32 exID = exportInfoP->exporterPluginID;
  ExportSettings *mySettings =
      reinterpret_cast<ExportSettings *>(exportInfoP->privateData);
  exParamValues ticksPerFrame, width, height, sampleRate, channelType;

  mySettings->exportParamSuite->GetParamValue(exID, 0, ADBEVideoFPS, &ticksPerFrame);
  mySettings->exportParamSuite->GetParamValue(exID, 0, ADBEVideoWidth, &width);
  mySettings->exportParamSuite->GetParamValue(exID, 0, ADBEVideoHeight, &height);

  // ==== Get the REAL output path from Premiere (keep as wide string) ====
  std::wstring outputPathW;
  {
    csSDK_int32 pathLen = 0;
    mySettings->exportFileSuite->GetPlatformPath(exportInfoP->fileObject, &pathLen, NULL);
    if (pathLen > 0) {
      std::vector<prUTF16Char> pathBuf(pathLen + 1, 0);
      mySettings->exportFileSuite->GetPlatformPath(exportInfoP->fileObject, &pathLen, pathBuf.data());
      outputPathW = PrPathToWString(pathBuf.data(), pathLen);
    }
  }
  if (outputPathW.empty()) outputPathW = L"output.mp4";

  // ==== Debug: log the output path Premiere gave us ====
  {
    wchar_t ad[MAX_PATH] = {};
    SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, ad);
    std::wstring logPath = std::wstring(ad) + L"\\FFmAdobe\\export_debug.log";
    FILE* fp = _wfopen(logPath.c_str(), L"a, ccs=UTF-8");
    if (fp) {
      fwprintf(fp, L"[FFmAdobe] outputPathW = '%s'\n", outputPathW.c_str());
      fclose(fp);
    }
  }

  // ==== Calculate FPS ====
  PrTime ticksPerSecond;
  mySettings->timeSuite->GetTicksPerSecond(&ticksPerSecond);
  double fps = (double)ticksPerSecond / (double)ticksPerFrame.value.timeValue;

  // ==== Audio params ====
  float audioSampleRate = 48000.0f;
  int numAudioChannels = 2;
  int channelTypeInt = 1;
  bool hasAudio = (exportInfoP->exportAudio != 0);
  if (hasAudio) {
    mySettings->exportParamSuite->GetParamValue(exID, 0, ADBEAudioRatePerSecond, &sampleRate);
    audioSampleRate = sampleRate.value.floatValue;
    mySettings->exportParamSuite->GetParamValue(exID, 0, ADBEAudioNumChannels, &channelType);
    channelTypeInt = channelType.value.intValue;
    numAudioChannels = GetNumberOfAudioChannels(channelTypeInt);
  }

  // ==== Color range & color space params ====
  // Map our int enum values to FFmpeg argument strings
  exParamValues colorRangeVal, colorSpaceVal;
  mySettings->exportParamSuite->GetParamValue(exID, 0, FFMADOBE_COLOR_RANGE_ID, &colorRangeVal);
  mySettings->exportParamSuite->GetParamValue(exID, 0, FFMADOBE_COLOR_SPACE_ID, &colorSpaceVal);

  const wchar_t* colorRangeArgs[] = { L"tv", L"pc" };
  const wchar_t* colorSpaceArgs[] = { L"bt709", L"bt470bg", L"smpte170m", L"bt2020nc", L"srgb" };
  int crIdx = colorRangeVal.value.intValue;
  int csIdx = colorSpaceVal.value.intValue;
  if (crIdx < 0 || crIdx > 1)  crIdx = 0;
  if (csIdx < 0 || csIdx > 4)  csIdx = 0;
  std::wstring colorRangeArg  = std::wstring(L"-color_range ")  + colorRangeArgs[crIdx];
  std::wstring colorSpaceArg  = std::wstring(L"-colorspace ")   + colorSpaceArgs[csIdx];
  std::wstring colorTransferArg; // match transfer function to colorspace
  // Set -color_trc to the industry-standard value for each colorspace
  const wchar_t* transferArgs[] = { L"bt709", L"gamma28", L"smpte170m", L"arib-std-b67", L"iec61966-2-1" };
  colorTransferArg = std::wstring(L"-color_trc ") + transferArgs[csIdx];

  PrTime exportDuration = exportInfoP->endTime - exportInfoP->startTime;

  // ==== Create unique named pipe names using process ID ====
  DWORD pid = GetCurrentProcessId();
  std::wstring videoPipeName = L"\\\\.\\pipe\\ffmpeg_video_" + std::to_wstring(pid);
  std::wstring audioPipeName = L"\\\\.\\pipe\\ffmpeg_audio_" + std::to_wstring(pid);

  // ==== Create named pipe for video (server side) ====
  HANDLE hVideoPipe = CreateNamedPipeW(
      videoPipeName.c_str(),
      PIPE_ACCESS_OUTBOUND,
      PIPE_TYPE_BYTE | PIPE_WAIT,
      1, 1024 * 1024, 0, 0, NULL);
  if (hVideoPipe == INVALID_HANDLE_VALUE)
    return exportReturn_ErrOther;

  // ==== Create named pipe for audio (server side) ====
  HANDLE hAudioPipe = INVALID_HANDLE_VALUE;
  if (hasAudio) {
    hAudioPipe = CreateNamedPipeW(
        audioPipeName.c_str(),
        PIPE_ACCESS_OUTBOUND,
        PIPE_TYPE_BYTE | PIPE_WAIT,
        1, 512 * 1024, 0, 0, NULL);
    if (hAudioPipe == INVALID_HANDLE_VALUE) {
      CloseHandle(hVideoPipe);
      return exportReturn_ErrOther;
    }
  }

  // ==== Read simplified preset JSON ====
  std::wstring presetPath;
  {
    exParamValues ppv;
    mySettings->exportParamSuite->GetParamValue(exID, 0, FFMPEGFREEUI_PRESET_PATH_ID, &ppv);
    presetPath = std::wstring(reinterpret_cast<wchar_t*>(ppv.paramString));
    if (presetPath.empty()) {
      wchar_t ad[MAX_PATH];
      SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, ad);
      presetPath = std::wstring(ad) + L"\\FFmAdobe\\premiere_preset_simplified.json";
    }
  }

  std::wstring vEncArgs = L"-c:v libx264 -preset fast -crf 18 -pix_fmt yuv420p";
  std::wstring aEncArgs = L"-c:a aac -b:a 320k";
  std::wstring vFilters, aFilters, extraInputArgs;

  if (!presetPath.empty() && GetFileAttributesW(presetPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
    HANDLE hFile = CreateFileW(presetPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
      DWORD sz = GetFileSize(hFile, NULL);
      if (sz > 0 && sz < 256*1024) {
        std::string jsonBuf(sz, '\0'); DWORD rd=0;
        ReadFile(hFile, &jsonBuf[0], sz, &rd, NULL); jsonBuf.resize(rd);
        auto exJ = [&](const std::string& j, const std::string& k) -> std::string {
          std::string s = "\"" + k + "\""; size_t p = j.find(s);
          if(p==std::string::npos) return ""; p=j.find(':',p+s.size());
          if(p==std::string::npos) return ""; p=j.find('"',p+1);
          if(p==std::string::npos) return ""; size_t e=p+1;
          while(e<j.size()){if(j[e]=='\\'){e+=2;continue;}if(j[e]=='"')break;e++;}
          return j.substr(p+1,e-p-1);
        };
        auto u2w = [](const std::string& s) -> std::wstring {
          if(s.empty()) return L"";
          int l=MultiByteToWideChar(CP_UTF8,0,s.c_str(),(int)s.size(),NULL,0);
          std::wstring w(l,0); MultiByteToWideChar(CP_UTF8,0,s.c_str(),(int)s.size(),&w[0],l);
          return w;
        };
        std::string va=exJ(jsonBuf,"video_args"), aa=exJ(jsonBuf,"audio_args");
        std::string vf=exJ(jsonBuf,"video_filters"), af=exJ(jsonBuf,"audio_filters");
        std::string ei=exJ(jsonBuf,"extra_input_args");
        std::string co=exJ(jsonBuf,"container");  // e.g. ".mkv", ".mp4"
        if(!va.empty()) vEncArgs=u2w(va); if(!aa.empty()) aEncArgs=u2w(aa);
        vFilters=u2w(vf); aFilters=u2w(af); extraInputArgs=u2w(ei);

        // Replace output path extension with the container from the preset
        // Premiere always gives us the extension we registered (mp4), but user
        // may have chosen a different container in FFmpegFreeUI.
        if (!co.empty()) {
          std::wstring wCo = u2w(co);  // e.g. L".mkv"
          // Ensure it starts with a dot
          if (!wCo.empty() && wCo[0] != L'.') wCo = L"." + wCo;
          // Find last dot in outputPathW
          size_t dotPos = outputPathW.rfind(L'.');
          if (dotPos != std::wstring::npos)
            outputPathW = outputPathW.substr(0, dotPos) + wCo;
          else
            outputPathW += wCo;
        }
      }
      CloseHandle(hFile);
    }
  }

  std::wstring wW = std::to_wstring(width.value.intValue);
  std::wstring wH = std::to_wstring(height.value.intValue);
  char fpsBuf[32]; snprintf(fpsBuf, sizeof(fpsBuf), "%.6f", fps);
  std::string sFps(fpsBuf);
  std::wstring wFps(sFps.begin(), sFps.end());

  std::wstring vfArg = L"vflip";
  if (!vFilters.empty()) vfArg += L"," + vFilters;
  std::wstring afArg;
  if (!aFilters.empty()) afArg = L" -af \"" + aFilters + L"\"";

  // ==== Resolve ffmpeg.exe full path so AME can find it ====
  wchar_t ffmpegFullPath[MAX_PATH] = {};
  if (!SearchPathW(NULL, L"ffmpeg.exe", L".exe", MAX_PATH, ffmpegFullPath, NULL)) {
    // SearchPathW failed — try common Scoop/winget locations as fallback
    const wchar_t* fallbacks[] = {
      L"D:\\Scoop\\Applications\\ffmpeg\\current\\bin\\ffmpeg.exe",
      L"D:\\Scoop\\Applications\\ffmpeg-full\\current\\bin\\ffmpeg.exe",
      L"C:\\ffmpeg\\bin\\ffmpeg.exe",
      nullptr
    };
    bool found = false;
    for (int i = 0; fallbacks[i]; i++) {
      if (GetFileAttributesW(fallbacks[i]) != INVALID_FILE_ATTRIBUTES) {
        wcsncpy(ffmpegFullPath, fallbacks[i], MAX_PATH - 1);
        found = true;
        break;
      }
    }
    if (!found) {
      MessageBoxW(NULL,
        L"FFmpeg not found.\n\n"
        L"Please install FFmpeg and ensure it is in the system PATH.\n"
        L"Download from: https://ffmpeg.org/download.html",
        L"FFmAdobe Export Error", MB_OK | MB_ICONERROR);
      CloseHandle(hVideoPipe);
      if (hAudioPipe != INVALID_HANDLE_VALUE) CloseHandle(hAudioPipe);
      return exportReturn_ErrOther;
    }
  }
  std::wstring ffmpegExe = std::wstring(L"\"") + ffmpegFullPath + L"\"";

  // ==== Validate configuration before starting real export ====
  {
    int valResult = ValidateFFmpegConfig(ffmpegExe, vEncArgs, aEncArgs,
                                         vFilters, aFilters, extraInputArgs, wW, wH, hasAudio);
    if (valResult != 0) {
      // Read validation log for specific error message
      wchar_t ad2[MAX_PATH] = {};
      SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, ad2);
      std::wstring valLogPath = std::wstring(ad2) + L"\\FFmAdobe\\ffmpeg_validation.log";
      std::wstring detail;
      HANDLE hVLog = CreateFileW(valLogPath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      if (hVLog != INVALID_HANDLE_VALUE) {
        DWORD sz = GetFileSize(hVLog, NULL);
        if (sz > 0 && sz < 64 * 1024) {
          std::string buf(sz, '\0'); DWORD rd = 0;
          ReadFile(hVLog, &buf[0], sz, &rd, NULL);
          // Extract last meaningful error line
          size_t ePos = buf.rfind("Error");
          if (ePos == std::string::npos) ePos = buf.rfind("Unknown encoder");
          if (ePos != std::string::npos) {
            size_t eEnd = buf.find('\n', ePos);
            std::string errLine = buf.substr(ePos, eEnd != std::string::npos ? eEnd - ePos : 80);
            int wlen = MultiByteToWideChar(CP_UTF8, 0, errLine.c_str(), (int)errLine.size(), NULL, 0);
            detail.resize(wlen);
            MultiByteToWideChar(CP_UTF8, 0, errLine.c_str(), (int)errLine.size(), &detail[0], wlen);
          }
        }
        CloseHandle(hVLog);
      }
      std::wstring msg = L"FFmAdobe: Export configuration is not supported by your FFmpeg installation.\n\n";
      if (!detail.empty()) msg += L"FFmpeg reported:\n" + detail + L"\n\n";
      msg += L"Please open Configure and select an encoder supported by your FFmpeg version.\n";
      msg += L"(Log saved to: %APPDATA%\\FFmAdobe\\ffmpeg_validation.log)";
      MessageBoxW(NULL, msg.c_str(), L"FFmAdobe: Invalid Configuration", MB_OK | MB_ICONERROR);
      CloseHandle(hVideoPipe);
      if (hAudioPipe != INVALID_HANDLE_VALUE) CloseHandle(hAudioPipe);
      return 1;
    }
  }

  std::wstring cmd;
  // Color metadata flags (output-side, placed before output file)
  std::wstring colorFlags = L" " + colorRangeArg + L" " + colorSpaceArg + L" " + colorTransferArg;

  if (hasAudio) {
    std::wstring wSR = std::to_wstring((int)audioSampleRate);
    std::wstring wNCh = std::to_wstring(numAudioChannels);
    cmd = ffmpegExe + L" -y";
    if (!extraInputArgs.empty()) cmd += L" " + extraInputArgs;
    cmd += L" -f rawvideo -pix_fmt bgra -s " + wW + L"x" + wH + L" -r " + wFps
         + L" -i \\\\.\\pipe\\ffmpeg_video_" + std::to_wstring(pid)
         + L" -f f32le -ar " + wSR + L" -ac " + wNCh
         + L" -i \\\\.\\pipe\\ffmpeg_audio_" + std::to_wstring(pid)
         + L" -vf \"" + vfArg + L"\" " + vEncArgs + afArg + L" " + aEncArgs
         + colorFlags
         + L" \"" + outputPathW + L"\"";
  } else {
    cmd = ffmpegExe + L" -y";
    if (!extraInputArgs.empty()) cmd += L" " + extraInputArgs;
    cmd += L" -f rawvideo -pix_fmt bgra -s " + wW + L"x" + wH + L" -r " + wFps
         + L" -i \\\\.\\pipe\\ffmpeg_video_" + std::to_wstring(pid)
         + L" -vf \"" + vfArg + L"\" " + vEncArgs
         + colorFlags
         + L" \"" + outputPathW + L"\"";
  }
  // ==== Launch FFmpeg via CreateProcessW for Unicode path support ====
  // Redirect FFmpeg stderr to a log file for diagnostics
  wchar_t adPath[MAX_PATH] = {};
  SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, adPath);
  std::wstring ffmpegLogPath = std::wstring(adPath) + L"\\FFmAdobe\\ffmpeg_last.log";

  // Write full cmd to debug log (split to avoid fwprintf wchar_t limits)
  {
    FILE* fp = _wfopen((std::wstring(adPath) + L"\\FFmAdobe\\export_debug.log").c_str(), L"a, ccs=UTF-8");
    if (fp) {
      fwprintf(fp, L"[FFmAdobe] full cmd:\n");
      // Write in 200-char chunks
      const wchar_t* p = cmd.c_str();
      size_t remaining = cmd.size();
      while (remaining > 0) {
        size_t chunk = remaining > 200 ? 200 : remaining;
        fwprintf(fp, L"%.*s", (int)chunk, p);
        p += chunk; remaining -= chunk;
      }
      fwprintf(fp, L"\n");
      fclose(fp);
    }
  }

  PROCESS_INFORMATION piProcInfo;
  STARTUPINFOW siStartInfo;
  ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
  ZeroMemory(&siStartInfo, sizeof(STARTUPINFOW));
  siStartInfo.cb = sizeof(STARTUPINFOW);

  // Create stderr log file handle for FFmpeg
  SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
  HANDLE hFFmpegLog = CreateFileW(ffmpegLogPath.c_str(),
    GENERIC_WRITE, FILE_SHARE_READ, &sa,
    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFFmpegLog != INVALID_HANDLE_VALUE) {
    siStartInfo.hStdError  = hFFmpegLog;
    siStartInfo.hStdOutput = hFFmpegLog;
    siStartInfo.dwFlags   |= STARTF_USESTDHANDLES;
  }

  std::vector<wchar_t> cmdBuf(cmd.begin(), cmd.end());
  cmdBuf.push_back(0);

  if (!CreateProcessW(NULL, cmdBuf.data(), NULL, NULL, TRUE,
                      CREATE_NO_WINDOW, NULL, NULL, &siStartInfo, &piProcInfo)) {
    CloseHandle(hVideoPipe);
    if (hAudioPipe != INVALID_HANDLE_VALUE) CloseHandle(hAudioPipe);
    if (hFFmpegLog != INVALID_HANDLE_VALUE) CloseHandle(hFFmpegLog);
    return exportReturn_ErrOther;
  }
  if (hFFmpegLog != INVALID_HANDLE_VALUE) CloseHandle(hFFmpegLog);

  // ==== Watchdog: unblocks ConnectNamedPipe if FFmpeg exits early ====
  HANDLE hWatchdogThread = NULL;
  {
    WatchdogCtx* wdCtx = new WatchdogCtx();
    wdCtx->hProcess  = piProcInfo.hProcess;
    wdCtx->videoPipe = videoPipeName;
    wdCtx->audioPipe = hasAudio ? audioPipeName : L"";
    hWatchdogThread = CreateThread(NULL, 0, WatchdogThread, wdCtx, 0, NULL);
  }

  // ==== Start audio thread BEFORE connecting video pipe ====
  HANDLE hAudioThread = NULL;
  AudioThreadContext audioCtx = {};
  if (hasAudio) {
    audioCtx.settings        = mySettings;
    audioCtx.exID            = exID;
    audioCtx.startTime       = exportInfoP->startTime;
    audioCtx.exportDuration  = exportDuration;
    audioCtx.ticksPerSecond  = ticksPerSecond;
    audioCtx.ticksPerFrame   = ticksPerFrame.value.timeValue;
    audioCtx.channelTypeInt  = channelTypeInt;
    audioCtx.audioSampleRate = audioSampleRate;
    audioCtx.numAudioChannels= numAudioChannels;
    audioCtx.hAudioPipe      = hAudioPipe;
    audioCtx.aborted         = false;
    hAudioThread = CreateThread(NULL, 0, AudioWriterThread, &audioCtx, 0, NULL);
  }

  // ==== Connect video pipe (blocks until FFmpeg opens it or watchdog unblocks) ====
  ConnectNamedPipe(hVideoPipe, NULL);

  // ==== Render and pipe VIDEO frames ====
  mySettings->sequenceRenderSuite->MakeVideoRenderer(
      exID, &mySettings->videoRenderID, ticksPerFrame.value.timeValue);

  SequenceRender_ParamsRec renderParms;
  PrPixelFormat pixelFormats[] = {PrPixelFormat_BGRA_4444_8u};
  renderParms.inRequestedPixelFormatArray      = pixelFormats;
  renderParms.inRequestedPixelFormatArrayCount = 1;
  renderParms.inWidth                          = width.value.intValue;
  renderParms.inHeight                         = height.value.intValue;
  renderParms.inPixelAspectRatioNumerator      = 1;
  renderParms.inPixelAspectRatioDenominator    = 1;
  renderParms.inRenderQuality                  = kPrRenderQuality_Max;
  renderParms.inFieldType                      = prFieldsNone;
  renderParms.inDeinterlace                    = kPrFalse;
  renderParms.inDeinterlaceQuality             = kPrRenderQuality_Max;
  renderParms.inCompositeOnBlack               = kPrFalse;

  SequenceRender_GetFrameReturnRec getFrameReturn;
  float videoWeight = hasAudio ? 0.85f : 1.0f;

  for (PrTime videoTime = exportInfoP->startTime;
       videoTime <= (exportInfoP->endTime - ticksPerFrame.value.timeValue);
       videoTime += ticksPerFrame.value.timeValue)
  {
    result = mySettings->sequenceRenderSuite->RenderVideoFrame(
        mySettings->videoRenderID, videoTime, &renderParms,
        kRenderCacheType_None, &getFrameReturn);

    if (result == suiteError_NoError && getFrameReturn.outFrame) {
      char *pixelData;
      csSDK_int32 rowBytes;
      mySettings->ppixSuite->GetPixels(getFrameReturn.outFrame,
                                       PrPPixBufferAccess_ReadOnly, &pixelData);
      mySettings->ppixSuite->GetRowBytes(getFrameReturn.outFrame, &rowBytes);

      DWORD dwWritten = 0;
      // rowBytes may be negative (bottom-up), use abs for byte count
      csSDK_int32 absRowBytes = rowBytes < 0 ? -rowBytes : rowBytes;
      WriteFile(hVideoPipe, pixelData, absRowBytes * height.value.intValue, &dwWritten, NULL);
      mySettings->ppixSuite->Dispose(getFrameReturn.outFrame);
    }

    float progress = static_cast<float>(videoTime - exportInfoP->startTime) /
                     static_cast<float>(exportDuration) * videoWeight;
    result = mySettings->exportProgressSuite->UpdateProgressPercent(exID, progress);
    if (result == suiteError_ExporterSuspended)
      mySettings->exportProgressSuite->WaitForResume(exID);
    else if (result == exportReturn_Abort) {
      audioCtx.aborted = true;
      break;
    }
  }

  mySettings->sequenceRenderSuite->ReleaseVideoRenderer(exID, mySettings->videoRenderID);

  // Close video pipe to signal EOF to FFmpeg
  FlushFileBuffers(hVideoPipe);
  DisconnectNamedPipe(hVideoPipe);
  CloseHandle(hVideoPipe);

  // Wait for audio thread to finish
  if (hAudioThread) {
    WaitForSingleObject(hAudioThread, INFINITE);
    CloseHandle(hAudioThread);
    CloseHandle(hAudioPipe);
  }

  // Wait for FFmpeg to finish encoding
  mySettings->exportProgressSuite->UpdateProgressPercent(exID, 0.95f);
  WaitForSingleObject(piProcInfo.hProcess, INFINITE);
  mySettings->exportProgressSuite->UpdateProgressPercent(exID, 1.0f);

  if (hWatchdogThread) { WaitForSingleObject(hWatchdogThread, 5000); CloseHandle(hWatchdogThread); }

  // ==== Debug: log the ffmpeg command and exit code ====
  {
    DWORD exitCode = 0;
    GetExitCodeProcess(piProcInfo.hProcess, &exitCode);
    wchar_t ad[MAX_PATH] = {};
    SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, ad);
    std::wstring logPath = std::wstring(ad) + L"\\FFmAdobe\\export_debug.log";
    FILE* fp = _wfopen(logPath.c_str(), L"a, ccs=UTF-8");
    if (fp) {
      fwprintf(fp, L"[FFmAdobe] cmd = %s\n", cmd.c_str());
      fwprintf(fp, L"[FFmAdobe] ffmpeg exit code = %lu\n\n", exitCode);
      fclose(fp);
    }
  }

  CloseHandle(piProcInfo.hProcess);
  CloseHandle(piProcInfo.hThread);

  return (result == exportReturn_Abort) ? result : malNoError;
}
