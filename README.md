# Files.fm Desktop Client setup

#### Official setup document: https://doc.owncloud.com/desktop/next/appendices/building.html.
#### Official github repository: https://github.com/owncloud/client.
#### Newest installer: https://files.fm/u/eyuvsdcs4

## Setting up KDE Craft

To install KDE Craft, Python 3.6+, Microsoft Visual Studio 2019 and PowerShell 5.0+ must be installed.

For Microsoft Visual Studio 2019, make sure the following components are selected at the minimum:
*	Desktop Development with C++
*	C++ ATL 
*	Windows SDK

## Install KDE Craft

Open PowerShell as admin and run the following:
1.	Set-ExecutionPolicy -Scope CurrentUser RemoteSigned
2.	iex ((new-object net.webclient).DownloadString('https://raw.githubusercontent.com/KDE/craft/master/setup/install_craft.ps1'))

**IMPORTANT**: Choose Microsoft Visual Studio 2019 as the compiler but for everything else accept the defaults.

## Launch the KDE Craft Environment

3.	C:\CraftRoot\craft\craftenv.ps1
4.	craft nsis
5.	craft -i libs/zlib
6.	craft -i libs/openssl
7.	craft --no-cache -i libs/libcurl
8.	Open "C:\CraftRoot\build\libs\libcurl\work\curl-7.78.0\CMakeLists.txt" 
find
cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
after it add
set(CURL_USE_OPENSSL ON)
9.	craft --compile libs/libcurl
10.	craft --add-blueprint-repository https://github.com/filesfm/craft-blueprints-owncloud.git
11.	craft owncloud-client

## Commands for developing

#### Open PowerShell as admin
*	C:\CraftRoot\craft\craftenv.ps1 (allows to work with craft)
#### If everything is up to date:
*	craft owncloud-client
#### If changes have been made in the working branch:
*	craft --install-deps owncloud-client
*	craft --fetch owncloud-client
*	craft --configure --make --install 
*	craft owncloud-client
#### To switch branches(for example to build the 2.10 branch):
*	git checkout 2.10
*	craft --set version=2.10 owncloud-client

#### To compile locally modified changes:
*	craft --compile --install --qmerge owncloud-client
_The compiled result can be found under "C:\CraftRoot\build\owncloud\owncloud-client\work\build\bin" and changes tested from C:\CraftRoot\bin_

#### To create an installer execute: 
*	craft --package owncloud-client
_The result can be found under "C:\CraftRoot\tmp"_

# Local code signing

1)	Edit "C:\CraftRoot\etc\CraftSettings.ini", navigate to [CodeSigning] and change it to:

```
Enabled = True
Protected = False
SignCache = ${CodeSigning:Enabled}
CommonName = Files.fm
Organization = Files.fm
Street = 
Locality = Riga
Country = LV
State = 
PostalCode =

```
2)	Run this script in PowerShell as admin:

