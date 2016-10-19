//---------------------------------------------------------------------------
// VerInfo.cpp - get module version information.
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "VerInfo.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
bool VersionInfo(HMODULE hModule, TVersionInfo& mvi)
{
  // initialize module information structure
  ::memset(&mvi, 0, sizeof(mvi));

  // make sure that HMODULE is valid
  if ((int) hModule < HINSTANCE_ERROR) return false;

  // get name of DLL to use with GetFileVersionInfo()
  TCHAR modulePath[MAX_PATH];
  if (!::GetModuleFileName(hModule, modulePath,
    sizeof(modulePath) / sizeof(TCHAR)))
    return false;

  // get size of version information
  DWORD dummy;
  DWORD size = ::GetFileVersionInfoSize(modulePath, &dummy);
  if (!size) return false;

  // allocate a buffer for version information
  BYTE* buffer = (BYTE*) new BYTE[size];

  // get the version information
  if (!::GetFileVersionInfo(modulePath, 0, size, buffer)) {
    delete[] buffer;
    return false;
    }

  // get the static version information (we could get other information,
  // but this is all we need today)
  VS_FIXEDFILEINFO* vsInfo;
  UINT vsInfoSize;
  if (!::VerQueryValue(buffer, "\\", (LPVOID*) &vsInfo, &vsInfoSize))
  {
    delete[] buffer;
    return false;
  }

  // put version information into returned structure
  mvi.Major = HIWORD(vsInfo->dwFileVersionMS);
  mvi.Minor = LOWORD(vsInfo->dwFileVersionMS);
  mvi.Release = HIWORD(vsInfo->dwFileVersionLS);
  mvi.Build = LOWORD(vsInfo->dwFileVersionLS);

  // clean up and return success
  delete[] buffer;
  return true;
}
//---------------------------------------------------------------------------

