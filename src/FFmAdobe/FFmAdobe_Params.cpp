#include "FFmAdobe_Params.h"
#include <windows.h>
#include <shlobj.h>
#include <commdlg.h>
#include <string>
#include <vector>

// ===========================================================
// exSDKGenerateDefaultParams
// ===========================================================
prMALError exSDKGenerateDefaultParams(exportStdParms* stdParms, exGenerateDefaultParamRec* rec)
{
    prMALError result = malNoError;
    ExportSettings* lRec = reinterpret_cast<ExportSettings*>(rec->privateData);
    PrSDKExportParamSuite* ps = lRec->exportParamSuite;
    PrSDKExportInfoSuite*  is = lRec->exportInfoSuite;
    PrSDKTimeSuite*        ts = lRec->timeSuite;
    csSDK_uint32 exID = rec->exporterPluginID;
    csSDK_int32 mg = 0;
    prUTF16Char tmp[256];

    PrParam seqW={}, seqH={}, seqFR={}, seqPARNum={}, seqPARDen={}, seqField={}, seqCh={}, seqSR={};
    if (is) {
        is->GetExportSourceInfo(exID, kExportInfo_VideoWidth,            &seqW);
        is->GetExportSourceInfo(exID, kExportInfo_VideoHeight,           &seqH);
        is->GetExportSourceInfo(exID, kExportInfo_VideoFrameRate,        &seqFR);
        is->GetExportSourceInfo(exID, kExportInfo_PixelAspectNumerator,  &seqPARNum);
        is->GetExportSourceInfo(exID, kExportInfo_PixelAspectDenominator,&seqPARDen);
        is->GetExportSourceInfo(exID, kExportInfo_VideoFieldType,        &seqField);
        is->GetExportSourceInfo(exID, kExportInfo_AudioChannelsType,     &seqCh);
        is->GetExportSourceInfo(exID, kExportInfo_AudioSampleRate,       &seqSR);
        if (seqW.mInt32 == 0) seqW.mInt32 = 1920;
        if (seqH.mInt32 == 0) seqH.mInt32 = 1080;
        lRec->sourceFieldType = seqField.mInt32;
    }
    if (!ps) return result;

    ps->AddMultiGroup(exID, &mg);

    // ============ FFmpegFreeUI TAB (own collapsible section) ============
    copyConvertStringLiteralIntoUTF16(FFMPEGFREEUI_GROUP_LABEL, tmp);
    ps->AddParamGroup(exID, mg, ADBETopParamGroup, FFMPEGFREEUI_TAB_ID, tmp, kPrFalse, kPrFalse, kPrFalse);

    // Inner group inside FFmpegFreeUI tab
    copyConvertStringLiteralIntoUTF16(FFMPEGFREEUI_GROUP_LABEL, tmp);
    ps->AddParamGroup(exID, mg, FFMPEGFREEUI_TAB_ID, FFMPEGFREEUI_INNER_GROUP_ID, tmp, kPrFalse, kPrFalse, kPrFalse);

    // Configure button
    {
        exNewParamInfo p; exParamValues v;
        safeStrCpy(p.identifier, 256, FFMPEGFREEUI_CONFIGURE_BTN);
        p.paramType = exParamType_button; p.flags = exParamFlag_none;
        v.disabled = kPrFalse; v.hidden = kPrFalse;
        v.arbData = 0; v.arbDataSize = 0;
        p.paramValues = v;
        ps->AddParam(exID, mg, FFMPEGFREEUI_INNER_GROUP_ID, &p);
    }

    // Hidden preset path string (stores the simplified .json path after conversion)
    {
        exNewParamInfo p; exParamValues v;
        safeStrCpy(p.identifier, 256, FFMPEGFREEUI_PRESET_PATH_ID);
        p.paramType = exParamType_string; p.flags = exParamFlag_none;
        v.value.intValue = 0; v.disabled = kPrFalse; v.hidden = kPrTrue;
        p.paramValues = v;
        ps->AddParam(exID, mg, FFMPEGFREEUI_INNER_GROUP_ID, &p);
    }

    // ============ VIDEO TAB ============
    copyConvertStringLiteralIntoUTF16(BASIC_VIDEO_GROUP_NAME, tmp);
    ps->AddParamGroup(exID, mg, ADBETopParamGroup, ADBEVideoTabGroup, tmp, kPrFalse, kPrFalse, kPrFalse);
    copyConvertStringLiteralIntoUTF16(BASIC_VIDEO_GROUP_NAME, tmp);
    ps->AddParamGroup(exID, mg, ADBEVideoTabGroup, ADBEBasicVideoGroup, tmp, kPrFalse, kPrFalse, kPrFalse);

    auto AddInt = [&](const char* group, const char* id, csSDK_int32 val, csSDK_int32 mn, csSDK_int32 mx) {
        exNewParamInfo p; exParamValues v;
        safeStrCpy(p.identifier, 256, id);
        p.paramType = exParamType_int; p.flags = exParamFlag_none;
        v.rangeMin.intValue = mn; v.rangeMax.intValue = mx;
        v.value.intValue = val; v.disabled = kPrFalse; v.hidden = kPrFalse;
        p.paramValues = v;
        ps->AddParam(exID, mg, group, &p);
    };

    AddInt(ADBEBasicVideoGroup, ADBEVideoWidth,  seqW.mInt32, 16, 8192);
    AddInt(ADBEBasicVideoGroup, ADBEVideoHeight, seqH.mInt32, 16, 8192);

    { // PAR
        exNewParamInfo p; exParamValues v;
        safeStrCpy(p.identifier, 256, ADBEVideoAspect);
        p.paramType = exParamType_ratio; p.flags = exParamFlag_none;
        v.rangeMin.ratioValue = {10,11}; v.rangeMax.ratioValue = {2,1};
        v.value.ratioValue = {seqPARNum.mInt32 ? seqPARNum.mInt32 : 1,
                              seqPARDen.mInt32 ? seqPARDen.mInt32 : 1};
        v.disabled = kPrFalse; v.hidden = kPrFalse;
        p.paramValues = v;
        ps->AddParam(exID, mg, ADBEBasicVideoGroup, &p);
    }
    { // FPS
        exNewParamInfo p; exParamValues v;
        safeStrCpy(p.identifier, 256, ADBEVideoFPS);
        p.paramType = exParamType_ticksFrameRate; p.flags = exParamFlag_none;
        PrTime tps; ts->GetTicksPerSecond(&tps);
        v.rangeMin.timeValue = 1; v.rangeMax.timeValue = tps;
        v.value.timeValue = seqFR.mInt64 ? seqFR.mInt64 : tps / 30;
        v.disabled = kPrFalse; v.hidden = kPrFalse;
        p.paramValues = v;
        ps->AddParam(exID, mg, ADBEBasicVideoGroup, &p);
    }
    AddInt(ADBEBasicVideoGroup, ADBEVideoFieldType, seqField.mInt32, 0, 3);

    // Color range (default: tv = limited)
    AddInt(ADBEBasicVideoGroup, FFMADOBE_COLOR_RANGE_ID, COLOR_RANGE_TV, 0, 1);

    // Color space (default: bt709 for HD content)
    AddInt(ADBEBasicVideoGroup, FFMADOBE_COLOR_SPACE_ID, COLOR_SPACE_BT709, 0, 4);

    // ============ AUDIO TAB ============
    copyConvertStringLiteralIntoUTF16(BASIC_AUDIO_GROUP_NAME, tmp);
    ps->AddParamGroup(exID, mg, ADBETopParamGroup, ADBEAudioTabGroup, tmp, kPrFalse, kPrFalse, kPrFalse);
    copyConvertStringLiteralIntoUTF16(BASIC_AUDIO_GROUP_NAME, tmp);
    ps->AddParamGroup(exID, mg, ADBEAudioTabGroup, ADBEBasicAudioGroup, tmp, kPrFalse, kPrFalse, kPrFalse);

    { // Sample rate
        exNewParamInfo p; exParamValues v;
        safeStrCpy(p.identifier, 256, ADBEAudioRatePerSecond);
        p.paramType = exParamType_float; p.flags = exParamFlag_none;
        v.value.floatValue = seqSR.mFloat64 > 0 ? (float)seqSR.mFloat64 : 48000.0f;
        v.disabled = kPrFalse; v.hidden = kPrFalse;
        p.paramValues = v;
        ps->AddParam(exID, mg, ADBEBasicAudioGroup, &p);
    }
    AddInt(ADBEBasicAudioGroup, ADBEAudioNumChannels, seqCh.mInt32 ? seqCh.mInt32 : kPrAudioChannelType_Stereo, 0, 5);

    ps->SetParamsVersion(exID, 1);
    return result;
}

// ===========================================================
// exSDKPostProcessParams - set display names and constrained values
// ===========================================================
prMALError exSDKPostProcessParams(exportStdParms* stdParmsP, exPostProcessParamsRec* rec)
{
    prMALError result = malNoError;
    csSDK_uint32 exID = rec->exporterPluginID;
    ExportSettings* lRec = reinterpret_cast<ExportSettings*>(rec->privateData);
    PrSDKExportParamSuite* ps = lRec->exportParamSuite;
    PrSDKTimeSuite* ts = lRec->timeSuite;
    prUTF16Char tmp[256];

    auto SetName = [&](const char* id, const wchar_t* name) {
        copyConvertStringLiteralIntoUTF16(name, tmp);
        ps->SetParamName(exID, 0, id, tmp);
    };

    SetName(FFMPEGFREEUI_TAB_ID,        FFMPEGFREEUI_GROUP_LABEL);
    SetName(FFMPEGFREEUI_INNER_GROUP_ID, FFMPEGFREEUI_GROUP_LABEL);
    SetName(FFMPEGFREEUI_CONFIGURE_BTN,  FFMPEGFREEUI_BTN_LABEL);
    SetName(ADBEBasicVideoGroup,      BASIC_VIDEO_GROUP_NAME);
    SetName(ADBEVideoWidth,           STR_WIDTH);
    SetName(ADBEVideoHeight,          STR_HEIGHT);
    SetName(ADBEVideoAspect,          STR_PAR);
    SetName(ADBEVideoFPS,             STR_FRAME_RATE);
    SetName(ADBEVideoFieldType,       STR_FIELD_ORDER);
    SetName(FFMADOBE_COLOR_RANGE_ID,  STR_COLOR_RANGE);
    SetName(FFMADOBE_COLOR_SPACE_ID,  STR_COLOR_SPACE);
    SetName(ADBEBasicAudioGroup,      BASIC_AUDIO_GROUP_NAME);
    SetName(ADBEAudioRatePerSecond,   STR_SAMPLE_RATE);
    SetName(ADBEAudioNumChannels,     STR_CHANNEL_TYPE);

    // Frame rate constrained values
    PrTime tps = 0; ts->GetTicksPerSecond(&tps);
    PrTime frs[][2] = {{10,1},{15,1},{24000,1001},{24,1},{25,1},{30000,1001},{30,1},{50,1},{60000,1001},{60,1}};
    const wchar_t* frStr[] = {STR_FRAME_RATE_10,STR_FRAME_RATE_15,STR_FRAME_RATE_23976,
        STR_FRAME_RATE_24,STR_FRAME_RATE_25,STR_FRAME_RATE_2997,STR_FRAME_RATE_30,
        STR_FRAME_RATE_50,STR_FRAME_RATE_5994,STR_FRAME_RATE_60};
    ps->ClearConstrainedValues(exID, 0, ADBEVideoFPS);
    for (int i = 0; i < 10; i++) {
        exOneParamValueRec v; v.timeValue = tps / frs[i][0] * frs[i][1];
        copyConvertStringLiteralIntoUTF16(frStr[i], tmp);
        ps->AddConstrainedValuePair(exID, 0, ADBEVideoFPS, &v, tmp);
    }

    // PAR constrained values
    int PARs[][2] = {{1,1},{10,11},{40,33},{768,702},{1024,702},{2,1},{4,3},{3,2}};
    const wchar_t* parStr[] = {STR_PAR_10,STR_PAR_09,STR_PAR_12,STR_PAR_11,STR_PAR_144,STR_PAR_20,STR_PAR_13,STR_PAR_15};
    ps->ClearConstrainedValues(exID, 0, ADBEVideoAspect);
    for (int i = 0; i < 8; i++) {
        exOneParamValueRec v; v.ratioValue = {PARs[i][0], PARs[i][1]};
        copyConvertStringLiteralIntoUTF16(parStr[i], tmp);
        ps->AddConstrainedValuePair(exID, 0, ADBEVideoAspect, &v, tmp);
    }

    // Field order constrained values
    int fields[] = {prFieldsUpperFirst, prFieldsLowerFirst, prFieldsNone};
    const wchar_t* fieldStr[] = {STR_FIELD_ORDER_UPPER, STR_FIELD_ORDER_LOWER, STR_FIELD_ORDER_NONE};
    ps->ClearConstrainedValues(exID, 0, ADBEVideoFieldType);
    for (int i = 0; i < 3; i++) {
        exOneParamValueRec v; v.intValue = fields[i];
        copyConvertStringLiteralIntoUTF16(fieldStr[i], tmp);
        ps->AddConstrainedValuePair(exID, 0, ADBEVideoFieldType, &v, tmp);
    }

    // Sample rate constrained values
    float srs[] = {8000,16000,32000,44100,48000,96000};
    const wchar_t* srStr[] = {STR_SAMPLE_RATE_8,STR_SAMPLE_RATE_16,STR_SAMPLE_RATE_32,STR_SAMPLE_RATE_441,STR_SAMPLE_RATE_48,STR_SAMPLE_RATE_96};
    ps->ClearConstrainedValues(exID, 0, ADBEAudioRatePerSecond);
    for (int i = 0; i < 6; i++) {
        exOneParamValueRec v; v.floatValue = srs[i];
        copyConvertStringLiteralIntoUTF16(srStr[i], tmp);
        ps->AddConstrainedValuePair(exID, 0, ADBEAudioRatePerSecond, &v, tmp);
    }

    // Channel type constrained values
    int chs[] = {kPrAudioChannelType_Mono, kPrAudioChannelType_Stereo, kPrAudioChannelType_51};
    const wchar_t* chStr[] = {STR_CHANNEL_TYPE_MONO, STR_CHANNEL_TYPE_STEREO, STR_CHANNEL_TYPE_51};
    ps->ClearConstrainedValues(exID, 0, ADBEAudioNumChannels);
    for (int i = 0; i < 3; i++) {
        exOneParamValueRec v; v.intValue = chs[i];
        copyConvertStringLiteralIntoUTF16(chStr[i], tmp);
        ps->AddConstrainedValuePair(exID, 0, ADBEAudioNumChannels, &v, tmp);
    }

    // Color range constrained values
    {
        int ranges[]              = {COLOR_RANGE_TV,         COLOR_RANGE_PC};
        const wchar_t* rangeStr[] = {STR_COLOR_RANGE_TV,     STR_COLOR_RANGE_PC};
        ps->ClearConstrainedValues(exID, 0, FFMADOBE_COLOR_RANGE_ID);
        for (int i = 0; i < 2; i++) {
            exOneParamValueRec v; v.intValue = ranges[i];
            copyConvertStringLiteralIntoUTF16(rangeStr[i], tmp);
            ps->AddConstrainedValuePair(exID, 0, FFMADOBE_COLOR_RANGE_ID, &v, tmp);
        }
    }

    // Color space constrained values
    {
        int spaces[]              = {COLOR_SPACE_BT709,         COLOR_SPACE_BT601_625,         COLOR_SPACE_BT601_525,         COLOR_SPACE_BT2020,         COLOR_SPACE_SRGB};
        const wchar_t* spaceStr[] = {STR_COLOR_SPACE_BT709,     STR_COLOR_SPACE_BT601_625,     STR_COLOR_SPACE_BT601_525,     STR_COLOR_SPACE_BT2020,     STR_COLOR_SPACE_SRGB};
        ps->ClearConstrainedValues(exID, 0, FFMADOBE_COLOR_SPACE_ID);
        for (int i = 0; i < 5; i++) {
            exOneParamValueRec v; v.intValue = spaces[i];
            copyConvertStringLiteralIntoUTF16(spaceStr[i], tmp);
            ps->AddConstrainedValuePair(exID, 0, FFMADOBE_COLOR_SPACE_ID, &v, tmp);
        }
    }

    return result;
}

