//---------------------------------------------------------------------------
// YahCoLoRiZe - Edit, process and send colorized text into IRC chat-rooms
// via various chat-clients such as mIRC, IceCHat and LeafCHat
//
// Author: Scott Swift
//
// Released to GitHub under GPL v3 October, 2016
//
//---------------------------------------------------------------------------
#include <vcl.h>
#include <Controls.hpp>
#include "Main.h"
// Include header files common to all files before this directive!
#pragma hdrstop

#include "Paltalk.h"
#include "PaltalkClass.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------
TPaltalk* GPaltalk;

//---------------------------------------------------------------------------
// Constructor
__fastcall TPaltalk::TPaltalk(void)
{
  _paltalkService = NULL;
  _currentRoomIndex = -1;

  _roomList = new TStringList();

  GPaltalk = this;

  readPaltalkIniFile(); // Get the window class-names and index of send-handle
}
//---------------------------------------------------------------------------
// Destructor
__fastcall TPaltalk::~TPaltalk(void)
{
  try
  {
    if (_roomList)
      delete _roomList;
  } catch(...) {}

  try
  {
    if (_paltalkService)
      delete _paltalkService;
  } catch(...) {}

  GPaltalk = NULL;
}
//---------------------------------------------------------------------------
//callback (parameter of  EnumWindows) function to find rooms' windows handles
BOOL CALLBACK TPaltalk::dlgFindRooms(HWND hWnd, LPARAM lparam)
// callback (parameter of  EnumWindows) function to find rooms'
// windows handles
{
  if (!lparam)
    return FALSE;

  // Get the class-name in UTF-16 string S
  char tmp[1024];
  GetClassName(hWnd, tmp, 1024);
  String sClass = String(tmp).LowerCase();

  TPaltalk* p = (TPaltalk*)lparam;

  // Each open chat-window's class is "DlgGroupChat Window Class"
  // (we only look for "DlgGroupChat")
  if (sClass.Pos(p->PalChatRoomClass) > 0) // (1-based index!)
  {
    wchar_t wtmp[1024];

    // We store the room and its handle as-is, in UTF-8, and parse stuff later
    if (GetWindowTextW(hWnd, wtmp, 1024))
    {
      // 100 rooms max!
      if (p->roomList->Count < MAXROOMHISTORY)
        p->roomList->AddObject(utils->WideToUtf8(WideString(wtmp)),
                                                       (TObject*)hWnd);
    }
  }

  return TRUE;
}
//---------------------------------------------------------------------------
/// <summary>
/// Get all room and pm window names
/// </summary>
/// <returns></returns>
TStringList* __fastcall TPaltalk::getAllRoomsAndPms(void)
{
  int count = refreshRooms();

  if (!count)
  {
    deletePaltalkService();
    _currentRoomIndex = -1;
    return NULL;
  }

  return _roomList;
}
//---------------------------------------------------------------------------
int __fastcall TPaltalk::refreshRooms(void)
{
  _roomList->Clear();

  // Enumaeate the paltalk windows (see above callback function...)
  EnumWindows((WNDENUMPROC)dlgFindRooms, (LPARAM)this);

  return _roomList->Count;
}
//---------------------------------------------------------------------------
int __fastcall TPaltalk::getRoomCount(void)
{
  return _roomList->Count;
}
//---------------------------------------------------------------------------
String __fastcall TPaltalk::getCurrentRoomOrPmName(void)
{
  return getRoomOrPmName(_currentRoomIndex);
}
//---------------------------------------------------------------------------
String __fastcall TPaltalk::getRoomOrPmName(int idx)
{
  try
  {
    if (_roomList->Count > 0 && idx >= 0 && idx < _roomList->Count)
      return _roomList->Strings[idx];
  }
  catch(...){}

  return "";
}
//---------------------------------------------------------------------------
// Overloaded
bool __fastcall TPaltalk::clearText(void)
// Return true if all goes well...
{
  return clearText(getCurrentRoomOrPmName());
}

