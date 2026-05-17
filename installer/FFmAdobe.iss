; ============================================================
; FFmAdobe Installer - Inno Setup Script
; ============================================================
; Flow: Welcome → License → Components → Plugin Path → FFmpegFreeUI Path → Ready → Install
; ============================================================

#define MyAppName      "FFmAdobe"
#define MyAppVersion   "26w20a"
#define MyAppPublisher "FFmAdobe Project"
#define MyAppURL       "https://github.com/Cloud-FeiYang/FFmAdobe"

; ============================================================
; [Setup]
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
DisableProgramGroupPage=yes
; Hide the default dir page — we use custom pages instead
DisableDirPage=yes

; ============================================================
; [Languages]
; ============================================================
[Languages]
Name: "chinesesimplified"; MessagesFile: "ChineseSimplified.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

; ============================================================
; [Types]
; ============================================================
[Types]
Name: "full";    Description: "完整安装 (Full installation)"
Name: "plugin";  Description: "仅安装插件 (Plugin only)"
Name: "custom";  Description: "自定义 (Custom)"; Flags: iscustom

; ============================================================
; [Components] — hint text tells user paths come next
; ============================================================
[Components]
Name: "plugin";    Description: "FFmAdobe 插件 (.prm)  —  Premiere Pro 导出器"; Types: full plugin custom; Flags: fixed
Name: "converter"; Description: "PremierePresetConverter  —  预设转换工具（随插件安装）"; Types: full plugin custom
Name: "ffui";      Description: "FFmpegFreeUI  —  编码参数配置前端"; Types: full custom

; ============================================================
; [Files] — DestDir uses {code:...} to read custom page values
; ============================================================
[Files]
; Plugin
Source: "staging\Plugin\FFmAdobe.prm";              DestDir: "{code:GetPluginDir}"; Flags: ignoreversion; Components: plugin
Source: "staging\Plugin\PremierePresetConverter.exe"; DestDir: "{code:GetPluginDir}"; Flags: ignoreversion; Components: converter
; FFmpegFreeUI
Source: "staging\FFmpegFreeUI\FFmpegFreeUI.exe";            DestDir: "{code:GetFFUIDir}"; Flags: ignoreversion; Components: ffui
Source: "staging\FFmpegFreeUI\D3DCompiler_47_cor3.dll";      DestDir: "{code:GetFFUIDir}"; Flags: ignoreversion; Components: ffui
Source: "staging\FFmpegFreeUI\wpfgfx_cor3.dll";              DestDir: "{code:GetFFUIDir}"; Flags: ignoreversion; Components: ffui
Source: "staging\FFmpegFreeUI\PresentationNative_cor3.dll";  DestDir: "{code:GetFFUIDir}"; Flags: ignoreversion; Components: ffui
Source: "staging\FFmpegFreeUI\vcruntime140_cor3.dll";         DestDir: "{code:GetFFUIDir}"; Flags: ignoreversion; Components: ffui
Source: "staging\FFmpegFreeUI\MonoPosixHelper.dll";           DestDir: "{code:GetFFUIDir}"; Flags: ignoreversion; Components: ffui
Source: "staging\FFmpegFreeUI\libMonoPosixHelper.dll";        DestDir: "{code:GetFFUIDir}"; Flags: ignoreversion; Components: ffui
Source: "staging\FFmpegFreeUI\PenImc_cor3.dll";               DestDir: "{code:GetFFUIDir}"; Flags: ignoreversion; Components: ffui

; ============================================================
; [Dirs]
; ============================================================
[Dirs]
Name: "{code:GetPluginDir}"; Components: plugin
Name: "{code:GetFFUIDir}";   Components: ffui

; ============================================================
; [Registry]
; ============================================================
[Registry]
Root: HKLM; Subkey: "SOFTWARE\FFmAdobe"; ValueType: string; ValueName: "FFmpegFreeUIPath"; ValueData: "{code:GetFFUIDir}\FFmpegFreeUI.exe"; Flags: uninsdeletevalue; Components: ffui
Root: HKLM; Subkey: "SOFTWARE\FFmAdobe"; ValueType: string; ValueName: "PluginPath"; ValueData: "{code:GetPluginDir}"; Flags: uninsdeletevalue; Components: plugin
Root: HKLM; Subkey: "SOFTWARE\FFmAdobe"; Flags: uninsdeletekeyifempty

