//---------------------------------------------------------------------------
#include <vcl.h>
#include <Controls.hpp>
#include "Main.h"
// Include header files common to all files before this directive!
#pragma hdrstop

#include "Paltalk.h"
#include "PaltalkClass.h"
#include "Play.h"
#include "Playback.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------
// Constructor
__fastcall TPaltalkClass::TPaltalkClass(String roomName, TPaltalk* pt)
// roomName is in UTF-8!
{
  this->pt = pt;

  // Init Window Handles
  _roomHwnd = 0;
  _sendHwnd = 0;

  if (!GPaltalk || roomName.IsEmpty())
    return;

  GPaltalk->refreshRooms();

  if (!pt->roomList->Count)
    return;

  int roomIndex = GPaltalk->findRoom(roomName);

  if (roomIndex < 0)
    return;

  _richEditControlHandleList = new TList();

  if (!_richEditControlHandleList)
    return;

  // Found the room, get it's window handle and associated controls...
  _roomHwnd = (HWND)pt->roomList->Objects[roomIndex];

  if (!roomExists())
  {
    pt->roomList->Delete(roomIndex);
    _roomHwnd = NULL;
    return;
  }

  EnumChildWindows(_roomHwnd, (WNDENUMPROC)dlgFindControls, (LPARAM)this);

  // This is stored in Colorize.ini and is the index into a list of
  // rich-edit control handles from one single chat-room's form, that
  // is the send-tect control (and could be changed as time goes on...)
  int idx = pt->PalRichEditIndex;

  if (_richEditControlHandleList->Count)
  {
    if (idx < _richEditControlHandleList->Count)
      _sendHwnd = _richEditControlHandleList->Items[idx];
    else // try 0 to count-1 ???
      _sendHwnd = _richEditControlHandleList->Items[0];
  }
  else
    _sendHwnd = NULL;

  cbRTF = (unsigned short)RegisterClipboardFormat(RTF_REGISTERED_NAME);
}
//---------------------------------------------------------------------------
// Destructor
__fastcall TPaltalkClass::~TPaltalkClass(void)
{
  try
  {
    if (_richEditControlHandleList)
      delete _richEditControlHandleList;
  }
  catch(...) {}
}
//---------------------------------------------------------------------------
BOOL CALLBACK TPaltalkClass::dlgFindControls(HWND hWnd, LPARAM lparam)
// callback function (parameter of EnumChildWindows) to find
// handles of richedits and sysviewlist
{
  if (!lparam)
    return FALSE;

  char tmp[1024];
  GetClassName(hWnd, tmp, 1024);
  String S = String(tmp).LowerCase(); // convert "RichEdit20W" to "richedit20w"

  TPaltalkClass* p = (TPaltalkClass*)lparam;

  // We want a handle to the PalTalk user text-entry control... but we
  // have FIVE rich-edit controls on one chat-room form! Two for Control
  // ID 202, two for Control ID 203 and one for Control ID 1814 which displays
  // "Like Paltalk On FaceBook" at the top...
  //
  // MSFTEDIT is class-name "RICHEDIT50W" (not used in PalTalk as yet), but ALL
  // RichEdit controls class names contain "richedit". "RichEdit20W" is
  // the name of the class we want.
  //
  // Our strategy will be to save all richedit handles that contain
  // "richedit" in a TList and have the index to the one we want stored
  // in Colorize.ini. (so we can change it if PalTalk morphs!)
  if (S.Pos(p->pt->PalTextEntryClass) > 0)
    p->_richEditControlHandleList->Add(hWnd);

  return TRUE;
}
//---------------------------------------------------------------------------
HWND __fastcall TPaltalkClass::getCurrentRoomHandle(void)
{
  // we look up _roomHwnd in _roomList given the room's name in the constructor
  return _roomHwnd;
}
//---------------------------------------------------------------------------
bool __fastcall TPaltalkClass::roomExists(void)
{
  return IsWindow(_roomHwnd);
}
//---------------------------------------------------------------------------
void __fastcall TPaltalkClass::sendCommand(PaltalkCmd cmd)
{
  SendMessageW(_roomHwnd, WM_COMMAND, (int)cmd, 0);
}
//---------------------------------------------------------------------------
bool __fastcall TPaltalkClass::postTextToCurrentRoom(String txt)
// Returns true if no error
// txt is UTF-8...
{
  try
  {
    if (!txt.IsEmpty() && IsWindow(_sendHwnd))
    {
      WideString wTxt = utils->Utf8ToWide(txt);

      DWORD result;
      if (SendMessageTimeoutW(_sendHwnd, WM_SETTEXT, 0, (LPARAM)wTxt.c_bstr(),
                                          SMTO_NORMAL, SMTO_TIMEOUT, &result))
      {
        if(!DTSColor->ShiftHeldOnPlay)
        {
          // Remotely press Enter if SHIFT is NOT pressed (clears old text)
          PostMessageW(_sendHwnd, WM_KEYDOWN, C_CR, 0);
          PostMessageW(_sendHwnd, WM_KEYUP, C_CR, 0);
        }
      }
    }
    else
      return false;
  }
  catch(...) {return false;}

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TPaltalkClass::postRtfTextToCurrentRoom(String txt)
// Returns true if no error
// txt is UTF-8...
{
  try
  {
    if (!IsWindow(_sendHwnd))
      return false;

    if (txt.IsEmpty())
      return true;

    CopyRTFToClipBoard(txt);

    if (!Clipboard()->HasFormat(cbRTF))
      return false;

    DWORD result;

    // on StartPlay in Playback.cpp we call GPaltalk->clearText();
    // and doing the remote press of Enter (below) will also clear the
    // old text. In DoStopPlay in Play.cpp we also call clearText()
    // and clear the clipboard. (so we don't need the following unless testing)
    // Select all
    //  SendMessageTimeout(_sendHwnd, EM_SETSEL, 0, -1, SMTO_NORMAL,
    //                                          SMTO_TIMEOUT, &result);
    // Clear text
    //  SendMessageTimeout(_sendHwnd, EM_REPLACESEL, FALSE, (LPARAM)"",
    //                     SMTO_NORMAL, SMTO_TIMEOUT, &result);

    // Paste in the rtf text
    SendMessageTimeoutW(_sendHwnd, EM_PASTESPECIAL, (WPARAM)cbRTF, 0,
                                        SMTO_NORMAL, SMTO_TIMEOUT, &result);

    if(!DTSColor->ShiftHeldOnPlay)
    {
      // Remotely press Enter if SHIFT is NOT pressed (clears old text)
      PostMessageW(_sendHwnd, WM_KEYDOWN, C_CR, 0);
      PostMessageW(_sendHwnd, WM_KEYUP, C_CR, 0);
    }
    return true;
  }
  catch(...) {return false;}
}
//---------------------------------------------------------------------------
bool __fastcall TPaltalkClass::clearEditControl(void)
// Returns true if no error
{
  try
  {
    if (IsWindow(_sendHwnd))
    {
      DWORD result;
      // Select all
      SendMessageTimeoutW(_sendHwnd, EM_SETSEL, 0, -1, SMTO_NORMAL,
                                                  SMTO_TIMEOUT, &result);
      // Clear text
      SendMessageTimeoutW(_sendHwnd, EM_REPLACESEL, FALSE, (LPARAM)"",
                                        SMTO_NORMAL, SMTO_TIMEOUT, &result);
    }
    else
      return false;
  }
  catch(...) {return false;}

  return true;
}
//---------------------------------------------------------------------------
bool __fastcall TPaltalkClass::CopyRTFToClipBoard(String rtfStr)
// We put two memory handles on the clipboard, one for CF_TEXT and
// one for CF_RTF (which we must register...)
//
// CF_RTF         = 'Rich Text Format';
// CF_RTFNOOBJS   = 'Rich Text Format Without Objects';
// CF_RETEXTOBJ   = 'RichEdit Text and Objects';
{
  try
  {
    if (!OpenClipboard(DTSColor->Handle))
      return false;

    EmptyClipboard();

    // Now set the CF_RTF handle...
    int len = rtfStr.Length()+4;
    HANDLE hMem = GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE, len);

    if (!hMem)
      return false;

    try
    {
      // Move string into global-memory
      CopyMemory(GlobalLock(hMem), rtfStr.c_str(), len);
      // Put the memory-handle on the clipboard
      // DO NOT FREE this memory!
      SetClipboardData(cbRTF, hMem);
      GlobalUnlock(hMem);
    }
    catch(...) {return false;}

    CloseClipboard();
    return true;
  }
  catch(...) {return false;}
}
//---------------------------------------------------------------------------