``` 

#
# This script will create and install two certificates:
#     1. `MyCA.cer`: A self-signed root authority certificate. 
#     2. `MySPC.cer`: The cerificate to sign code in 
#         a development environment (signed with `MyCA.cer`).
# 
# No user interaction is needed (unattended). 
# Powershell 4.0 or higher is required.
#

# Define the expiration date for certificates.
$notAfter = (Get-Date).AddYears(10)

# Create a self-signed root Certificate Authority (CA).
$rootCert = New-SelfSignedCertificate -KeyExportPolicy Exportable -CertStoreLocation Cert:\CurrentUser\My -DnsName "My CA" -NotAfter $notAfter -TextExtension @("2.5.29.37={text}1.3.6.1.5.5.7.3.3", "2.5.29.19={text}CA=1") -KeyusageProperty All -KeyUsage CertSign, CRLSign, DigitalSignature

# Export the CA private key.
[System.Security.SecureString] $password = ConvertTo-SecureString -String "passwordx" -Force -AsPlainText
[String] $rootCertPath = Join-Path -Path cert:\CurrentUser\My\ -ChildPath "$($rootcert.Thumbprint)"
Export-PfxCertificate -Cert $rootCertPath -FilePath "MyCA.pfx" -Password $password
Export-Certificate -Cert $rootCertPath -FilePath "MyCA.crt"

# Create an end certificate signed by our CA.
$cert = New-SelfSignedCertificate -CertStoreLocation Cert:\LocalMachine\My -DnsName "Files.fm" -NotAfter $notAfter -Signer $rootCert -Type CodeSigningCert -TextExtension @("2.5.29.37={text}1.3.6.1.5.5.7.3.3", "2.5.29.19={text}CA=0&pathlength=0")

# Save the signed certificate with private key into a PFX file and just the public key into a CRT file.
[String] $certPath = Join-Path -Path cert:\LocalMachine\My\ -ChildPath "$($cert.Thumbprint)"
Export-PfxCertificate -Cert $certPath -FilePath "MySPC.pfx" -Password $password
Export-Certificate -Cert $certPath -FilePath "MySPC.crt"

# Add MyCA certificate to the Trusted Root Certification Authorities.
$pfx = new-object System.Security.Cryptography.X509Certificates.X509Certificate2
$pfx.import("MyCA.pfx", $password, "Exportable,PersistKeySet")
$store = new-object System.Security.Cryptography.X509Certificates.X509Store(
    [System.Security.Cryptography.X509Certificates.StoreName]::Root,
    "localmachine"
)
$store.open("MaxAllowed")
$store.add($pfx)
$store.close()

# Remove MyCA from CurrentUser to avoid issues when signing with "signtool.exe /a ..."
Remove-Item -Force "cert:\CurrentUser\My\$($rootCert.Thumbprint)"

# Import certificate.
Import-PfxCertificate -FilePath MySPC.pfx cert:\CurrentUser\My -Password $password -Exportable

```

3)	Rename MyCA.pfx to mycert.pfx and place it in C:\CraftRoot

# Editing the installer

The script responsible for making the installer can be found at C:\CraftRoot\craft\bin\Packager\Nsis\NullsoftInstaller.nsi

Add this script into C:\CraftRoot\craft\bin\Packager\Nsis to get file association helper macros 

