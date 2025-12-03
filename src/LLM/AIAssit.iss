; AIAssit Inno Setup 安装脚本
; 用于创建 Windows 安装程序

#define MyAppName "AIAssit"
#define MyAppVersion "1.17.1"
#define MyAppPublisher "GKG"
#define MyAppURL "https://github.com/yourusername/AIAssit"
#define MyAppExeName "AIAssit.exe"
#define MyAppId "{{A1B2C3D4-E5F6-4A5B-8C9D-0E1F2A3B4C5D}"

[Setup]
; 应用程序基本信息
AppId={#MyAppId}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=
InfoBeforeFile=
InfoAfterFile=
OutputDir=..\..\bin\installer
OutputBaseFilename=AIAssit-Setup-{#MyAppVersion}
SetupIconFile=
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64
UninstallDisplayIcon={app}\{#MyAppExeName}

; 安装程序外观
WizardImageFile=
WizardSmallImageFile=

[Languages]
Name: "chinesesimp"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 6.1; Check: not IsAdminInstallMode

[Files]
; 主程序
Source: "..\..\bin\release\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion

; Qt 核心 DLL (只包含 Release 版本，不包含 Debug 版本)
; 注意：打包前请确保 release 目录中只保留 Release 版本的 DLL（不带 'd' 后缀）
; 建议使用 windeployqt 工具来准备发布文件：windeployqt.exe release\AIAssit.exe
Source: "..\..\bin\release\Qt5Core.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\bin\release\Qt5Gui.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\bin\release\Qt5Network.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\bin\release\Qt5Widgets.dll"; DestDir: "{app}"; Flags: ignoreversion

; cmark DLL
Source: "..\..\bin\release\cmark.dll"; DestDir: "{app}"; Flags: ignoreversion

; Qt platforms 插件目录（只包含 Release 版本）
; 注意：确保只打包不带 'd' 后缀的 DLL
Source: "..\..\bin\release\platforms\qwindows.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion
Source: "..\..\bin\release\platforms\qminimal.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion
Source: "..\..\bin\release\platforms\qoffscreen.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion

; 注意：不要包含 Debug 版本的 DLL（带 'd' 后缀的，如 qwindowsd.dll）
; 建议在打包前清理 release 目录中的 Debug DLL，或使用 windeployqt 工具

; 配置文件模板（如果存在）
Source: "..\..\bin\release\AIAssit\AIModelConfig.json"; DestDir: "{app}\AIAssit"; Flags: ignoreversion onlyifdoesntexist
Source: "..\..\bin\release\AIAssit\FunctionCall.json"; DestDir: "{app}\AIAssit"; Flags: ignoreversion onlyifdoesntexist
;Source: "..\..\bin\release\AIAssit\PromptLibrary.json"; DestDir: "{app}\AIAssit"; Flags: ignoreversion onlyifdoesntexist

; Visual C++ 运行时库（如果需要）
; 注意：如果用户系统没有安装 VC++ Redistributable，需要包含这些文件
; Source: "vcredist\vc_redist.x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Run]
; 安装后运行程序（可选）
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

; 安装 VC++ Redistributable（如果需要）
; Filename: "{tmp}\vc_redist.x64.exe"; Parameters: "/quiet /norestart"; StatusMsg: "Installing Visual C++ Redistributable..."; Check: VCRedistNeedsInstall

[Code]
// 检查是否需要安装 VC++ Redistributable
function VCRedistNeedsInstall: Boolean;
var
  Version: String;
begin
  Result := not RegQueryStringValue(HKEY_LOCAL_MACHINE,
    'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Version', Version);
end;


[UninstallDelete]
; 卸载时删除用户数据（可选，根据需要调整）
Type: filesandordirs; Name: "{app}\AIAssit\LLMChatHistory"
Type: filesandordirs; Name: "{localappdata}\{#MyAppName}"

