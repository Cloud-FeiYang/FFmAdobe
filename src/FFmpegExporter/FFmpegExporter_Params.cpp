#include "FFmpegExporter_Params.h"
#include <windows.h>
#include <shlobj.h>
#include <string>

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

    // Hidden preset path string (stores the .3fuipreset file path chosen by user)
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
// exSDKParamButton - Launch FFmpegFreeUI when Configure is clicked
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
    {
        // 1. Check same directory as the plugin DLL
        HMODULE hSelf = NULL;
        GetModuleHandleExW(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)&exSDKParamButton, &hSelf);
        wchar_t dllPath[MAX_PATH] = {};
        GetModuleFileNameW(hSelf, dllPath, MAX_PATH);
        std::wstring dir(dllPath);
        size_t slash = dir.find_last_of(L"\\/");
        if (slash != std::wstring::npos) dir = dir.substr(0, slash + 1);
        std::wstring sameDir = dir + L"FFmpegFreeUI.exe";

        if (GetFileAttributesW(sameDir.c_str()) != INVALID_FILE_ATTRIBUTES) {
            ffuiExePath = sameDir;
        } else {
            // 2. Read path from registry (written by MSI installer)
            HKEY hKey = NULL;
            if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\FFmAdobe", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                wchar_t regPath[MAX_PATH] = {};
                DWORD sz = sizeof(regPath);
                DWORD type = REG_SZ;
                if (RegQueryValueExW(hKey, L"FFmpegFreeUIPath", NULL, &type, (LPBYTE)regPath, &sz) == ERROR_SUCCESS)
                    ffuiExePath = regPath;
                RegCloseKey(hKey);
            }
        }
    }

    // Verify the resolved path is valid
    if (ffuiExePath.empty() || GetFileAttributesW(ffuiExePath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        HWND mainWnd = lRec->windowSuite ? lRec->windowSuite->GetMainWindow() : NULL;
        std::wstring msg = L"FFmpegFreeUI.exe not found.\n\n"
            L"Please install FFmpegFreeUI and ensure it is either:\n"
            L"  \x2022 In the same folder as FFmpegExporter.prm, or\n"
            L"  \x2022 Installed via the FFmAdobe installer (which configures the path automatically).\n\n"
            L"Registry key checked: HKLM\\SOFTWARE\\FFmAdobe\\FFmpegFreeUIPath";
        MessageBoxW(mainWnd, msg.c_str(), L"FFmpegFreeUI Not Found", MB_OK | MB_ICONERROR);
        return exportReturn_ErrOther;
    }

    // Get current preset path (if any)
    exParamValues presetPathVal;
    ps->GetParamValue(exID, rec->multiGroupIndex, FFMPEGFREEUI_PRESET_PATH_ID, &presetPathVal);
    std::wstring currentPresetPath(reinterpret_cast<wchar_t*>(presetPathVal.paramString));

    // If no preset yet, suggest a default save location
    if (currentPresetPath.empty()) {
        wchar_t appData[MAX_PATH];
        SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appData);
        currentPresetPath = std::wstring(appData) + L"\\FFmpegFreeUI\\last_preset.3fuipreset";
        // Ensure directory exists
        std::wstring presetDir = std::wstring(appData) + L"\\FFmpegFreeUI";
        CreateDirectoryW(presetDir.c_str(), NULL);
    }

    // Build args: pass the preset path so FFmpegFreeUI opens/creates it
    std::wstring args = L"\"" + currentPresetPath + L"\"";

    // Launch FFmpegFreeUI.exe (full path, not relying on PATH)
    SHELLEXECUTEINFOW sei = {};
    sei.cbSize = sizeof(sei);
    sei.fMask  = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = L"open";
    sei.lpFile = ffuiExePath.c_str();
    sei.lpParameters = args.c_str();
    sei.nShow  = SW_SHOWNORMAL;

    if (!ShellExecuteExW(&sei)) {
        HWND mainWnd = lRec->windowSuite ? lRec->windowSuite->GetMainWindow() : NULL;
        MessageBoxW(mainWnd,
            L"Failed to launch FFmpegFreeUI.exe.\nCheck if the file is corrupted or blocked.",
            L"Launch Failed", MB_OK | MB_ICONERROR);
        return exportReturn_ErrOther;
    }

    // Wait for FFmpegFreeUI to close (so the preset file is written)
    if (sei.hProcess) {
        WaitForSingleObject(sei.hProcess, INFINITE);
        CloseHandle(sei.hProcess);
    }

    // After FFmpegFreeUI closes, check if the preset file was written
    if (GetFileAttributesW(currentPresetPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
        // Store the preset path in our hidden param
        exParamValues newVal = presetPathVal;
        wcsncpy(reinterpret_cast<wchar_t*>(newVal.paramString), currentPresetPath.c_str(), 1023);
        reinterpret_cast<wchar_t*>(newVal.paramString)[1023] = 0;
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