```
; fileassoc.nsh
; File association helper macros
; Written by Saivert
; 
; Improved by Nikku<https://github.com/nikku>.
;
; Features automatic backup system and UPDATEFILEASSOC macro for
; shell change notification.
;
; |> How to use <|
; To associate a file with an application so you can double-click it in explorer, use
; the APP_ASSOCIATE macro like this:
;
;   Example:
;   !insertmacro APP_ASSOCIATE "txt" "myapp.textfile" "Description of txt files" \
;     "$INSTDIR\myapp.exe,0" "Open with myapp" "$INSTDIR\myapp.exe $\"%1$\""
;
; Never insert the APP_ASSOCIATE macro multiple times, it is only ment
; to associate an application with a single file and using the
; the "open" verb as default. To add more verbs (actions) to a file
; use the APP_ASSOCIATE_ADDVERB macro.
;
;   Example:
;   !insertmacro APP_ASSOCIATE_ADDVERB "myapp.textfile" "edit" "Edit with myapp" \
;     "$INSTDIR\myapp.exe /edit $\"%1$\""
;
; To have access to more options when registering the file association use the
; APP_ASSOCIATE_EX macro. Here you can specify the verb and what verb is to be the
; standard action (default verb).
;
; Note, that this script takes into account user versus global installs.
; To properly work you must initialize the SHELL_CONTEXT variable via SetShellVarContext.
;
; And finally: To remove the association from the registry use the APP_UNASSOCIATE
; macro. Here is another example just to wrap it up:
;   !insertmacro APP_UNASSOCIATE "txt" "myapp.textfile"
;
; |> Note <|
; When defining your file class string always use the short form of your application title
; then a period (dot) and the type of file. This keeps the file class sort of unique.
;   Examples:
;   Winamp.Playlist
;   NSIS.Script
;   Photoshop.JPEGFile
;
; |> Tech info <|
; The registry key layout for a global file association is:
;
; HKEY_LOCAL_MACHINE\Software\Classes
;     <".ext"> = <applicationID>
;     <applicationID> = <"description">
;         shell
;             <verb> = <"menu-item text">
;                 command = <"command string">
;
;
; The registry key layout for a per-user file association is:
;
; HKEY_CURRENT_USER\Software\Classes
;     <".ext"> = <applicationID>
;     <applicationID> = <"description">
;         shell
;             <verb> = <"menu-item text">
;                 command = <"command string">
;

!macro APP_ASSOCIATE EXT FILECLASS DESCRIPTION ICON COMMANDTEXT COMMAND
  ; Backup the previously associated file class
  ReadRegStr $R0 SHELL_CONTEXT "Software\Classes\.${EXT}" ""
  WriteRegStr SHELL_CONTEXT "Software\Classes\.${EXT}" "${FILECLASS}_backup" "$R0"

  WriteRegStr SHELL_CONTEXT "Software\Classes\.${EXT}" "" "${FILECLASS}"

  WriteRegStr SHELL_CONTEXT "Software\Classes\${FILECLASS}" "" `${DESCRIPTION}`
  WriteRegStr SHELL_CONTEXT "Software\Classes\${FILECLASS}\DefaultIcon" "" `${ICON}`
  WriteRegStr SHELL_CONTEXT "Software\Classes\${FILECLASS}\shell" "" "open"
  WriteRegStr SHELL_CONTEXT "Software\Classes\${FILECLASS}\shell\open" "" `${COMMANDTEXT}`
  WriteRegStr SHELL_CONTEXT "Software\Classes\${FILECLASS}\shell\open\command" "" `${COMMAND}`
!macroend

!macro APP_ASSOCIATE_EX EXT FILECLASS DESCRIPTION ICON VERB DEFAULTVERB SHELLNEW COMMANDTEXT COMMAND
  ; Backup the previously associated file class
  ReadRegStr $R0 SHELL_CONTEXT "Software\Classes\.${EXT}" ""
  WriteRegStr SHELL_CONTEXT "Software\Classes\.${EXT}" "${FILECLASS}_backup" "$R0"

  WriteRegStr SHELL_CONTEXT "Software\Classes\.${EXT}" "" "${FILECLASS}"
  StrCmp "${SHELLNEW}" "0" +2
  WriteRegStr SHELL_CONTEXT "Software\Classes\.${EXT}\ShellNew" "NullFile" ""

  WriteRegStr SHELL_CONTEXT "Software\Classes\${FILECLASS}" "" `${DESCRIPTION}`
  WriteRegStr SHELL_CONTEXT "Software\Classes\${FILECLASS}\DefaultIcon" "" `${ICON}`
  WriteRegStr SHELL_CONTEXT "Software\Classes\${FILECLASS}\shell" "" `${DEFAULTVERB}`
  WriteRegStr SHELL_CONTEXT "Software\Classes\${FILECLASS}\shell\${VERB}" "" `${COMMANDTEXT}`
  WriteRegStr SHELL_CONTEXT "Software\Classes\${FILECLASS}\shell\${VERB}\command" "" `${COMMAND}`
!macroend

!macro APP_ASSOCIATE_ADDVERB FILECLASS VERB COMMANDTEXT COMMAND
  WriteRegStr SHELL_CONTEXT "Software\Classes\${FILECLASS}\shell\${VERB}" "" `${COMMANDTEXT}`
  WriteRegStr SHELL_CONTEXT "Software\Classes\${FILECLASS}\shell\${VERB}\command" "" `${COMMAND}`
!macroend

!macro APP_ASSOCIATE_REMOVEVERB FILECLASS VERB
  DeleteRegKey SHELL_CONTEXT `Software\Classes\${FILECLASS}\shell\${VERB}`
!macroend


!macro APP_UNASSOCIATE EXT FILECLASS
  ; Backup the previously associated file class
  ReadRegStr $R0 SHELL_CONTEXT "Software\Classes\.${EXT}" `${FILECLASS}_backup`
  WriteRegStr SHELL_CONTEXT "Software\Classes\.${EXT}" "" "$R0"

  DeleteRegKey SHELL_CONTEXT `Software\Classes\${FILECLASS}`
!macroend

!macro APP_ASSOCIATE_GETFILECLASS OUTPUT EXT
  ReadRegStr ${OUTPUT} SHELL_CONTEXT "Software\Classes\.${EXT}" ""
!macroend


; !defines for use with SHChangeNotify
!ifdef SHCNE_ASSOCCHANGED
!undef SHCNE_ASSOCCHANGED
!endif
!define SHCNE_ASSOCCHANGED 0x08000000
!ifdef SHCNF_FLUSH
!undef SHCNF_FLUSH
!endif
!define SHCNF_FLUSH        0x1000

!macro UPDATEFILEASSOC
; Using the system.dll plugin to call the SHChangeNotify Win32 API function so we
; can update the shell.
  System::Call "shell32::SHChangeNotify(i,i,i,i) (${SHCNE_ASSOCCHANGED}, ${SHCNF_FLUSH}, 0, 0)"
!macroend
```
Edited Files.fm script:
```
; Copyright 2010 Patrick Spendrin <ps_ml@gmx.de>
; Copyright 2016 Kevin Funk <kfunk@kde.org>
; Copyright Hannah von Reth <vonreth@kde.org>
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions
; are met:
; 1. Redistributions of source code must retain the above copyright
;    notice, this list of conditions and the following disclaimer.
; 2. Redistributions in binary form must reproduce the above copyright
;    notice, this list of conditions and the following disclaimer in the
;    documentation and/or other materials provided with the distribution.
;
; THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
; ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
; ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
; FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
; DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
; OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
; HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
; LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
; OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
; SUCH DAMAGE.

; registry stuff
!define regkey "Software\@{company}\@{productname}"
!define uninstkey "Software\Microsoft\Windows\CurrentVersion\Uninstall\@{productname}"

BrandingText "Generated by Craft https://community.kde.org/Craft"

;--------------------------------

XPStyle on
ManifestDPIAware true


Name "@{productname}"
Caption "@{productname} setup"

OutFile "@{setupname}"

!define MULTIUSER_EXECUTIONLEVEL Highest
!define MULTIUSER_MUI
!define MULTIUSER_INSTALLMODE_COMMANDLINE
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_KEY "${regkey}"
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_VALUENAME "Install_Mode"
!define MULTIUSER_INSTALLMODE_INSTDIR "@{productname}"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_KEY "${regkey}"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_VALUENAME "Install_Dir"

;Start Menu Folder Page Configuration
Var StartMenuFolder
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "SHCTX"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${regkey}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

;!define MULTIUSER_USE_PROGRAMFILES64
@{multiuser_use_programfiles64}
;!define MULTIUSER_USE_PROGRAMFILES64

@{nsis_include_internal}
@{nsis_include}

!include "MultiUser.nsh"
!include "MUI2.nsh"
!include "LogicLib.nsh"
!include "x64.nsh"
!include "process.nsh"
!include "fileassoc.nsh"
!include Library.nsh
!include LogicLib.nsh 

;!define MUI_ICON
@{installerIcon}
;!define MUI_ICON

!insertmacro MUI_PAGE_WELCOME

;!insertmacro MUI_PAGE_LICENSE
@{license}
;!insertmacro MUI_PAGE_LICENSE

;!insertmacro MUI_FINISHPAGE_SHOWREADME
@{readme}
;!insertmacro MUI_FINISHPAGE_SHOWREADME

!insertmacro MULTIUSER_PAGE_INSTALLMODE
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder

!define MUI_COMPONENTSPAGE_NODESC
;!insertmacro MUI_PAGE_COMPONENTS
@{sections_page}
;!insertmacro MUI_PAGE_COMPONENTS

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!define MUI_FINISHPAGE_LINK "Visit project homepage"
!define MUI_FINISHPAGE_LINK_LOCATION "@{website}"
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_FUNCTION myrun
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

SetDateSave on
SetDatablockOptimize on
CRCCheck on
SilentInstall normal

Function myrun
    Exec '"$WINDIR\System32\regsvr32.exe" /s "$INSTDIR\FMOverlays.dll"'
    Exec '"$WINDIR\System32\regsvr32.exe" /s "$INSTDIR\FMContextMenu.dll"'
    MessageBox MB_YESNO "To make sure the Explorer integration is working you$\nneed to restart your system. To restart now click Yes, or$\nclick No if you plan to manually restart at a later time." IDYES true IDNO false
    true:
        WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\RunOnce" "Files.fm Sync" '"$InstDir\files.fm-sync.exe"'
        Reboot   
    false:
        ExecShell "" "files.fm-sync.exe"
        Quit
FunctionEnd

Function .onGUIEnd
    Exec '"$WINDIR\System32\regsvr32.exe" /s "$INSTDIR\FMOverlays.dll"'
    Exec '"$WINDIR\System32\regsvr32.exe" /s "$INSTDIR\FMContextMenu.dll"'
    MessageBox MB_YESNO|MB_DEFBUTTON2 "To make sure the Explorer integration is working you$\nneed to restart your system. To restart now click Yes, or$\nclick No if you plan to manually restart at a later time." IDYES true IDNO false
    true:
        Reboot
    false:
FunctionEnd

Function .onInit
    !insertmacro MULTIUSER_INIT
    !insertmacro APP_ASSOCIATE "filesfm" "files.fm-sync.filesfmfile" "FILESFM Files" \
    "$INSTDIR\files.fm-sync.exe,0" "Open with files.fm-sync" "$INSTDIR\files.fm-sync.exe $\"%1$\""
    !define LIBRARY_COM
    !define LIBRARY_SHELL_EXTENSION
    !define LIBRARY_IGNORE_VERSION
    !undef LIBRARY_COM
    !undef LIBRARY_SHELL_EXTENSION
    !undef LIBRARY_IGNORE_VERSION
    !if "@{architecture}" == "x64"
        ${IfNot} ${RunningX64}
            MessageBox MB_OK|MB_ICONEXCLAMATION "This installer can only be run on 64-bit Windows."
            Abort
        ${EndIf}
    !endif
FunctionEnd

Function un.onInit
    !insertmacro MULTIUSER_UNINIT
FunctionEnd

Function un.RebootWindowsExplorer
  MessageBox MB_YESNO "To make sure that all Files.fm Sync files are deleted$\nWindows Explorer must be restarted. To continue click Yes, or$\nclick No to quit." IDYES true IDNO false
    false:
        Quit
    true:
        !insertmacro APP_UNASSOCIATE "FILESFM" "files.fm-sync.filesfmfile"
        Exec '"$WINDIR\System32\regsvr32.exe" /s /u "$INSTDIR\FMOverlays.dll"'
        Exec '"$WINDIR\System32\regsvr32.exe" /s /u "$INSTDIR\FMContextMenu.dll"'
        nsExec::Exec 'cmd /c "taskkill /F /IM explorer.exe"'
        nsExec::Exec 'cmd /c "$WINDIR\explorer.exe"'
FunctionEnd
;--------------------------------

AutoCloseWindow false

; beginning (invisible) section
Section
  !insertmacro EndProcessWithDialog
  ExecWait '"$MultiUser.InstDir\uninstall.exe" /S _?=$MultiUser.InstDir'
  @{preInstallHook}
  WriteRegStr SHCTX "${regkey}" "Install_Dir" "$INSTDIR"
  WriteRegStr SHCTX "${MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_KEY}" "${MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_VALUENAME}" "$MultiUser.InstallMode"
  ; write uninstall strings
  WriteRegStr SHCTX "${uninstkey}" "DisplayName" "@{productname}"
  WriteRegStr SHCTX "${uninstkey}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr SHCTX "${uninstkey}" "DisplayIcon" "$INSTDIR\@{iconname}"
  WriteRegStr SHCTX "${uninstkey}" "URLInfoAbout" "@{website}"
  WriteRegStr SHCTX "${uninstkey}" "Publisher" "@{company}"
  WriteRegStr SHCTX "${uninstkey}" "DisplayVersion" "@{version}"
  WriteRegDWORD SHCTX "${uninstkey}" "EstimatedSize" "@{estimated_size}"

  @{registry_hook}

  SetOutPath $INSTDIR


; package all files, recursively, preserving attributes
; assume files are in the correct places

File /a "@{dataPath}"
File /a "@{7za}"
File /a "@{icon}"
nsExec::ExecToLog '"$INSTDIR\7za.exe" x -r -y "$INSTDIR\@{dataName}" -o"$INSTDIR"'
Delete "$INSTDIR\7za.exe"
Delete "$INSTDIR\@{dataName}"

AddSize @{installSize}

WriteUninstaller "uninstall.exe"

SectionEnd

; create shortcuts
@{shortcuts}

;  allow to define additional sections
@{sections}

; Uninstaller
; All section names prefixed by "Un" will be in the uninstaller

UninstallText "This will uninstall @{productname}."

Section "Uninstall"
!insertmacro EndProcessWithDialog

DeleteRegKey SHCTX "${uninstkey}"
DeleteRegKey SHCTX "${regkey}"

!insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
call un.RebootWindowsExplorer
RMDir /r "$SMPROGRAMS\$StartMenuFolder"


@{uninstallFiles}
@{uninstallDirs}
SectionEnd

;  allow to define additional Un.sections
@{un_sections}
```
