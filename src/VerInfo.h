//---------------------------------------------------------------------------
// VerInfo.h - header file for VerInfo.cpp.
//---------------------------------------------------------------------------
#ifndef VerInfoH
#define VerInfoH
//---------------------------------------------------------------------------
typedef struct tagTVersionInfo
{
  int Major;
  int Minor;
  int Release;
  int Build;
} TVersionInfo;

bool VersionInfo(HMODULE hModule, TVersionInfo& mvi);
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------

