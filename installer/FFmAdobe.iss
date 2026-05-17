; ============================================================
; FFmAdobe Installer - Inno Setup Script
; ============================================================
; New install:  Welcome → Components → Plugin Path → FFUI Path → Ready
; Maintenance:  Welcome → Repair/Modify/Uninstall → (depends on choice)
; ============================================================

#define MyAppName      "FFmAdobe"
#define MyAppVersion   "26w21b"
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
DisableDirPage=yes
; Allow same version to be reinstalled (repair/modify)
UsePreviousAppDir=yes

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
; [Components]
; ============================================================
[Components]
Name: "plugin";    Description: "FFmAdobe 插件 (.prm)  —  Premiere Pro 导出器"; Types: full plugin custom; Flags: fixed
Name: "converter"; Description: "PremierePresetConverter  —  预设转换工具（随插件安装）"; Types: full plugin custom
Name: "ffui";      Description: "FFmpegFreeUI  —  编码参数配置前端"; Types: full custom

; ============================================================
; [Files]
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
; [Code] — Maintenance mode + custom directory pages
; ============================================================
[Code]
var
  PluginDirPage:      TInputDirWizardPage;
  FFUIDirPage:        TInputDirWizardPage;
  MaintenancePage:    TWizardPage;
  RadioRepair:        TNewRadioButton;
  RadioModify:        TNewRadioButton;
  RadioUninstall:     TNewRadioButton;
  IsMaintenanceMode:  Boolean;
  PrevUninstallStr:   String;
  PrevPluginDir:      String;
  PrevFFUIDir:        String;
  PrevVersion:        String;

// ---- Registry helpers ----
function GetPreviousUninstallString: String;
var
  sUnInstPath: String;
  sUnInstStr:  String;
begin
  Result := '';
  sUnInstPath := 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}_is1';
  if RegQueryStringValue(HKLM, sUnInstPath, 'UninstallString', sUnInstStr) then
    Result := RemoveQuotes(sUnInstStr);
end;

function GetPreviousVersion: String;
var
  sUnInstPath: String;
  sVer: String;
begin
  Result := '';
  sUnInstPath := 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}_is1';
  if RegQueryStringValue(HKLM, sUnInstPath, 'DisplayVersion', sVer) then
    Result := sVer;
end;

procedure ReadPreviousPaths;
var
  sVal: String;
begin
  PrevPluginDir := '';
  PrevFFUIDir := '';
  if RegQueryStringValue(HKLM, 'SOFTWARE\FFmAdobe', 'PluginPath', sVal) then
    PrevPluginDir := sVal;
  if RegQueryStringValue(HKLM, 'SOFTWARE\FFmAdobe', 'FFmpegFreeUIPath', sVal) then
    PrevFFUIDir := ExtractFilePath(sVal);
end;

// ---- Run the old uninstaller silently ----
function RunPreviousUninstall: Boolean;
var
  ResultCode: Integer;
