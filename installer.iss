; Inno Setup 脚本 - 哔哩哔哩下载器一键安装包
; 需要安装 Inno Setup: https://jrsoftware.org/isinfo.php

#define MyAppName "哔哩哔哩视频下载器"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "卓卓世界"
#define MyAppExeName "BilibiliDownloader.exe"
#define MyAppAssocName "B站视频下载器"

[Setup]
; 应用基本信息
AppId={{A8B9C7D6-E5F4-1234-5678-9ABCDEF01234}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
; 输出设置
OutputDir=installer
OutputBaseFilename=BilibiliDownloader_Setup_v{#MyAppVersion}
SetupIconFile=
Compression=lzma
SolidCompression=yes
WizardStyle=modern
; 管理员权限
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog

[Languages]
Name: "ChineseSimplified"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
[Files]
; 主程序
Source: "out\build\x64-Debug\BilibiliDownloader.exe"; DestDir: "{app}"; Flags: ignoreversion
; BBDown.exe（如果存在）
Source: "BBDown.exe"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist

; 自动收集的Qt运行时文件
Source: "out\build\x64-Debug\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "out\build\x64-Debug\platforms\*.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion
Source: "out\build\x64-Debug\styles\*.dll"; DestDir: "{app}\styles"; Flags: ignoreversion
Source: "out\build\x64-Debug\imageformats\*.dll"; DestDir: "{app}\imageformats"; Flags: ignoreversion

; VC++运行时库（可选）
Source: "C:\Windows\System32\vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Windows\System32\msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent