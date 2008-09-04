name "RLIB"
outFile "rlibsetup.exe"

LicenseText "You must agree to this license before installing."
LicenseData "COPYING"

InstallDir "$PROGRAMFILES\RLIB"
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Sicom\RLIB" ""
DirText "Select the directory to install RLIB in:"

InstType "Full"
InstType "Minimal"

ComponentText "Select what you wish to install."

Section "-Prerequisites"

ReadRegStr $R1 HKLM Software\GnuWin32\Gd "InstallPath"
IfErrors nogd
goto gtk
nogd:
MessageBox MB_OK "Your system does not appear to have GD installed.$\n$\nPress OK to install it."
SetOutPath "$INSTDIR\Prerequisites"
File "..\Prerequisites\gd-2.0.33-1.exe"
ExecWait "$INSTDIR\Prerequisites\gd-2.0.33-1.exe"

gtk:
ReadRegStr $R1 HKLM Software\GTK\2.0 "GLibVersion"
IfErrors nogtk
goto out
nogtk:
MessageBox MB_OK "Your system does not appear to have GTK+ 2.x installed.$\n$\nPress OK to install it."
SetOutPath "$INSTDIR\Prerequisites"
File "..\Prerequisites\gtk+-2.10.13-setup.exe"
ExecWait "$INSTDIR\Prerequisites\gtk+-2.10.13-setup.exe"

out:
SectionEnd ; end of Prerequisites section

Section "" ; (default, requried section)
SetOutPath "$INSTDIR"
file "rpdf\.libs\librpdf.dll"
file "libsrc\.libs\libr.dll"
file "inputs\odbc\.libs\libr-odbc.dll"
SectionEnd ; end of default section

Section "Development files"
SectionIn 1
SetOutPath "$INSTDIR\include"
file rpdf\rpdf.h
file libsrc\charencoder.h
file libsrc\datetime.h
file libsrc\pcode.h
file libsrc\rlib.h
file libsrc\rlib_input.h
file libsrc\util.h
file libsrc\value.h
SetOutPath "$INSTDIR\lib"
file libsrc\libr.def
file libsrc\libr.lib
SectionEnd

section "Native MySQL input"
SectionIn 1
setOutPath "$INSTDIR"
file inputs\mysql\.libs\libr-mysql.dll
sectionEnd

section "Native PostgreSQL input"
SectionIn 1
setOutPath "$INSTDIR"
file inputs\postgres\.libs\libr-postgres.dll
sectionEnd

section "C# binding"
SectionIn 1
setOutPath "$INSTDIR\csharp"
file bindings\csharp\.libs\librlibcsharp.dll
sectionEnd

Section "Java binding"
SectionIn 1
SetOutPath "$INSTDIR\java"
file bindings\java\.libs\librlibjava.dll
file src\examples\java\SWIGTYPE_p_f_p_rlib_p_void__int.java
file src\examples\java\SWIGTYPE_p_f_p_rlib_p_void__int.class
file src\examples\java\SWIGTYPE_p_rlib.java
file src\examples\java\SWIGTYPE_p_rlib.class
file src\examples\java\SWIGTYPE_p_void.java
file src\examples\java\SWIGTYPE_p_void.class
file src\examples\java\rlibJNI.java
file src\examples\java\rlibJNI.class
file src\examples\java\rlib.java
file src\examples\java\rlib.class
SectionEnd ; end of section 'java'

Section "Perl binding"
SectionIn 1

ReadRegStr $R1 HKLM Software\Perl "BinDir"
IfErrors noperl

DetailPrint "Found Perl at $R1"

nsExec::ExecToStack '"$R1" "-e" "use Config; print $$Config{version};"'
Pop $R0
Pop $R2

StrCpy $R3 "$R2" 5
StrCmp "$R3" "5.10." getperllib noperl510

getperllib:
DetailPrint "Perl 5.10 found, good."
nsExec::ExecToStack '"$R1" "-e" "use Config; print $$Config{archlib};"'
Pop $R0
Pop $R3
StrCmp "$R0" "0" instperl noperl
DetailPrint "Perl 5.10 module directory is $R3"

instperl:
Push "$R3"
goto out

noperl510:
Push "$INSTDIR\perl"
DetailPrint "Perl 5.10.x not installed, found version $R2"
goto out

