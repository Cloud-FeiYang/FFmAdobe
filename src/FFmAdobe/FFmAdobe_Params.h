#pragma once
#include <SDK_File.h>

// ==== FFmpegFreeUI integration params ====
#define FFMPEGFREEUI_TAB_ID         "FFmpegFreeUITab"         // top-level tab (own collapsible section)
#define FFMPEGFREEUI_INNER_GROUP_ID "FFmpegFreeUIInnerGroup"  // inner group inside the tab
#define FFMPEGFREEUI_CONFIGURE_BTN  "FFmpegFreeUIConfigure"
#define FFMPEGFREEUI_PRESET_PATH_ID "FFmpegFreeUIPresetPath"  // hidden string: stores .3fuipreset path

// ==== Basic AV params (Pr-standard) ====
// (video size/fps/par/field are kept for Pr sequence matching)
// Audio: sample rate and channels stay in standard Pr audio tab

// ==== Group / tab names ====
#define FFMPEGFREEUI_GROUP_LABEL    L"FFmpegFreeUI"
#define FFMPEGFREEUI_BTN_LABEL      L"Configure"
#define BASIC_VIDEO_GROUP_NAME      L"Basic Video"
#define BASIC_AUDIO_GROUP_NAME      L"Basic Audio"

// ==== Standard label strings ====
#define STR_WIDTH               L"Width"
#define STR_HEIGHT              L"Height"
#define STR_FRAME_RATE          L"Frame Rate (fps)"
#define STR_PAR                 L"Pixel Aspect Ratio"
#define STR_FIELD_ORDER         L"Field Type"
#define STR_SAMPLE_RATE         L"Sample Rate"
#define STR_CHANNEL_TYPE        L"Channels"

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
#define STR_PAR_10              L"Square pixels (1.0)"
#define STR_PAR_09              L"D1/DV NTSC (0.9091)"
#define STR_PAR_12              L"D1/DV NTSC Widescreen (1.2121)"
#define STR_PAR_11              L"D1/DV PAL (1.0940)"
#define STR_PAR_144             L"D1/DV PAL Widescreen (1.4587)"
#define STR_PAR_20              L"Anamorphic 2:1 (2.0)"
#define STR_PAR_13              L"HD Anamorphic 1080 (1.3333)"
#define STR_PAR_15              L"DVCPRO HD (1.5)"

// Field order
#define STR_FIELD_ORDER_UPPER   L"Upper First"
#define STR_FIELD_ORDER_LOWER   L"Lower First"
#define STR_FIELD_ORDER_NONE    L"None (Progressive)"

// Sample rates
#define STR_SAMPLE_RATE_8       L"8000 Hz"
#define STR_SAMPLE_RATE_16      L"16000 Hz"
#define STR_SAMPLE_RATE_32      L"32000 Hz"
#define STR_SAMPLE_RATE_441     L"44100 Hz"
#define STR_SAMPLE_RATE_48      L"48000 Hz"
#define STR_SAMPLE_RATE_96      L"96000 Hz"

// Channels
#define STR_CHANNEL_TYPE_MONO   L"Mono"
#define STR_CHANNEL_TYPE_STEREO L"Stereo"
#define STR_CHANNEL_TYPE_51     L"5.1"

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