begin
  Result := True;
  if PrevUninstallStr <> '' then
  begin
    if not Exec(PrevUninstallStr, '/SILENT /NORESTART', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
      Result := False;
  end;
end;

// ---- Default paths ----
function DefaultPluginDir: String;
begin
  if PrevPluginDir <> '' then
    Result := PrevPluginDir
  else
    Result := ExpandConstant('{autopf}\Adobe\Common\Plug-ins\7.0\MediaCore\FFmAdobe');
end;

function DefaultFFUIDir: String;
begin
  if (PrevFFUIDir <> '') and (Pos('Program Files', PrevFFUIDir) = 0) then
    Result := PrevFFUIDir
  else
    Result := ExpandConstant('{localappdata}\FFmAdobe\FFmpegFreeUI');
end;

// ---- {code:} getters ----
function GetPluginDir(Param: String): String;
begin
  if Assigned(PluginDirPage) then
    Result := PluginDirPage.Values[0]
  else
    Result := DefaultPluginDir;
end;

function GetFFUIDir(Param: String): String;
begin
  if Assigned(FFUIDirPage) then
    Result := FFUIDirPage.Values[0]
  else
    Result := DefaultFFUIDir;
end;

// ---- Initialization: detect existing install ----
function InitializeSetup: Boolean;
begin
  Result := True;
  PrevUninstallStr := GetPreviousUninstallString;
  PrevVersion := GetPreviousVersion;
  IsMaintenanceMode := (PrevUninstallStr <> '');
  ReadPreviousPaths;
end;

// ---- Create wizard pages ----
procedure InitializeWizard;
var
  lbl: TNewStaticText;
begin
  // --- Maintenance page (shown only if already installed) ---
  MaintenancePage := CreateCustomPage(
    wpWelcome,
    'FFmAdobe 已安装 (Maintenance)',
    '检测到 FFmAdobe ' + PrevVersion + ' 已安装，请选择操作。');

  lbl := TNewStaticText.Create(MaintenancePage);
  lbl.Parent := MaintenancePage.Surface;
  lbl.Top := 0;
  lbl.Left := 0;
  lbl.Width := MaintenancePage.SurfaceWidth;
  lbl.WordWrap := True;
  lbl.Caption := '请选择要执行的操作：';

  RadioRepair := TNewRadioButton.Create(MaintenancePage);
  RadioRepair.Parent := MaintenancePage.Surface;
  RadioRepair.Top := lbl.Top + lbl.Height + 16;
  RadioRepair.Left := 12;
  RadioRepair.Width := MaintenancePage.SurfaceWidth - 24;
  RadioRepair.Caption := '修复 / 更新  —  重新安装所有文件（保留设置）';
  RadioRepair.Checked := True;
  RadioRepair.Font.Size := 10;

  RadioModify := TNewRadioButton.Create(MaintenancePage);
  RadioModify.Parent := MaintenancePage.Surface;
  RadioModify.Top := RadioRepair.Top + RadioRepair.Height + 12;
  RadioModify.Left := 12;
  RadioModify.Width := MaintenancePage.SurfaceWidth - 24;
  RadioModify.Caption := '修改  —  更改组件选择或安装路径';
  RadioModify.Font.Size := 10;

  RadioUninstall := TNewRadioButton.Create(MaintenancePage);
  RadioUninstall.Parent := MaintenancePage.Surface;
  RadioUninstall.Top := RadioModify.Top + RadioModify.Height + 12;
  RadioUninstall.Left := 12;
  RadioUninstall.Width := MaintenancePage.SurfaceWidth - 24;
  RadioUninstall.Caption := '卸载  —  完全移除 FFmAdobe';
  RadioUninstall.Font.Size := 10;

  // --- Plugin directory page ---
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

  // --- FFmpegFreeUI directory page ---
  FFUIDirPage := CreateInputDirPage(
    PluginDirPage.ID,
    'FFmpegFreeUI 安装目录',
    '请选择 FFmpegFreeUI 前端工具的安装目录。',
    'FFmpegFreeUI 将安装到以下目录。',
    False, '');
  FFUIDirPage.Add('FFmpegFreeUI 目录:');
  FFUIDirPage.Values[0] := DefaultFFUIDir;
end;

// ---- Page flow control ----
function ShouldSkipPage(PageID: Integer): Boolean;
begin
  Result := False;

  // Maintenance page: skip if NOT already installed
  if PageID = MaintenancePage.ID then
  begin
    Result := not IsMaintenanceMode;
    Exit;
  end;

  // If maintenance mode with Repair selected: skip component + path pages (reinstall same config)
  if IsMaintenanceMode and RadioRepair.Checked then
  begin
    if (PageID = wpSelectComponents) or (PageID = PluginDirPage.ID) or (PageID = FFUIDirPage.ID) then
    begin
      Result := True;
      Exit;
    end;
  end;

  // If maintenance mode with Uninstall selected: skip everything except ready
  if IsMaintenanceMode and RadioUninstall.Checked then
  begin
    if (PageID = wpSelectComponents) or (PageID = PluginDirPage.ID) or (PageID = FFUIDirPage.ID) then
    begin
      Result := True;
      Exit;
    end;
  end;

  // Normal/Modify: hide FFUI dir page if component not selected
  if (PageID = FFUIDirPage.ID) and (not WizardIsComponentSelected('ffui')) then
    Result := True;
end;

// ---- Handle maintenance actions ----
function NextButtonClick(CurPageID: Integer): Boolean;
var
  ResultCode: Integer;
begin
  Result := True;

  if CurPageID = MaintenancePage.ID then
  begin
    if RadioUninstall.Checked then
    begin
      // Run uninstaller and abort setup
      if MsgBox('确定要完全卸载 FFmAdobe 吗？' + #13#10 + '用户预设数据也将被删除。',
                mbConfirmation, MB_YESNO) = IDYES then
      begin
        if PrevUninstallStr <> '' then
          Exec(PrevUninstallStr, '/NORESTART', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ResultCode);
        MsgBox('FFmAdobe 已卸载。', mbInformation, MB_OK);
      end;
      Result := False;  // Stay on page / exit setup
      WizardForm.Close;
      Exit;
    end;

    if RadioRepair.Checked then
    begin
      // Silent uninstall old version first, then reinstall
      RunPreviousUninstall;
    end;

    if RadioModify.Checked then
    begin
      // Silent uninstall, then let user reconfigure
      RunPreviousUninstall;
    end;
  end;
end;

// ---- Prepare to install: uninstall old version for fresh install too ----
function PrepareToInstall(var NeedsRestart: Boolean): String;
begin
  Result := '';
  // For new installs over existing (edge case: user ran setup directly)
  // The maintenance flow already handles uninstall via NextButtonClick
end;

// ---- Ready page summary ----
function UpdateReadyMemo(Space, NewLine, MemoUserInfoInfo, MemoDirInfo, MemoTypeInfo, MemoComponentsInfo, MemoGroupInfo, MemoTasksInfo: String): String;
var
  S: String;
begin
  S := '';

  if IsMaintenanceMode and RadioRepair.Checked then
  begin
    S := S + '操作: 修复/更新安装' + NewLine + NewLine;
    S := S + '插件安装目录:' + NewLine;
    S := S + Space + GetPluginDir('') + NewLine + NewLine;
    if PrevFFUIDir <> '' then
    begin
      S := S + 'FFmpegFreeUI 安装目录:' + NewLine;
      S := S + Space + GetFFUIDir('') + NewLine;
    end;
    Result := S;
    Exit;
  end;

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