noperl:
DetailPrint "Perl not installed" 
Push "$INSTDIR\perl"

out:
Pop $R3
SetOutPath "$R3"
file bindings\interfaces\rlib.pm 
SetOutPath "$R3\auto\rlib"
file bindings\perl\.libs\rlib.dll
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Sicom\RLIB" "PerlDir" "$R3"
SectionEnd

Section "PHP binding"
SectionIn 1
ReadRegStr $R0 HKLM Software\PHP "InstallDir"
IfErrors nophp

DetailPrint "PHP found in $R0"
Push "$R0\ext"
goto out

nophp:
DetailPrint "PHP not found"
Push "$INSTDIR\php"

out:
Pop $R1
SetOutPath "$R1"
file bindings\php\php_rlib.dll
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Sicom\RLIB" "PHPDir" "$R1"
SectionEnd

Section "Python binding"
SectionIn 1
nsExec::ExecToStack '"python" "--version"'
Pop $R0
Pop $R1
StrCmp "$R0" "0" checkpythonver nopython

checkpythonver:
StrCpy $R2 "$R1" 11
StrCmp "$R2" "Python 2.5." getsitepkgdir nopython25

getsitepkgdir:
DetailPrint "Python 2.5.x found, good"
SetOutPath "$INSTDIR"
file python.bat
nsExec::ExecToStack '"cmd" "/C" "$INSTDIR\python.bat"'
Pop $R0
Pop $R3
Delete "$INSTDIR\python.bat"
DetailPrint "Python module directory: $R3"
Push "$R3"
goto out

nopython25:
DetailPrint "Python 2.5.x not found, found $R1"
Push "$INSTDIR\python"
goto out

nopython:
DetailPrint "Python 2.5.x not found on $$PATH"
Push "$INSTDIR\python" 
goto out

out:
Pop $R3
SetOutPath "$R3"
file bindings\python\.libs\rlibmodule.dll
file bindings\python\.libs\_rlib.dll
file bindings\interfaces\rlib.py
file bindings\interfaces\rlibcompat.py
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Sicom\RLIB" "PythonDir" "$R3"
SectionEnd

Section "-post" ; (post install section, happens last after any optional sections)
; add any commands that need to happen after any optional sections here
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Sicom\RLIB" "" "$INSTDIR"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\RLIB" "DisplayName" "RLIB (remove only)"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\RLIB" "UninstallString" '"$INSTDIR\uninst.exe"'
; write out uninstaller
WriteUninstaller "$INSTDIR\uninst.exe"
SectionEnd ; end of -post section

; begin uninstall settings/section
UninstallText "This will uninstall RLIB from your system"

Section Uninstall
ReadRegStr $R0 HKEY_LOCAL_MACHINE "SOFTWARE\Sicom\RLIB" "PerlDir"
Delete "$R0\auto\rlib\rlib.dll"
RMDir "$R0\auto\rlib"
Delete "$R0\rlib.pm"
RMDir "$R0"

ReadRegStr $R0 HKEY_LOCAL_MACHINE "SOFTWARE\Sicom\RLIB" "PythonDir"
Delete "$R0\rlibmodule.dll"
Delete "$R0\rlibcompat.py"
Delete "$R0\_rlib.dll"
Delete "$R0\rlib.py"
RMDir "$R0"

ReadRegStr $R0 HKEY_LOCAL_MACHINE "SOFTWARE\Sicom\RLIB" "PHPDir"
Delete "$R0\php_rlib.dll"
RMDir "$R0"

Delete "$INSTDIR\Prerequisites\*.*"
RMDir "$INSTDIR\Prerequisites"
Delete "$INSTDIR\csharp\*.*"
RMDir "$INSTDIR\csharp"
Delete "$INSTDIR\java\*.*"
RMDir "$INSTDIR\java"
Delete "$INSTDIR\lib\*.*"
RMDir "$INSTDIR\lib"
Delete "$INSTDIR\include\*.*"
RMDir "$INSTDIR\include"
Delete "$INSTDIR\*.dll"
Delete "$INSTDIR\uninst.exe"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Sicom\RLIB"
DeleteRegKey /ifempty HKEY_LOCAL_MACHINE "SOFTWARE\Sicom"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\RLIB"
RMDir "$INSTDIR"
SectionEnd ; end of uninstall section
