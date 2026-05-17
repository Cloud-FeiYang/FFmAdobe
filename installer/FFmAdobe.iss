; ============================================================
; FFmAdobe Installer - Inno Setup Script
; ============================================================
; Installs:
;   1. FFmpegExporter.prm + PremierePresetConverter.exe (Plugin)
;   2. FFmpegFreeUI (encoding configuration frontend)
; Both components have independent uninstall entries in Windows Settings.
; ============================================================

#define MyAppName      "FFmAdobe"
#define MyAppVersion   "1.0.0"
#define MyAppPublisher "FFmAdobe Project"
#define MyAppURL       "https://github.com/FFmAdobe"

; ============================================================
; [Setup] - Installer metadata and behavior
; ============================================================
[Setup]
AppId={{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
DefaultDirName={autopf}\FFmAdobe
DefaultGroupName={#MyAppName}
OutputDir=output
OutputBaseFilename=FFmAdobe-{#MyAppVersion}-x64-Setup
Compression=lzma2/ultra64
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
WizardStyle=modern
SetupIconFile=compiler:SetupClassicIcon.ico
UninstallDisplayIcon={app}\uninstall-plugin.exe
DisableProgramGroupPage=yes
; Allow user to change dirs for each component
DisableDirPage=no

; ============================================================
; [Languages]
; ============================================================
[Languages]
Name: "chinesesimplified"; MessagesFile: "ChineseSimplified.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

; ============================================================
; [Types] - Installation types
; ============================================================
[Types]
Name: "full";    Description: "完整安装 (Full installation)"
Name: "plugin";  Description: "仅安装插件 (Plugin only)"
Name: "custom";  Description: "自定义 (Custom)"; Flags: iscustom

; ============================================================
; [Components] - Selectable components
; ============================================================
[Components]
Name: "plugin";    Description: "Premiere Pro FFmpeg Exporter 插件"; Types: full plugin custom; Flags: fixed
Name: "ffui";      Description: "FFmpegFreeUI 编码参数配置前端";     Types: full custom
Name: "converter"; Description: "PremierePresetConverter 预设转换工具"; Types: full custom

; ============================================================
; [Dirs] - Directories to create
; ============================================================
[Dirs]
; Plugin install directory (Adobe MediaCore standard path)
Name: "{autopf}\Adobe\Common\Plug-ins\7.0\MediaCore\FFmAdobe"; Components: plugin
; FFmpegFreeUI install directory
Name: "{app}\FFmpegFreeUI"; Components: ffui

; ============================================================
; [Files] - Files to install
; ============================================================
[Files]
; --- Plugin files → Adobe MediaCore directory ---
Source: "staging\Plugin\FFmpegExporter.prm";          DestDir: "{autopf}\Adobe\Common\Plug-ins\7.0\MediaCore\FFmAdobe"; Flags: ignoreversion; Components: plugin
Source: "staging\Plugin\PremierePresetConverter.exe";  DestDir: "{autopf}\Adobe\Common\Plug-ins\7.0\MediaCore\FFmAdobe"; Flags: ignoreversion; Components: converter

; --- FFmpegFreeUI files → {app}\FFmpegFreeUI ---
Source: "staging\FFmpegFreeUI\FFmpegFreeUI.exe";            DestDir: "{app}\FFmpegFreeUI"; Flags: ignoreversion; Components: ffui
Source: "staging\FFmpegFreeUI\D3DCompiler_47_cor3.dll";      DestDir: "{app}\FFmpegFreeUI"; Flags: ignoreversion; Components: ffui
Source: "staging\FFmpegFreeUI\wpfgfx_cor3.dll";              DestDir: "{app}\FFmpegFreeUI"; Flags: ignoreversion; Components: ffui
Source: "staging\FFmpegFreeUI\PresentationNative_cor3.dll";  DestDir: "{app}\FFmpegFreeUI"; Flags: ignoreversion; Components: ffui
Source: "staging\FFmpegFreeUI\vcruntime140_cor3.dll";         DestDir: "{app}\FFmpegFreeUI"; Flags: ignoreversion; Components: ffui
Source: "staging\FFmpegFreeUI\MonoPosixHelper.dll";           DestDir: "{app}\FFmpegFreeUI"; Flags: ignoreversion; Components: ffui
Source: "staging\FFmpegFreeUI\libMonoPosixHelper.dll";        DestDir: "{app}\FFmpegFreeUI"; Flags: ignoreversion; Components: ffui
Source: "staging\FFmpegFreeUI\PenImc_cor3.dll";               DestDir: "{app}\FFmpegFreeUI"; Flags: ignoreversion; Components: ffui

; ============================================================
; [Registry] - Write FFmpegFreeUI path for plugin discovery
; ============================================================
[Registry]
; The C++ plugin reads this key to locate FFmpegFreeUI.exe
Root: HKLM; Subkey: "SOFTWARE\FFmAdobe"; ValueType: string; ValueName: "FFmpegFreeUIPath"; ValueData: "{app}\FFmpegFreeUI\FFmpegFreeUI.exe"; Flags: uninsdeletevalue; Components: ffui
; Clean up the key on full uninstall
Root: HKLM; Subkey: "SOFTWARE\FFmAdobe"; Flags: uninsdeletekeyifempty

; ============================================================
; [Icons] - Start Menu shortcuts
; ============================================================
[Icons]
Name: "{group}\FFmpegFreeUI";          Filename: "{app}\FFmpegFreeUI\FFmpegFreeUI.exe"; Components: ffui
Name: "{group}\卸载 FFmAdobe";          Filename: "{uninstallexe}"

; ============================================================
; [UninstallDelete] - Clean up extra files on uninstall
; ============================================================
[UninstallDelete]
; Remove the plugin directory if empty after uninstall
Type: dirifempty; Name: "{autopf}\Adobe\Common\Plug-ins\7.0\MediaCore\FFmAdobe"
; Remove FFmpegFreeUI directory
Type: filesandordirs; Name: "{app}\FFmpegFreeUI"
; Remove user preset data
Type: filesandordirs; Name: "{commonappdata}\FFmAdobe"

; ============================================================
; [Code] - Pascal script for custom behavior
; ============================================================
[Code]
// Show a summary page before installation
function UpdateReadyMemo(Space, NewLine, MemoUserInfoInfo, MemoDirInfo, MemoTypeInfo, MemoComponentsInfo, MemoGroupInfo, MemoTasksInfo: String): String;
var
  S: String;
begin
  S := '';
  S := S + '安装组件:' + NewLine;
  S := S + MemoComponentsInfo + NewLine + NewLine;

  S := S + '插件安装目录:' + NewLine;
  S := S + Space + ExpandConstant('{autopf}\Adobe\Common\Plug-ins\7.0\MediaCore\FFmAdobe') + NewLine + NewLine;

  if WizardIsComponentSelected('ffui') then
  begin
    S := S + 'FFmpegFreeUI 安装目录:' + NewLine;
    S := S + Space + ExpandConstant('{app}\FFmpegFreeUI') + NewLine + NewLine;
  end;

  S := S + '注册表:' + NewLine;
  S := S + Space + 'HKLM\SOFTWARE\FFmAdobe\FFmpegFreeUIPath' + NewLine;

  Result := S;
end;
