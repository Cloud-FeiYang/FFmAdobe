#pragma once
#include <SDK_File.h>
#include <windows.h>

// ==== FFmpegFreeUI integration params ====
#define FFMPEGFREEUI_TAB_ID         "FFmpegFreeUITab"
#define FFMPEGFREEUI_INNER_GROUP_ID "FFmpegFreeUIInnerGroup"
#define FFMPEGFREEUI_CONFIGURE_BTN  "FFmpegFreeUIConfigure"
#define FFMPEGFREEUI_PRESET_PATH_ID "FFmpegFreeUIPresetPath"

// ==== New color params ====
#define FFMADOBE_COLOR_RANGE_ID     "FFmAdobeColorRange"
#define FFMADOBE_COLOR_SPACE_ID     "FFmAdobeColorSpace"

// ==== Color range values ====
#define COLOR_RANGE_TV   0   // tv / limited (16-235)
#define COLOR_RANGE_PC   1   // pc / full    (0-255)

// ==== Color space values ====
#define COLOR_SPACE_BT709     0
#define COLOR_SPACE_BT601_625 1
#define COLOR_SPACE_BT601_525 2
#define COLOR_SPACE_BT2020    3
#define COLOR_SPACE_SRGB      4

// ===========================================================
// Language detection: returns true if OS UI language is Chinese
// ===========================================================
inline bool IsChineseUI() {
    LANGID lang = GetUserDefaultUILanguage();
    WORD primary = PRIMARYLANGID(lang);
    return (primary == LANG_CHINESE);
}

// Helper: returns zh if Chinese UI, otherwise en
// Uses a real inline function to avoid macro comma-arg ambiguity.
inline const wchar_t* LStr(const wchar_t* zh, const wchar_t* en) {
    return IsChineseUI() ? zh : en;
}

// ==== Group / tab names ====
#define FFMPEGFREEUI_GROUP_LABEL    L"FFmpegFreeUI"
#define FFMPEGFREEUI_BTN_LABEL      LStr(L"\u914D\u7F6E",           L"Configure")
#define BASIC_VIDEO_GROUP_NAME      LStr(L"\u57FA\u672C\u89C6\u9891", L"Basic Video")
#define BASIC_AUDIO_GROUP_NAME      LStr(L"\u57FA\u672C\u97F3\u9891", L"Basic Audio")

// ==== Standard label strings ====
#define STR_WIDTH               LStr(L"\u5BBD\u5EA6",             L"Width")
#define STR_HEIGHT              LStr(L"\u9AD8\u5EA6",             L"Height")
#define STR_FRAME_RATE          LStr(L"\u5E27\u7387 (fps)",       L"Frame Rate (fps)")
#define STR_PAR                 LStr(L"\u50CF\u7D20\u957F\u5BBD\u6BD4", L"Pixel Aspect Ratio")
#define STR_FIELD_ORDER         LStr(L"\u573A\u7C7B\u578B",       L"Field Type")
#define STR_SAMPLE_RATE         LStr(L"\u91C7\u6837\u7387",       L"Sample Rate")
#define STR_CHANNEL_TYPE        LStr(L"\u58F0\u9053",             L"Channels")
#define STR_COLOR_RANGE         LStr(L"\u8272\u5F69\u8303\u56F4", L"Color Range")
#define STR_COLOR_SPACE         LStr(L"\u8272\u5F69\u7A7A\u95F4", L"Color Space")

// Frame rates
#define STR_FRAME_RATE_10       L"10"
#define STR_FRAME_RATE_15       L"15"
#define STR_FRAME_RATE_23976    L"23.976"
#define STR_FRAME_RATE_24       L"24"
#define STR_FRAME_RATE_25       L"25 (PAL)"
#define STR_FRAME_RATE_2997     L"29.97 (NTSC)"
#define STR_FRAME_RATE_30       L"30"
#define STR_FRAME_RATE_50       L"50"
#define STR_FRAME_RATE_5994     L"59.94"
#define STR_FRAME_RATE_60       L"60"

