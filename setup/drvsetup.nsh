/*
  BOOL UpdateDriverForPlugAndPlayDevices(
    _In_opt_   HWND hwndParent,
    _In_       LPCTSTR HardwareId,
    _In_       LPCTSTR FullInfPath,
    _In_       DWORD InstallFlags,
    _Out_opt_  PBOOL bRebootRequired
  );
*/
!define sysUpdateDriverForPlugAndPlayDevices "newdev::UpdateDriverForPlugAndPlayDevices(i, t, t, i, *i) i"
!define NO_ERROR              0
!define ERROR_FILE_NOT_FOUND  2
!define ERROR_IN_WOW64        -536870347  ; 0xE0000235
!define ERROR_INVALID_FLAGS   1004        ; 0x3EC
!define ERROR_NO_SUCH_DEVINST -536870389  ; 0xE000020B
; the masked value of ERROR_NO_SUCH_DEVINST is 523
 
!define INSTALLFLAG_FORCE           0x1
!define INSTALLFLAG_READONLY        0x2
!define INSTALLFLAG_NONINTERACTIVE  0x4
; Use FORCE flag ex: pushing an older release over top of new one.
 
 
;BOOL SetupCopyOEMInf(PSTR, PSTR, DWORD, DWORD, PSTR, DWORD, PDWORD, PSTR);
!define sysSetupCopyOEMInf "setupapi::SetupCopyOEMInf(t, t, i, i, i, i, *i, t) i"
!define SPOST_NONE 0
!define SPOST_PATH 1
!define SPOST_URL 2
!define SP_COPY_DELETESOURCE 0x1
!define SP_COPY_REPLACEONLY 0x2
!define SP_COPY_NOOVERWRITE 0x8
!define SP_COPY_OEMINF_CATALOG_ONLY 0x40000