bool __fastcall TPaltalk::clearText(String name)
// Return true if all goes well...
{
  TPaltalkClass* pts = this->getPaltalkService(name);

  if (pts)
  {
    // Did we find the room? If so the handle will be non-zero
    // If not, delete the instance.
    if (pts->getCurrentRoomHandle())
      return pts->clearEditControl();

    delete pts;
    _currentRoomIndex = -1;
    _paltalkService = NULL;
  }

  return false;
}
//---------------------------------------------------------------------------
bool __fastcall TPaltalk::sendTextToRoomOrPm(String name,
                                                String text, bool bRtf)
// Return true if all goes well...
// name and text are in UTF-8...
{
  TPaltalkClass* pts = this->getPaltalkService(name);

  if (pts)
  {
    // Did we find the room? If so the handle will be non-zero
    // If not, delete the instance.
    if (pts->getCurrentRoomHandle())
    {
      // Only need this if we will NOT do a remote press of ENTER!
      if(DTSColor->ShiftHeldOnPlay)
        pts->clearEditControl();

      if (bRtf)
        return pts->postRtfTextToCurrentRoom(text);

      return pts->postTextToCurrentRoom(text);
    }
    else
    {
      deletePaltalkService(); // delete the only service
      return false;
    }
  }
  else
    return false;
}
//---------------------------------------------------------------------------
TPaltalkClass* __fastcall TPaltalk::getPaltalkService(void)
// Instantiates new TPaltalk class for current room (if needed)
{
  return getPaltalkService(_currentRoomIndex);
}

TPaltalkClass* __fastcall TPaltalk::getPaltalkService(int roomIdx)
// Instantiates new TPaltalk class if new room and deletes old class object
{
  String roomName;

  try
  {
    if (roomIdx >= 0 && roomIdx < _roomList->Count)
      roomName = _roomList->Strings[roomIdx];
  } catch(...){}

  if (roomName.IsEmpty())
    return NULL;

  return getPaltalkService(roomName);
}

TPaltalkClass* __fastcall TPaltalk::getPaltalkService(String roomName)
// Instantiates new TPaltalk class if new room and deletes old class object
{
  String cr = getCurrentRoomOrPmName();

  if (cr.IsEmpty() || roomName != cr)
    deletePaltalkService();

  if (_paltalkService == NULL)
    _paltalkService = new TPaltalkClass(roomName, this);

  return _paltalkService;
}
//---------------------------------------------------------------------------
bool __fastcall TPaltalk::deleteServiceIfRoomGone(void)
{
  if (!_paltalkService)
  {
    _currentRoomIndex = -1;
    return false;
  }

  if (!_paltalkService->roomExists())
  {
    deletePaltalkService();
    return false;
  }

  return true;
}
//---------------------------------------------------------------------------
void __fastcall TPaltalk::deletePaltalkService(void)
{
  if (_paltalkService)
  {
    try
    {
      delete _paltalkService;
    }
    catch(...) {}
  }

  _currentRoomIndex = -1;
  _paltalkService = NULL;
}
//---------------------------------------------------------------------------
int __fastcall TPaltalk::findRoom(String roomName)
// Pass in name as UTF-8
// Returns index into the room-list or -1
{
  if (_roomList->Count && !roomName.IsEmpty())
    for(int ii = 0; ii < _roomList->Count; ii++)
      if (_roomList->Strings[ii] == roomName)
        return ii;

  return -1;
}
//---------------------------------------------------------------------------
bool __fastcall TPaltalk::readPaltalkIniFile(void)
// Reads the colorize.ini file to get the Paltalk section
{
  bool bRet;

  TIniFile *pIni = NULL;

  try
  {
    try
    {
      pIni = utils->OpenIniFile();

      if (pIni == NULL)
        return false;

      _palRichEditIndex =
        pIni->ReadInteger("Paltalk", "richeditindex", PAL_RICHEDITINDEX);

      _palChatRoomClass =
        pIni->ReadString("paltalk", "chatroomclass", PAL_CHATROOMCLASS);

      _palTextEntryClass =
        pIni->ReadString("paltalk", "textentryclass", PAL_TEXTENTRYCLASS);

      bRet = true;
    }
    catch(const std::exception& e)
    {
      printf("ReadPaltalkIniFile() Exception: %s :\n", e.what());
      bRet = false;
    }
  }
  __finally
  {
    try
    {
      if (pIni != NULL)
        delete pIni;
    }
    catch(...) {}
  }

  return bRet;
}
//---------------------------------------------------------------------------