// PAR
#define STR_PAR_10      LStr(L"\u65B9\u5F62\u50CF\u7D20 (1.0)",                  L"Square pixels (1.0)")
#define STR_PAR_09      LStr(L"D1/DV NTSC (0.9091)",                              L"D1/DV NTSC (0.9091)")
#define STR_PAR_12      LStr(L"D1/DV NTSC \u5BBD\u5C4F (1.2121)",                L"D1/DV NTSC Widescreen (1.2121)")
#define STR_PAR_11      LStr(L"D1/DV PAL (1.0940)",                               L"D1/DV PAL (1.0940)")
#define STR_PAR_144     LStr(L"D1/DV PAL \u5BBD\u5C4F (1.4587)",                 L"D1/DV PAL Widescreen (1.4587)")
#define STR_PAR_20      LStr(L"\u53D8\u5F62\u5BBD\u9280\u5E55 2:1 (2.0)",        L"Anamorphic 2:1 (2.0)")
#define STR_PAR_13      LStr(L"HD \u53D8\u5F62\u5BBD\u9280\u5E55 1080 (1.3333)", L"HD Anamorphic 1080 (1.3333)")
#define STR_PAR_15      LStr(L"DVCPRO HD (1.5)",                                  L"DVCPRO HD (1.5)")

// Field order
#define STR_FIELD_ORDER_UPPER   LStr(L"\u4E0A\u573A\u4F18\u5148", L"Upper First")
#define STR_FIELD_ORDER_LOWER   LStr(L"\u4E0B\u573A\u4F18\u5148", L"Lower First")
#define STR_FIELD_ORDER_NONE    LStr(L"\u65E0 (\u9010\u884C)",    L"None (Progressive)")

// Sample rates
#define STR_SAMPLE_RATE_8       L"8000 Hz"
#define STR_SAMPLE_RATE_16      L"16000 Hz"
#define STR_SAMPLE_RATE_32      L"32000 Hz"
#define STR_SAMPLE_RATE_441     L"44100 Hz"
#define STR_SAMPLE_RATE_48      L"48000 Hz"
#define STR_SAMPLE_RATE_96      L"96000 Hz"

// Channels
#define STR_CHANNEL_TYPE_MONO   LStr(L"\u5355\u58F0\u9053", L"Mono")
#define STR_CHANNEL_TYPE_STEREO LStr(L"\u7ACB\u4F53\u58F0", L"Stereo")
#define STR_CHANNEL_TYPE_51     L"5.1"

// Color range
#define STR_COLOR_RANGE_TV  LStr(L"\u53D7\u9650\u8303\u56F4 (16-235, TV)",  L"Limited (16-235, TV)")
#define STR_COLOR_RANGE_PC  LStr(L"\u5B8C\u6574\u8303\u56F4 (0-255, PC)",   L"Full (0-255, PC)")

// Color space
#define STR_COLOR_SPACE_BT709     LStr(L"BT.709 (\u9AD8\u6E05)",           L"BT.709 (HD)")
#define STR_COLOR_SPACE_BT601_625 LStr(L"BT.601 PAL (625\u7EBF)",          L"BT.601 PAL (625-line)")
#define STR_COLOR_SPACE_BT601_525 LStr(L"BT.601 NTSC (525\u7EBF)",         L"BT.601 NTSC (525-line)")
#define STR_COLOR_SPACE_BT2020    LStr(L"BT.2020 (\u8D85\u9AD8\u6E05/HDR)",L"BT.2020 (UHD/HDR)")
#define STR_COLOR_SPACE_SRGB      L"sRGB"

// ==== Function declarations ====
prMALError exSDKGenerateDefaultParams(
    exportStdParms              *stdParms,
    exGenerateDefaultParamRec   *generateDefaultParamRec);

prMALError exSDKPostProcessParams(
    exportStdParms          *stdParmsP,
    exPostProcessParamsRec  *postProcessParamsRecP);

prMALError exSDKGetParamSummary(
    exportStdParms      *stdParmsP,
    exParamSummaryRec   *summaryRecP);

prMALError exSDKParamButton(
    exportStdParms      *stdParmsP,
    exParamButtonRec    *getFilePrefsRecP);

prMALError exSDKValidateParamChanged(
    exportStdParms      *stdParmsP,
    exParamChangedRec   *validateParamChangedRecP);