// ===========================================================
// exSDKGetParamSummary
// ===========================================================
prMALError exSDKGetParamSummary(exportStdParms* stdParmsP, exParamSummaryRec* rec)
{
    ExportSettings* lRec = reinterpret_cast<ExportSettings*>(rec->privateData);
    PrSDKExportParamSuite* ps = lRec->exportParamSuite;
    PrSDKTimeSuite* ts = lRec->timeSuite;
    csSDK_uint32 exID = rec->exporterPluginID;
    wchar_t buf[256];

    exParamValues w, h, fps, presetPath, sr, ch;
    ps->GetParamValue(exID, 0, ADBEVideoWidth,          &w);
    ps->GetParamValue(exID, 0, ADBEVideoHeight,         &h);
    ps->GetParamValue(exID, 0, ADBEVideoFPS,            &fps);
    ps->GetParamValue(exID, 0, FFMPEGFREEUI_PRESET_PATH_ID, &presetPath);
    ps->GetParamValue(exID, 0, ADBEAudioRatePerSecond,  &sr);
    ps->GetParamValue(exID, 0, ADBEAudioNumChannels,    &ch);

    PrTime tps; ts->GetTicksPerSecond(&tps);
    double fpsVal = fps.value.timeValue > 0 ? (double)tps / fps.value.timeValue : 0.0;

    // Extract preset filename from path for display
    std::wstring presetName = L"(not configured)";
    if (presetPath.paramString[0] != 0) {
        std::wstring fullPath(reinterpret_cast<wchar_t*>(presetPath.paramString));
        size_t slash = fullPath.find_last_of(L"\\/");
        presetName = (slash != std::wstring::npos) ? fullPath.substr(slash + 1) : fullPath;
    }

    swprintf(buf, 256, L"%dx%d, %.3f fps | Preset: %s",
        w.value.intValue, h.value.intValue, fpsVal, presetName.c_str());
    copyConvertStringLiteralIntoUTF16(buf, rec->videoSummary);

    const wchar_t* chNames[] = {L"Mono", L"Stereo", L"5.1"};
    int chi = 0;
    if (ch.value.intValue == kPrAudioChannelType_Stereo) chi = 1;
    else if (ch.value.intValue == kPrAudioChannelType_51) chi = 2;
    swprintf(buf, 256, L"%.0f Hz, %s", sr.value.floatValue, chNames[chi]);
    copyConvertStringLiteralIntoUTF16(buf, rec->audioSummary);

    copyConvertStringLiteralIntoUTF16(L"FFmpegFreeUI", rec->bitrateSummary);
    return malNoError;
}

