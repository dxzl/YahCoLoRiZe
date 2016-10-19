//---------------------------------------------------------------------------
#ifndef PaltalkClassH
#define PaltalkClassH

#include <Classes.hpp>

#define MAX_LVMSTRING 255
//---------------------------------------------------------------------------

// Some of these values may be obsolete
enum PaltalkCmd
{
  CMD_REMOVE_HANDS = 32995,
  CMD_REMOVE_MICS = 32996,
  CMD_GIVE_MICS = 32997,
  CMD_ADMIN_CONSOLE = 32998,
  CMD_BOUNCE = 32947,
  CMD_REDDOT = 32946, // OnMic raise hand
  CMD_LOWER_HAND = 32950,
  CMD_VOICE_ACTIVATED = 33344,
  CMD_PUSH_TALK = 33343,
  CMD_LOCK_MIC = 33340,
  CMD_FREEZE_TEXT = 210,
  CMD_ROOM_IGNORE = 33003,
  CMD_ROOM_START_PM = 32901,
  CMD_ROOM_ADD_USER = 32898,
  CMD_ROOM_BLOCK_USER = 33381,
  CMD_INVIT_USERS = 32989,
  CMD_INSERT_NICK = 33379,
  CMD_EXIT_ROOM = 32849
};

enum PaltalkUserStat
{
  STAT_NONE = 0,
  STAT_REDDOT = 5,
  STAT_CAM_ON = 1,
  STAT_ON_MIC = 10,
  STAT_R_HAND = 20,
  STAT_ERROR = 255 // added by me to use in case of error, not from paltalk
};

//---------------------------------------------------------------------

class TPaltalkClass
{
protected:
  // callback function (parameter of EnumChildWindows) to find controls
  static BOOL CALLBACK dlgFindControls(HWND hWnd, LPARAM lparam);

private:

  int __fastcall getUserNameIndex(String strUser);
  bool __fastcall CopyRTFToClipBoard(String rtfStr );

  HWND _roomHwnd, _sendHwnd;

  unsigned short cbRTF; // Registered clipboard format

  TPaltalk* pt;
  TList* _richEditControlHandleList;

public:
  __fastcall TPaltalkClass(String roomName, TPaltalk* pt);
  __fastcall ~TPaltalkClass(void);

  bool __fastcall clearEditControl(void);
  bool __fastcall postTextToCurrentRoom(String txt);
  bool __fastcall postRtfTextToCurrentRoom(String txt);
  void __fastcall sendCommand(PaltalkCmd cmd);
  TStringList* __fastcall getUserNames(void);
  HWND __fastcall getCurrentRoomHandle(void);
  bool __fastcall roomExists(void);
};
//---------------------------------------------------------------------------
#endif