; ============================================================
; [Icons]
; ============================================================
[Icons]
Name: "{group}\FFmpegFreeUI";  Filename: "{code:GetFFUIDir}\FFmpegFreeUI.exe"; Components: ffui
Name: "{group}\卸载 FFmAdobe";  Filename: "{uninstallexe}"

; ============================================================
; [UninstallDelete]
; ============================================================
[UninstallDelete]
Type: dirifempty;     Name: "{code:GetPluginDir}"
Type: filesandordirs; Name: "{code:GetFFUIDir}"
Type: filesandordirs; Name: "{commonappdata}\FFmAdobe"

; ============================================================
; [Code] — Custom directory pages after component selection
; ============================================================
[Code]
var
  PluginDirPage: TInputDirWizardPage;
  FFUIDirPage:   TInputDirWizardPage;

// Default paths
function DefaultPluginDir: String;
begin
  Result := ExpandConstant('{autopf}\Adobe\Common\Plug-ins\7.0\MediaCore\FFmAdobe');
end;

function DefaultFFUIDir: String;
begin
  Result := ExpandConstant('{autopf}\FFmAdobe\FFmpegFreeUI');
end;

// Called by {code:GetPluginDir} — returns the user-selected plugin path
function GetPluginDir(Param: String): String;
begin
  if Assigned(PluginDirPage) then
    Result := PluginDirPage.Values[0]
  else
    Result := DefaultPluginDir;
end;

// Called by {code:GetFFUIDir} — returns the user-selected FFmpegFreeUI path
function GetFFUIDir(Param: String): String;
begin
  if Assigned(FFUIDirPage) then
    Result := FFUIDirPage.Values[0]
  else
    Result := DefaultFFUIDir;
end;

// Create custom pages after the component selection page
procedure InitializeWizard;
begin
  // Plugin directory page (always shown since plugin is fixed/required)
  PluginDirPage := CreateInputDirPage(
    wpSelectComponents,
    '插件安装目录 (Plugin Install Path)',
    '请选择 FFmAdobe 插件的安装目录。',
    '插件文件 (FFmAdobe.prm) 将安装到以下目录。' + #13#10 +
    '如果你的 Premiere Pro 安装在非默认位置，请修改为对应的 MediaCore 目录。' + #13#10#13#10 +
    '默认路径适用于标准安装的 Premiere Pro。',
    False, '');
  PluginDirPage.Add('插件目录:');
  PluginDirPage.Values[0] := DefaultPluginDir;

  // FFmpegFreeUI directory page (shown only when ffui component is selected)
  FFUIDirPage := CreateInputDirPage(
    PluginDirPage.ID,
    'FFmpegFreeUI 安装目录',
    '请选择 FFmpegFreeUI 前端工具的安装目录。',
    'FFmpegFreeUI 将安装到以下目录。',
    False, '');
  FFUIDirPage.Add('FFmpegFreeUI 目录:');
  FFUIDirPage.Values[0] := DefaultFFUIDir;
end;

// Show/hide FFmpegFreeUI dir page based on component selection
function ShouldSkipPage(PageID: Integer): Boolean;
begin
  Result := False;
  if (PageID = FFUIDirPage.ID) and (not WizardIsComponentSelected('ffui')) then
    Result := True;
end;

// Summary on Ready page
function UpdateReadyMemo(Space, NewLine, MemoUserInfoInfo, MemoDirInfo, MemoTypeInfo, MemoComponentsInfo, MemoGroupInfo, MemoTasksInfo: String): String;
var
  S: String;
begin
  S := '';
  S := S + '安装组件:' + NewLine;
  S := S + MemoComponentsInfo + NewLine + NewLine;

  S := S + '插件安装目录:' + NewLine;
  S := S + Space + GetPluginDir('') + NewLine + NewLine;

  if WizardIsComponentSelected('ffui') then
  begin
    S := S + 'FFmpegFreeUI 安装目录:' + NewLine;
    S := S + Space + GetFFUIDir('') + NewLine + NewLine;
  end;

  if WizardIsComponentSelected('ffui') then
  begin
    S := S + '注册表:' + NewLine;
    S := S + Space + 'HKLM\SOFTWARE\FFmAdobe\FFmpegFreeUIPath' + NewLine;
    S := S + Space + '  = ' + GetFFUIDir('') + '\FFmpegFreeUI.exe' + NewLine;
  end;

  Result := S;
end;