// ===========================================================
// exSDKParamButton - Launch FFmpegFreeUI --premiere, auto-convert, auto-read
// ===========================================================
prMALError exSDKParamButton(exportStdParms* stdParmsP, exParamButtonRec* rec)
{
    prMALError result = malNoError;
    ExportSettings* lRec = reinterpret_cast<ExportSettings*>(rec->privateData);
    PrSDKExportParamSuite* ps = lRec->exportParamSuite;
    csSDK_uint32 exID = rec->exporterPluginID;

    if (strcmp(rec->buttonParamIdentifier, FFMPEGFREEUI_CONFIGURE_BTN) != 0)
        return result;

    // ==== Resolve FFmpegFreeUI.exe: same-dir first, then registry fallback ====
    std::wstring ffuiExePath;
    std::wstring pluginDir;
    {
        HMODULE hSelf = NULL;
        GetModuleHandleExW(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)&exSDKParamButton, &hSelf);
        wchar_t dllPath[MAX_PATH] = {};
        GetModuleFileNameW(hSelf, dllPath, MAX_PATH);
        pluginDir = dllPath;
        size_t slash = pluginDir.find_last_of(L"\\/");
        if (slash != std::wstring::npos) pluginDir = pluginDir.substr(0, slash + 1);
        std::wstring sameDir = pluginDir + L"FFmpegFreeUI.exe";

        if (GetFileAttributesW(sameDir.c_str()) != INVALID_FILE_ATTRIBUTES) {
            ffuiExePath = sameDir;
        } else {
            HKEY hKey = NULL;
            if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\FFmAdobe", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                wchar_t regPath[MAX_PATH] = {};
                DWORD sz = sizeof(regPath); DWORD type = REG_SZ;
                if (RegQueryValueExW(hKey, L"FFmpegFreeUIPath", NULL, &type, (LPBYTE)regPath, &sz) == ERROR_SUCCESS)
                    ffuiExePath = regPath;
                RegCloseKey(hKey);
            }
        }
    }

    HWND mainWnd = lRec->windowSuite ? lRec->windowSuite->GetMainWindow() : NULL;

    if (ffuiExePath.empty() || GetFileAttributesW(ffuiExePath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        MessageBoxW(mainWnd,
            L"FFmpegFreeUI.exe not found.\n\n"
            L"Please install FFmpegFreeUI via the FFmAdobe installer,\n"
            L"or place it in the same folder as FFmAdobe.prm.",
            L"FFmpegFreeUI Not Found", MB_OK | MB_ICONERROR);
        return exportReturn_ErrOther;
    }

    // ==== Step 1: Launch FFmpegFreeUI --premiere and wait for exit ====
    {
        STARTUPINFOW si = {}; si.cb = sizeof(si);
        PROCESS_INFORMATION pi = {};
        std::wstring cmdLine = L"\"" + ffuiExePath + L"\" --premiere";
        std::vector<wchar_t> cmdBuf(cmdLine.begin(), cmdLine.end());
        cmdBuf.push_back(0);
        if (!CreateProcessW(NULL, cmdBuf.data(), NULL, NULL, FALSE,
                            0, NULL, NULL, &si, &pi)) {
            MessageBoxW(mainWnd, L"Failed to launch FFmpegFreeUI.exe.",
                L"Launch Failed", MB_OK | MB_ICONERROR);
            return exportReturn_ErrOther;
        }
        // Wait for FFmpegFreeUI to exit while pumping messages
        // (blocking the UI thread with WaitForSingleObject causes Premiere to crash)
        for (;;) {
            DWORD wr = MsgWaitForMultipleObjects(1, &pi.hProcess, FALSE, INFINITE, QS_ALLINPUT);
            if (wr == WAIT_OBJECT_0) break;          // process exited
            if (wr == WAIT_OBJECT_0 + 1) {            // messages pending
                MSG msg;
                while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
                    TranslateMessage(&msg);
                    DispatchMessageW(&msg);
                }
            } else break;                             // error
        }
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    // ==== Step 2: Auto-convert the saved preset ====
    wchar_t appData[MAX_PATH];
    SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appData);
    std::wstring outDir = std::wstring(appData) + L"\\FFmAdobe";
    CreateDirectoryW(outDir.c_str(), NULL);
    std::wstring nativePresetPath = outDir + L"\\premiere_preset.json";
    std::wstring simplifiedPath   = outDir + L"\\premiere_preset_simplified.json";

    if (GetFileAttributesW(nativePresetPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        MessageBoxW(mainWnd,
            L"No preset found. Please configure and save in FFmpegFreeUI first.\n"
            L"Default settings (H.264 CRF 18) will be used.",
            L"Warning", MB_OK | MB_ICONWARNING);
        return result;
    }

    std::wstring converterPath = pluginDir + L"PremierePresetConverter.exe";
    if (GetFileAttributesW(converterPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
        std::wstring cmdLine = L"\"" + converterPath + L"\" \"" +
                               nativePresetPath + L"\" \"" + simplifiedPath + L"\"";
        STARTUPINFOW si = {}; si.cb = sizeof(si);
        PROCESS_INFORMATION pi = {};
        std::vector<wchar_t> cmdBuf(cmdLine.begin(), cmdLine.end());
        cmdBuf.push_back(0);
        if (CreateProcessW(NULL, cmdBuf.data(), NULL, NULL, FALSE,
                           CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, 15000);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }

    // ==== Step 3: Store simplified preset path for export ====
    std::wstring finalPath = (GetFileAttributesW(simplifiedPath.c_str()) != INVALID_FILE_ATTRIBUTES)
                             ? simplifiedPath : nativePresetPath;
    {
        exParamValues presetPathVal;
        ps->GetParamValue(exID, rec->multiGroupIndex, FFMPEGFREEUI_PRESET_PATH_ID, &presetPathVal);
        exParamValues newVal = presetPathVal;
        wcsncpy(reinterpret_cast<wchar_t*>(newVal.paramString), finalPath.c_str(), 255);
        reinterpret_cast<wchar_t*>(newVal.paramString)[255] = 0;
        ps->ChangeParam(exID, rec->multiGroupIndex, FFMPEGFREEUI_PRESET_PATH_ID, &newVal);
    }

    return result;
}

// ===========================================================
// exSDKValidateParamChanged
// ===========================================================
prMALError exSDKValidateParamChanged(exportStdParms* stdParmsP, exParamChangedRec* rec)
{
    prMALError result = malNoError;
    ExportSettings* lRec = reinterpret_cast<ExportSettings*>(rec->privateData);
    PrSDKExportParamSuite* ps = lRec->exportParamSuite;
    csSDK_uint32 exID = rec->exporterPluginID;

    if (strcmp(rec->changedParamIdentifier, ADBEVideoWidth) == 0) {
        exParamValues v; ps->GetParamValue(exID, rec->multiGroupIndex, ADBEVideoWidth, &v);
        lRec->SDKFileRec.width = v.value.intValue;
    } else if (strcmp(rec->changedParamIdentifier, ADBEVideoHeight) == 0) {
        exParamValues v; ps->GetParamValue(exID, rec->multiGroupIndex, ADBEVideoHeight, &v);
        lRec->SDKFileRec.height = v.value.intValue;
    }
    return result;
}
