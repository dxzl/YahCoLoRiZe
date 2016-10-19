//---------------------------------------------------------------------------
#ifndef PaltalkH
#define PaltalkH

#include <Classes.hpp>

// Each chat-window has five or more child-windows two of which are
// RichEdit20W controls. For the text-entry box...
// (should be in the INI file!)
// NOTE: paltalk keeps updating and changing the order of the richedit's!
#define PAL_RICHEDITINDEX 0

// Full class name is "DlgGroupChat Window Class"
#define PAL_CHATROOMCLASS ("dlggroupchat") // we just look for this part...
// Full class name is "RichEdit20W"
#define PAL_TEXTENTRYCLASS ("richedit") // we just look for this part...
//---------------------------------------------------------------------------

class TPaltalk
{
  friend class TPaltalkClass;

protected:

  static BOOL CALLBACK dlgFindRooms(HWND hWnd, LPARAM lparam);

private:

  bool __fastcall readPaltalkIniFile(void);

  TPaltalkClass* _paltalkService;
  int _currentRoomIndex;
  TStringList* _roomList;

  String _palChatRoomClass, _palTextEntryClass;

  // index into a list of 5 hWnd handles in list of controls for
  // one chat-room's form
  int _palRichEditIndex;

public:
  __fastcall TPaltalk(void);
  __fastcall ~TPaltalk(void);

  int __fastcall findRoom(String roomName); // roomName is UTF-8
  TStringList* __fastcall getAllRoomsAndPms(void);
  bool __fastcall sendTextToRoomOrPm(String name,
                                   String text, bool bRtf = false);
  int __fastcall refreshRooms(void);
  int __fastcall getRoomCount(void);
  String __fastcall getCurrentRoomOrPmName(void);
  String __fastcall getRoomOrPmName(int idx);
  bool __fastcall clearText(String name);
  bool __fastcall clearText(void);
  void __fastcall deletePaltalkService(void);
  bool __fastcall deleteServiceIfRoomGone(void);
  TPaltalkClass* __fastcall getPaltalkService(int roomIdx);
  TPaltalkClass* __fastcall getPaltalkService(String roomName);
  TPaltalkClass* __fastcall getPaltalkService(void);

  __property int CurrentRoomIndex =
          { read = _currentRoomIndex, write = _currentRoomIndex };
  __property TStringList* roomList = { read = _roomList };
  __property int PalRichEditIndex = { read = _palRichEditIndex };
  __property String PalChatRoomClass = { read = _palChatRoomClass };
  __property String PalTextEntryClass = { read = _palTextEntryClass };
};
//---------------------------------------------------------------------------
  extern TPaltalk* GPaltalk;
//---------------------------------------------------------------------------
#endif
