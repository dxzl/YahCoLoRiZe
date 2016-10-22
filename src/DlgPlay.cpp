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

#include "DlgPlay.h"
#include "Playback.h"
#include "Paltalk.h"

// ComboBoxW.h is customized (by me) for Wide (UTF-16) calls (Scott S.)
#include "ComboBoxW.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "TaeRichEdit"
#pragma resource "*.dfm"

TPlayForm *PlayForm;
//---------------------------------------------------------------------------
__fastcall TPlayForm::TPlayForm(TComponent* Owner)
  : TForm(Owner)
{
  this->dts = DTSColor;

  try
  {
    DisableEventHandlers();

    if (dts->IsPlugin())
    {
      Version = dts->PluginVersion; // Set to global string (sent by plugin on init!)

      if (Version.IsEmpty())
        Version = String(PLAYMSG[21]); // + " (no Plugin!)";
      else
      {
        //int pos = Version.Pos(".0.0");
        //if (pos > 0)
        //  Version = Version.SubString(1, pos-1);

        if (dts->IsCli(C_HYDRAIRC))
          Version = String(PLAYMSG[21]) + " (HydraIRC: " + Version + ")";
        else if (dts->IsCli(C_ICECHAT))
          Version = String(PLAYMSG[21]) + " (IceChat: " + Version + ")";
        else if (dts->IsCli(C_LEAFCHAT))
          Version = String(PLAYMSG[21]) + " (LeafChat: " + Version + ")";
        else if (dts->IsCli(C_TRINITY))
          Version = String(PLAYMSG[21]) + " (Trinity: " + Version + ")";
        else Version = String(PLAYMSG[21]);
      }
    }
    else
    {
      TPlayback* pb = new TPlayback(dts);

      if (pb)
        Version = pb->GetDLLVersion();
      else
        Version = "";

      delete pb;

      if (Version.IsEmpty())
        Version = String(PLAYMSG[21]) + " (no DLL!)";
      else
        Version = String(PLAYMSG[21]) + " (DLL: " + Version + ")";
    }
  }
  catch(...)
  {
    Version = String(PLAYMSG[21]);
  }

  Caption = Version;

  // Some hints that won't change by-client...
  UseDLL->Hint = PLAYMSG[0];
  StripFont->Hint = PLAYMSG[1];
  SendUtf8CheckBox->Hint = PLAYMSG[4];

  PadSpaces->Hint = PLAYMSG[14];
  NSEdit->Hint = PLAYMSG[15];
  LabelRooms1->Hint = PLAYMSG[16];
  AddButton->Hint = PLAYMSG[10];
  DeleteButton->Hint = PLAYMSG[17];

  LabelBurstTime->Caption = PLAYMSG[22];
  LabelBurstInt->Caption = PLAYMSG[23];
  LabelRooms1->Caption = PLAYMSG[24];
  SendUtf8CheckBox->Caption = PLAYMSG[25];
  LabelFile->Caption = PLAYMSG[26];
  LabelMaxBytes->Caption = PLAYMSG[27];
  StripFont->Caption = PLAYMSG[28];
  UseDLL->Caption = PLAYMSG[29];
  UseFile->Caption = PLAYMSG[30];
  LineBurst->Caption = PLAYMSG[31];
  PadSpaces->Caption = PLAYMSG[32];
  DeleteButton->Caption = PLAYMSG[33];
  ResetButton->Caption = PLAYMSG[34];
  CancelButton->Caption = PLAYMSG[35];
  PaddingLabel->Caption = PLAYMSG[36];
  AddButton->Caption = PLAYMSG[37];

  // Initialize checkboxes and local flags from the main flags
  SendUtf8CheckBox->Checked = bSendUtf8 = dts->bSendUtf8;
  UseDLL->Checked = bUseDLL = dts->bUseDLL;
  UseFile->Checked = bUseFile = dts->bUseFile;
  SendUtf8CheckBox->Checked = bSendUtf8 = dts->bSendUtf8;
  StripFont->Checked = bStripFont = dts->bStripFont;
  PadSpaces->Checked = bPadSpaces = dts->bPadSpaces;

  LabelMin->Caption = String(PLAY_DLG_MIN_BYTES);
  MaxBytes->Min = PLAY_DLG_MIN_BYTES;
  LabelMax->Caption = String(PLAY_DLG_MAX_BYTES);
  MaxBytes->Max = PLAY_DLG_MAX_BYTES;

  m_hCombo = NULL;
  m_hComboEdit = NULL;
  m_hPathEdit = NULL;
  m_oldComboEditProc = NULL;
#if DEBUG_ON
  dts->CWrite("\r\nm_hEnd Constructor!\r\n");
#endif
}
//---------------------------------------------------------------------------
__fastcall TPlayForm::~TPlayForm()
{
  DisableEventHandlers();

  if (m_hPathEdit)
    DestroyWindow(m_hPathEdit);

  if (m_hCombo)
  {
    if (m_hComboEdit)
    {
      // remove hook
      if (m_oldComboEditProc)
      {
        (WNDPROC)SetWindowLongW(m_hComboEdit, GWL_WNDPROC, (LONG)m_oldComboEditProc);
        m_oldComboEditProc = NULL;
      }

      // Free resourses
      // NOTE: Do this during close so we are sure all the handles are still
      // valid.
      DestroyWindow(m_hCombo);
      RemoveProp(m_hComboEdit, "CMB");
    }
  }

  if (m_hFont)
    DeleteFont(m_hFont);

  if (OldWinProc)
  {
    WindowProc = OldWinProc;
    OldWinProc = NULL;
  }
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::FormCreate(TObject *Sender)
{
  HINSTANCE hInst = (HINSTANCE)GetModuleHandle(NULL);

  m_hPathEdit = CreateWindowExW(WS_EX_CLIENTEDGE,
      L"EDIT", L"",
      WS_TABSTOP|WS_OVERLAPPED|WS_CHILD|ES_LEFT|ES_AUTOHSCROLL,
      16, // x
      132, // y
      273, // width
      23, // height
      this->Handle, // parent
      (HMENU)ID_EditFile,
      hInst,
      NULL);

  if (!m_hPathEdit)
  {
#if DEBUG_ON
    dts->CWrite("\r\nm_hPathEdit is NULL!\r\n");
#endif
    return;
  }

  // NOTE: Within the combo-box is an edit control - later we will get it's
  // handle and subclass the edit-control's message-handler to get the Enter
  // key-press and to intercept Alt-X HexToUnicode conversion commands.
  m_hCombo = CreateWindowW(L"COMBOBOX", L"",
      WS_TABSTOP|WS_OVERLAPPED|WS_VISIBLE|WS_CHILD|CBS_DROPDOWN|
                                     CBS_HASSTRINGS|CBS_AUTOHSCROLL,
      16, // x
      90, // y
      273, // width
      200, // height (IMPORTANT: HAS TO INCLUDE THE LIST-BOX HEIGHT!)
      this->Handle, // parent
      (HMENU)ID_ComboRooms, // menu (This is the Control ID Also!) use GetDlgCtrlID()
      // Need the instance - the pop-up menu etc start working :-)
      // Also have seen this as (void*)GetWindowLongW(Application->Handle, GWL_HINSTANCE)
      hInst,
      NULL);

  if (!m_hCombo)
  {
#if DEBUG_ON
    dts->CWrite("\r\nm_hCombo is NULL!\r\n");
#endif
    return;
  }

#if DEBUG_ON
  dts->CWrite("\r\nm_hCombo Control ID: " + String(GetDlgCtrlID(m_hCombo)) + "\r\n");
#endif

  m_hFont = GetStockFont(DEFAULT_GUI_FONT);

  if (!m_hFont)
  {
#if DEBUG_ON
    dts->CWrite("\r\nm_hFont is NULL!\r\n");
#endif
    return;
  }

  // Set the default GUI font (default font is NOT pretty!)
  SetWindowFont(m_hCombo, m_hFont, FALSE);
  SetWindowFont(m_hPathEdit, m_hFont, FALSE);

  // Let user type up to 40 unicode chars
  ComboBoxW_LimitText(m_hCombo, MAXROOMCHARS);

  // Start processing custom windows messages
  OldWinProc = WindowProc;
  WindowProc = CustomMessageHandler;

  // Get handle of the edit window within our combobox control
  m_hComboEdit = ::GetWindow(m_hCombo, GW_CHILD);

  if (!m_hComboEdit)
  {
#if DEBUG_ON
    dts->CWrite("\r\nm_hComboEdit is NULL!\r\n");
#endif
    return;
  }

  // Replace the handler for the combo-box's edit-control
  if (!m_oldComboEditProc)
    m_oldComboEditProc = (WNDPROC)SetWindowLongW(m_hComboEdit,
                          GWL_WNDPROC, (LONG)ComboEditCallbackProc);

  SetProp(m_hComboEdit, "CMB", (void*)this);

  LoadRoomList();

#if DEBUG_ON
  dts->CWrite("\r\nm_hEnd Create!\r\n");
#endif
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::FormShow(TObject *Sender)
{
  ProcessUi();

  //-------------------------------------------------------------------
// Experimental!!!!!!!!!!!!!!!!!!!!!!!!
  // Took me a while to discover that you can set any control to unicode
  // for Vista and up "on the fly"
// won't work with our edit control????
//  SendMessageW(GetWindow(PlayPath->Handle, GW_HWNDFIRST),
//                        CCM_SETUNICODEFORMAT, TRUE, 0); // GW_CHILD
//  PlayPath->Visible = true;
  //-------------------------------------------------------------------

}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::FormClose(TObject *Sender, TCloseAction &Action)
{
#if DEBUG_ON
  dts->CWrite("\r\nGot WM_CLOSE!\r\n");
#endif
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::FormDestroy(TObject *Sender)
{
// try moving to close...
//  if (m_hCombo)
//  {
//    DestroyWindow(m_hCombo);
//    DeleteFont(m_hFont);
//  }
}
//---------------------------------------------------------------------------
// property getter for CurrentRoomW
WideString __fastcall TPlayForm::GetCurrentRoomW(void)
{
  int roomIdx = ComboBoxW_GetCurSel(m_hCombo);

  if (roomIdx != CB_ERR)
    return ComboBoxW_GetString(m_hCombo, roomIdx);

  return "";
}
//----------------------------------------------------------------------------
void __fastcall TPlayForm::CustomMessageHandler(TMessage &msg)
{
  // ReplyMessage(0) not needed since we handle lengthy
  // operations via timer-delay from another handler...
  try
  {
    // Intercept custom messages from our custom combo-box's custom edit
    // control's message handler (which detects tough-to-get key events!)
    if (msg.Msg == WM_COMBO_ENTER)
      AddRoom();
    else if (msg.Msg == WM_COMBO_ESC)
    {
#if DEBUG_ON
        dts->CWrite("\r\nESCAPE received!!!!!!!!!!!!!!!!\r\n");
#endif
    }
    else if (msg.Msg == WM_COMBO_TAB)
    {
#if DEBUG_ON
        dts->CWrite("\r\nTAB received!!!!!!!!!!!!!!!!\r\n");
#endif
    }
    else if (msg.Msg == WM_COMBO_ALT_X)
      HexToUnicode();
  }
  __finally
  {
    OldWinProc(msg); // Call main handler
  }
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::ProcessUi(void)
{
  try
  {
    DisableEventHandlers();

    // NOTE: All logic here was carefully designed via
    // a truth-table!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    if (dts->IsCli(C_YAHELITE))
      UseFile->Hint = PLAYMSG[2];
    else
      UseFile->Hint = PLAYMSG[3];

//    NSEdit->Font->Name = dts->cFont; // Use main edit-window font

    // Invisible-spaces init.
    NSEdit->Text = dts->NSChar; // Invisible spaces character

    if (PadSpaces->Visible && PadSpaces->Checked)
      NSEdit->Visible = true;
    else
      NSEdit->Visible = false;

    MainInt->Text = String(dts->PlayTime);
    TempTime = dts->PlayTime;
    MainInt->Visible = true; //time

    BurstInt->Text = String(dts->BurstInterval);
    TempBurstInterval = dts->BurstInterval;

    TempMaxBytes = dts->GMaxPlayXmitBytes;
    if (TempMaxBytes > MaxBytes->Max)
      TempMaxBytes = MaxBytes->Max;
    MaxBytes->Position =
      (TempMaxBytes < 0) ?
                MaxBytes->Max : TempMaxBytes;
    PrintCaption(TempMaxBytes);

//    PlayPath->Text = dts->Default_Playfile; // file
    EditW_SetText(m_hPathEdit, dts->Default_Playfile.c_bstr());

    dts->IsCli(C_YAHELITE) ? ComboBoxW_Hide(m_hCombo) : ComboBoxW_Show(m_hCombo);

    DeleteButton->Visible = dts->IsCli(C_YAHELITE) ? false : true;
    LabelRooms1->Visible = dts->IsCli(C_YAHELITE) ? false : true;

    if (dts->GLineBurstSize > 0)
      LineBurst->ItemIndex = dts->GLineBurstSize-1;
    else
      LineBurst->ItemIndex = 0;

    if (dts->IsCli(C_YAHELITE) || dts->IsCli(C_XIRCON) ||
                              dts->IsCli(C_MIRC) || dts->IsCli(C_VORTEC))
      SetUseFile(true);
    else
      SetUseFile(false);

    if (dts->IsCli(C_PALTALK))
    {
      // Default is to strip font-type when converting to RTF - set this true
      // to have the option appear to allow RTF font-indexes to go through...
      // (it really would be meaningless though - because the internal
      // YahColOrIzE font-table would not match a "hypothetical" Paltalk
      // master server-side RTF font-table anyway
      //
      // If you set it true also have a look at SendPaltalk() in Play.cpp
      SetStripFont(false);

      SetUseDLL(false);
      SetPadSpaces(false);
      SetTempFilePath(false);

      SetBurst(true);
    }
    else if (dts->IsCli(C_YAHELITE) || dts->IsPlugin())
    {
      SetUseDLL(false);
      SetPadSpaces(false);

      if (dts->IsCli(C_YAHELITE))
        SetStripFont(true);
      else
        SetStripFont(false);

      if (UseFile->Checked)
      {
        SetBurst(false);
        SetTempFilePath(true);
      }
      else
      {
        SetBurst(true);
        SetTempFilePath(false);
      }
    }
    else // mIRC, XircON or Vortec
    {
      StripFont->Visible = false;
      StripFont->Checked = false;

      if (dts->IsCli(C_XIRCON))
      {
        // XircON does not need space-padding if we append CTRL_K
        // Also modified the DLL so that it appends a CTRL_K 1/17/2014
        SetPadSpaces(false);

        UseDLL->Visible = false; // have to always use DLL
        UseDLL->Checked = true;

        if (UseFile->Checked)
        {
          SetBurst(false);
          SetTempFilePath(true);
        }
        else
        {
          SetBurst(true);
          SetTempFilePath(false);
        }
      }
      else // mIRC or Vortec
      {
        if (Version.IsEmpty())
          SetUseDLL(false); // DLL exists?
        else
          SetUseDLL(true);

        if (UseFile->Checked)
        {
          SetPadSpaces(false);
          SetBurst(false);
          SetTempFilePath(true);
        }
        else
        {
          SetPadSpaces(true);
          SetBurst(true);
          SetTempFilePath(false);
        }
      }
    }

    EnableEventHandlers();
  }
  catch(...)
  {
    ShowMessage("Exception thrown in PlayDlg.cpp, \"ProcessUi()\"");
  }
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::EnableEventHandlers(void)
{
  // Enable event handlers
  PadSpaces->OnClick = PadSpacesClick;
  UseFile->OnClick = UseFileClick;
  SendUtf8CheckBox->OnClick = SendUtf8Click;
  UseDLL->OnClick = UseDLLClick;
  StripFont->OnClick = StripFontClick;
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::DisableEventHandlers(void)
{
  // Disable event handlers
  PadSpaces->OnClick = NULL;
  UseFile->OnClick = NULL;
  SendUtf8CheckBox->OnClick = NULL;
  UseDLL->OnClick = NULL;
  StripFont->OnClick = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::LoadRoomList(void)
{
  if (dts->IsCli(C_PALTALK))
  {
    // For Paltalk we don't use the internal GRooms and GRoomIndex

    // Get list from Paltalk
    if (GPaltalk)
    {
      try
      {
        TStringList* ptRooms = GPaltalk->getAllRoomsAndPms();

        // Move Utf-8 paltalk room-list to the wide-strings in the combo-box
        if (ptRooms && ptRooms->Count)
        {
          ComboBoxW_ResetContent(m_hCombo); // Clear Rooms

          for (int ii = 0; ii < ptRooms->Count; ii++)
            ComboBoxU_AddString(m_hCombo, ptRooms->Strings[ii]);
        }
      }
      catch(...){}
    }

    if (!ComboBoxW_GetCount(m_hCombo))
    {
      ComboBoxU_AddString(m_hCombo, "Can't Locate Any Paltalk Rooms!");
      ComboBoxW_SetCurSel(m_hCombo, 0);
      bHaveRooms = false;
    }
    else
    {
      int idx = GPaltalk->findRoom(dts->PlayChan);

      if (idx < 0)
      {
        // Our current PlayChan not in the list? pick first room in list...
        GPaltalk->CurrentRoomIndex = 0;
        ComboBoxW_SetCurSel(m_hCombo, 0);
        dts->PlayChan = GPaltalk->getCurrentRoomOrPmName();
      }
      else // PlayChan is in the list
      {
        // Set index of current PlayChan...
        GPaltalk->CurrentRoomIndex = idx;
        ComboBoxW_SetCurSel(m_hCombo, idx);
      }

      bHaveRooms = true;
    }
  }
  else
  {
    try
    {
      if (dts->GRooms->Count <= 0 ||
              dts->GRoomIndex >= dts->GRooms->Count ||
                                                dts->GRoomIndex < 0)
      {
        // Repair!
        dts->GRooms->Clear();
        dts->GRooms->Add(STATUSCHAN);
        dts->GRoomIndex = 0;
        dts->PlayChan = STATUSCHAN; // "status"

#if DEBUG_ON
        dts->CWrite("\r\nTPlayForm::ProcessUi(): ROOMS REPAIR!\r\n");
#endif
      }
      else
      {
        String sCurrentRoomUtf8 = dts->GRooms->Strings[dts->GRoomIndex];

        if (dts->PlayChan != sCurrentRoomUtf8)
          // "Play channel was remotely changed to " // 9
          // "setting it back to " // 10
          utils->ShowMessageU(String(PLAYMSG[9]) + "\"" + dts->PlayChan +
             "\",\n" + String(PLAYMSG[10]) + "\"" + sCurrentRoomUtf8 + "\".");

        dts->PlayChan = sCurrentRoomUtf8;
      }
    }
    catch(...)
    {
      // Index out of bounds? (WHOOPS!)
      ShowMessage("Index is bad in ProcessUi()");
    }

    ComboBoxU_SetText(m_hCombo, dts->PlayChan); // channel

    int count = dts->GRooms->Count;

#if DEBUG_ON
    dts->CWrite("\r\nroom count: " + String(count) + "\r\n");
#endif

    // Populate drop-down rooms list
    // This works but will add empty strings from an old rev of YahCoLoRiZe!
    // Rooms->Items = dts->GRooms; So use this!
    for (int ii = 0; ii < count; ii++)
    {
      String sRoomUtf8 = dts->GRooms->Strings[ii];

      if (!sRoomUtf8.IsEmpty())
      {
        ComboBoxU_AddString(m_hCombo, sRoomUtf8);
#if DEBUG_ON
        dts->CWrite("\r\nTPlayForm::LoadRoomList(): Added room: " +
                                                      sRoomUtf8 + "\r\n");
#endif
      }
    }

    // Avoid out-of-bounds error if list is out of whack...
    if (dts->GRoomIndex > ComboBoxW_GetCount(m_hCombo))
    {
      dts->GRoomIndex = 0;
      utils->ShowMessageU("GRoomIndex out of bounds, repairing...");
    }

    ComboBoxW_SetCurSel(m_hCombo, dts->GRoomIndex);
    bHaveRooms = true;
  }
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::FormClick(TObject *Sender)
{
  ButtonOkClick(NULL); // Save and exit
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::ButtonOkClick(TObject *Sender)
{
  DisableEventHandlers();
  SaveSettings();
  Close();
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::SaveSettings(void)
{
  try
  {
    dts->PlayTime = TempTime;
    dts->BurstInterval = TempBurstInterval;

    if (dts->PlayTime < 50)
      dts->PlayTime = 50;

    if (dts->BurstInterval < 30)
      dts->BurstInterval = 30;

    if (dts->PlayTime < dts->BurstInterval)
      dts->PlayTime = dts->BurstInterval;

//    dts->Default_Playfile = PlayPath->Text;
    dts->Default_Playfile = EditBoxW_GetString(m_hPathEdit);

    EditW_SetText(m_hPathEdit, dts->Default_Playfile.c_bstr());

    dts->bUseDLL = bUseDLL;
    dts->bUseFile = bUseFile;
    dts->bSendUtf8 = bSendUtf8;
    dts->bStripFont = bStripFont;
    dts->bPadSpaces = bPadSpaces;

    dts->GMaxPlayXmitBytes = TempMaxBytes;

    dts->GLineBurstSize = LineBurst->ItemIndex+1;

    // "Invisible spaces" algorithm char
    if (NSEdit->Text.Length() >= 1)
      dts->NSChar =(char)NSEdit->Text[1];
    else
      dts->NSChar = '_';
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nError in SaveSettings - First Save Stage!\r\n");
#endif
  }

  if (!m_hCombo)
  {
#if DEBUG_ON
    dts->CWrite("\r\nm_hCombo is NULL in SaveSettings!\r\n");
#endif
    return;
  }

  try
  {
    WideString sRoom = ComboBoxW_GetString(m_hCombo);

    if (dts->IsCli(C_PALTALK))
    {
      if (bHaveRooms && !sRoom.IsEmpty())
      {
        // Assign the room's handles
        if (GPaltalk != NULL && !GPaltalk->getPaltalkService(dts->PlayChan))
          dts->PlayChan = "";
        else
          dts->PlayChan = utils->WideToUtf8(sRoom);
      }
      else
        dts->PlayChan = "";
    }
    else
    {
      // Add new room if it's not a duplicate...
      AddRoom(sRoom);

      int roomCount = ComboBoxW_GetCount(m_hCombo);

      if (roomCount)
      {
        int roomIdx = ComboBoxW_GetCurSel(m_hCombo);

        // Copy local rooms to global rooms list
#if DEBUG_ON
        dts->CWrite("\r\nTPlayForm::SaveSettings(): Copying new rooms to GRooms!\r\n");
#endif

        dts->GRooms->Clear();

        for (int ii = 0; ii < roomCount; ii++)
          dts->GRooms->Add(ComboBoxU_GetString(m_hCombo, ii));

        if (roomIdx == CB_ERR)
          dts->GRoomIndex = roomCount-1;
        else
          dts->GRoomIndex = roomIdx;

        dts->PlayChan = dts->GRooms->Strings[dts->GRoomIndex]; // both utf-8
      }
      else
        dts->PlayChan = STATUSCHAN;
    }

#if DEBUG_ON
    dts->CWrite("\r\nTPlayForm::SaveSettings(): PlayChan is: " + String(dts->PlayChan) + "\r\n");
#endif
  }
  catch(...)
  {
#if DEBUG_ON
    dts->CWrite("\r\nError in SaveSettings - Second Save Stage!\r\n");
#endif
  }
}
//---------------------------------------------------------------------------
bool __fastcall TPlayForm::AddRoom(void)
{
  if (dts->IsCli(C_PALTALK))
    return false; // rooms are fixed for Paltalk

  WideString sRoom = ComboBoxW_GetString(m_hCombo); // get edit text

  bool bRet = AddRoom(sRoom);

  if (bRet)
  {
    utils->ShowMessageU(String(PLAYMSG[12]) +
                utils->WideToUtf8(sRoom) + String(PLAYMSG[13]));

    ::SetFocus(m_hCombo);
  }

  return bRet;
}

bool __fastcall TPlayForm::AddRoom(WideString sRoom)
{
  if (dts->IsCli(C_PALTALK) || sRoom.IsEmpty())
    return false;

  // Already in list?
  int idx = ComboBoxW_FindString(m_hCombo, 0, sRoom.c_bstr());

  if (idx != CB_ERR)
  {
    ComboBoxW_DeleteString(m_hCombo, idx); // Delete it
    ComboBoxW_AddString(m_hCombo, sRoom.c_bstr()); // Add it back(may be different case!)
    ComboBoxW_SetCurSel(m_hCombo, ComboBoxW_GetCount(m_hCombo)-1); // set to new room
    return false;
  }

  if (ComboBoxW_GetCount(m_hCombo) >= MAXROOMHISTORY)
    ComboBoxW_DeleteString(m_hCombo, 1); // Delete oldest(not status)

  ComboBoxW_AddString(m_hCombo, sRoom.c_bstr());
  ComboBoxW_SetCurSel(m_hCombo, ComboBoxW_GetCount(m_hCombo)-1); // set to new room

  return true;
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::SetUseDLL(bool bOn)
{
  if (!bOn)
    UseDLL->Checked = false;
  else
    UseDLL->Checked = bUseDLL;

  UseDLL->Visible = bOn;
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::SetUseFile(bool bOn)
{
  if (!bOn)
    UseFile->Checked = false;
  else
    UseFile->Checked = bUseFile;

  UseFile->Visible = bOn;
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::SetSendUtf8(bool bOn)
{
  if (!bOn)
    SendUtf8CheckBox->Checked = false;
  else
    SendUtf8CheckBox->Checked = bSendUtf8;

  SendUtf8CheckBox->Visible = bOn;
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::SetStripFont(bool bOn)
{
  if (!bOn)
    StripFont->Checked = false;
  else
    StripFont->Checked = bStripFont;

  StripFont->Visible = bOn;
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::SetPadSpaces(bool bOn)
{
  if (!bOn)
    PadSpaces->Checked = false;
  else
    PadSpaces->Checked = bPadSpaces;

  PadSpaces->Visible = bOn;
  PaddingLabel->Visible = PadSpaces->Checked;
  NSEdit->Visible = PadSpaces->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::SetTempFilePath(bool bOn)
{
  ShowWindow(m_hPathEdit, bOn ? SW_SHOW : SW_HIDE);
  LabelFile->Visible = bOn;
  ResetButton->Visible = bOn;
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::SetBurst(bool bOn)
{
  // Burst interval edit
  BurstInt->Visible = bOn;
  LabelBurstInt->Visible = bOn;

  LineBurst->Visible = bOn;

  // Max Bytes control
  MaxBytes->Visible = bOn;
  LabelMaxBytes->Visible = bOn;
  LabelMin->Visible = bOn;
  LabelMax->Visible = bOn;
  LabelBytes->Visible = bOn;
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::MainIntChange(TObject *Sender)
{
  TempTime = MainInt->Text.ToIntDef(PLAYTIME);
  MainInt->Text = String(TempTime);
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::BurstIntChange(TObject *Sender)
{
  TempBurstInterval = BurstInt->Text.ToIntDef(BURSTINTERVAL);
  BurstInt->Text = String(TempBurstInterval);
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::ResetButtonClick(TObject *Sender)
{
  dts->Default_Playfile = utils->GetTempFileW(utils->Utf8ToWide(FN[6]));
//  PlayPath->Text = dts->Default_Playfile;
  EditW_SetText(m_hPathEdit, dts->Default_Playfile.c_bstr());
//  PlayPath->SetFocus();
  ::SetFocus(m_hPathEdit);
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::MaxBytesChange(TObject *Sender)
{
  int pos = MaxBytes->Position;
  if (pos == MaxBytes->Max)
    TempMaxBytes = -1; // no limit
  else
    TempMaxBytes = pos;
  PrintCaption(TempMaxBytes);
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::PrintCaption(int Value)
{
  String sTemp;
  if (Value < 0)
    sTemp = "*unlimited*";
  else
    sTemp = "*" + String(Value) + "*";
  LabelBytes->Caption = sTemp;
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::UseDLLClick(TObject *Sender)
{
  bUseDLL = UseDLL->Checked;
  ProcessUi();
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::UseFileClick(TObject *Sender)
{
  bUseFile = UseFile->Checked;
  ProcessUi();
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::SendUtf8Click(TObject *Sender)
{
  bSendUtf8 = SendUtf8CheckBox->Checked;
  ProcessUi();
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::StripFontClick(TObject *Sender)
{
  bStripFont = StripFont->Checked;
  ProcessUi();
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::PadSpacesClick(TObject *Sender)
{
  bPadSpaces = PadSpaces->Checked;
  ProcessUi();
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
  if (Key == VK_RETURN)
  {
    Key = 0;
    ButtonOkClick(NULL); // Save and exit
  }
  else if (Key == VK_ESCAPE)
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::CancelButtonClick(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::KillBeepKeyPress(TObject *Sender, char &Key)
{
  // Stop default "beep" sound
  if (Key == VK_RETURN)
  {
    Key = 0;
    ButtonOkClick(NULL); // Save and exit
  }
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::AddButtonClick(TObject *Sender)
{
  AddRoom();
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::DeleteButtonClick(TObject *Sender)
{
  if (dts->IsCli(C_PALTALK)) return; // rooms are fixed for Paltalk

  if (GetKeyState(VK_SHIFT) & 0x8000)
  {
    // Delete all rooms
    ComboBoxW_ResetContent(m_hCombo);
    ComboBoxU_AddString(m_hCombo, STATUSCHAN); // Add status
    ComboBoxW_SetCurSel(m_hCombo, 0);
    ComboBoxW_SetText(m_hCombo, CurrentRoomW.c_bstr());
    utils->ShowMessageU(PLAYMSG[11]); // Rooms deleted
  }
  else
  {
    // Delete one room
    DelRoom(ComboBoxW_GetString(m_hCombo)); // Delete room...
//    if (DelRoom(ComboBoxW_GetString(m_hCombo))) // Delete room...
//      utils->ShowMessageU(String(PLAYMSG[18]) + sRoomUtf8 + String(PLAYMSG[19]));
  }

  ::SetFocus(m_hCombo);
}
//---------------------------------------------------------------------------
bool __fastcall TPlayForm::DelRoom(WideString sRoom)
{
  if (dts->IsCli(C_PALTALK))
    return false; // rooms are fixed for Paltalk

  // Room is "status"?
  if (sRoom == utils->Utf8ToWide(STATUSCHAN))
  {
    utils->ShowMessageU(PLAYMSG[20]); // Can't delete status!
    return false;
  }

  int idx = ComboBoxW_FindString(m_hCombo, 0, sRoom.c_bstr());

  // Not in list or at position 0(status)?
  if (idx == CB_ERR)
    return false;

  ComboBoxW_DeleteString(m_hCombo, idx);

  int count = ComboBoxW_GetCount(m_hCombo);

  if (count)
  {
    int newIdx;
    if (idx == count)
      newIdx = count-1; // was at end? stay at end
    else
      newIdx = idx; // stay where we were

    ComboBoxW_SetCurSel(m_hCombo, newIdx);
  }

// think this is automatic now...???
//  EditRoom->TextW = CurrentRoomW;

  return true;
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::LineBurstClick(TObject *Sender)
// Shade out Burst-Int if only 1 line per burst
{
  if (LineBurst->ItemIndex == 0)
    BurstInt->Enabled = false;
  else
    BurstInt->Enabled = true;
}
//---------------------------------------------------------------------------
// CB_GETITEMDATA or CB_SETITEMDATA get/modify data
// CB_INSERTSTRING, WM_COMPAREITEM, CB_DIR
// GetComboBoxInfo
// https://msdn.microsoft.com/en-us/library/windows/desktop/ff485899(v=vs.85).aspx
/*
// Edit control macros
Edit_CanUndo
Edit_EmptyUndoBuffer
Edit_Enable
Edit_FmtLines
Edit_GetCueBannerText
Edit_GetFirstVisibleLine
Edit_GetHandle
Edit_GetHilite
Edit_GetLine
Edit_GetLineCount
Edit_GetModify
Edit_GetPasswordChar
Edit_GetRect
Edit_GetSel
Edit_GetText
Edit_GetTextLength
Edit_GetWordBreakProc
Edit_HideBalloonTip
Edit_LimitText
Edit_LineFromChar
Edit_LineIndex
Edit_LineLength
Edit_NoSetFocus
Edit_ReplaceSel
Edit_Scroll
Edit_ScrollCaret
Edit_SetCueBannerText
Edit_SetCueBannerTextFocused
Edit_SetHandle
Edit_SetHilite
Edit_SetModify
Edit_SetPasswordChar
Edit_SetReadOnly
Edit_SetRect
Edit_SetRectNoPaint
Edit_SetSel
Edit_SetTabStops
Edit_SetText
Edit_SetWordBreakProc
Edit_ShowBalloonTip
Edit_TakeFocus
Edit_Undo
*/
/*
ComboBoxW_Enable(h, fEnable)

ComboBoxW_GetTextLength(h)
ComboBoxW_GetText(h, lpch, cchMax)

ComboBoxW_SetText(h, lpsz)

ComboBoxW_LimitText(h, cchLimit)
ComboBoxW_GetEditSel(h)
ComboBoxW_SetEditSel(h, ichStart, ichEnd)

ComboBoxW_GetCount(h)
ComboBoxW_ResetContent(h)

ComboBoxW_AddString(h, lpsz)
ComboBoxW_InsertString(h, index, lpsz)

ComboBoxW_AddItemData(h, data)
ComboBoxW_InsertItemData(h, index, data)

ComboBoxW_DeleteString(h, index)

ComboBoxW_GetLBTextLen(h, index)
ComboBoxW_GetLBText(h, index, lpszBuffer)

ComboBoxW_GetItemData(h, index)
ComboBoxW_SetItemData(h, index, data)

ComboBoxW_FindString(h, indexStart, lpszFind)
ComboBoxW_FindStringExact(h, indexStart, lpszFind)
ComboBoxW_FindItemData(h, indexStart, data)

ComboBoxW_GetCurSel(h)
ComboBoxW_SetCurSel(h, index)

ComboBoxW_SelectString(h, indexStart, lpszSelect)
ComboBoxW_SelectItemData(h, indexStart, data)

ComboBoxW_Dir(h, attrs, lpszFileSpec)
ComboBoxW_ShowDropdown(h, fShow)
ComboBoxW_GetDroppedState(h)
ComboBoxW_GetDroppedControlRect(h, lprc)
ComboBoxW_GetItemHeight(h)
ComboBoxW_SetItemHeight(h, index, cyItem)
ComboBoxW_GetExtendedUI(h)
ComboBoxW_SetExtendedUI(h, flags)

CB_GETEDITSEL               0x0140
CB_LIMITTEXT                0x0141
CB_SETEDITSEL               0x0142
CB_ADDSTRING                0x0143
CB_DELETESTRING             0x0144
CB_DIR                      0x0145
CB_GETCOUNT                 0x0146
CB_GETCURSEL                0x0147
CB_GETLBTEXT                0x0148
CB_GETLBTEXTLEN             0x0149
CB_INSERTSTRING             0x014A
CB_RESETCONTENT             0x014B
CB_FINDSTRING               0x014C
CB_SELECTSTRING             0x014D
CB_SETCURSEL                0x014E
CB_SHOWDROPDOWN             0x014F
CB_GETITEMDATA              0x0150
CB_SETITEMDATA              0x0151
CB_GETDROPPEDCONTROLRECT    0x0152
CB_SETITEMHEIGHT            0x0153
CB_GETITEMHEIGHT            0x0154
CB_SETEXTENDEDUI            0x0155
CB_GETEXTENDEDUI            0x0156
CB_GETDROPPEDSTATE          0x0157
CB_FINDSTRINGEXACT          0x0158
CB_SETLOCALE                0x0159
CB_GETLOCALE                0x015A
CB_GETTOPINDEX              0x015b
CB_SETTOPINDEX              0x015c
CB_GETHORIZONTALEXTENT      0x015d
CB_SETHORIZONTALEXTENT      0x015e
CB_GETDROPPEDWIDTH          0x015f
CB_SETDROPPEDWIDTH          0x0160
CB_INITSTORAGE              0x0161


CB_ADDSTRING
CB_DELETESTRING
CB_DIR
CB_FINDSTRING
CB_FINDSTRINGEXACT
CB_GETCOMBOBOXINFO
CB_GETCOUNT
CB_GETCUEBANNER
CB_GETCURSEL
CB_GETDROPPEDCONTROLRECT
CB_GETDROPPEDSTATE
CB_GETDROPPEDWIDTH
CB_GETEDITSEL
CB_GETEXTENDEDUI
CB_GETHORIZONTALEXTENT
CB_GETITEMDATA // gets a tag you can associate with the item
CB_GETITEMHEIGHT
CB_GETLBTEXT
CB_GETLBTEXTLEN
CB_GETLOCALE
CB_GETMINVISIBLE
CB_GETTOPINDEX
CB_INITSTORAGE
CB_INSERTSTRING
CB_LIMITTEXT
CB_RESETCONTENT
CB_SELECTSTRING
CB_SETCUEBANNER
CB_SETCURSEL
CB_SETDROPPEDWIDTH
CB_SETEDITSEL
CB_SETEXTENDEDUI
CB_SETHORIZONTALEXTENT
CB_SETITEMDATA
CB_SETITEMHEIGHT
CB_SETLOCALE
CB_SETMINVISIBLE
CB_SETTOPINDEX
CB_SHOWDROPDOWN
*/

LRESULT CALLBACK TPlayForm::ComboEditCallbackProc(HWND hwnd, UINT uMsg,
                                                WPARAM wParam, LPARAM lParam)
{
//  static TPlayForm* spf = NULL;

//  if (!spf)
  TPlayForm* spf = (TPlayForm*)GetProp((void*)hwnd, "CMB");

  if (!spf)
    return 0;

  switch (uMsg)
  {
    case WM_PASTE:

#if DEBUG_ON
      spf->dts->CWrite("\r\nPaste\r\n");
#endif

    break;

    case WM_SYSKEYDOWN:

      if ((lParam & 0x20000000) && wParam == 'X') // ALT-X pressed?
      {
        SendMessage(spf->Handle, WM_COMBO_ALT_X, 0, 0);
        return 0;
      }

    break;

    case WM_KEYDOWN:

      switch (wParam)
      {
        case VK_TAB:
          SendMessage(spf->Handle, WM_COMBO_TAB, 0, 0);
        return 0;
        case VK_ESCAPE:
          SendMessageW(spf->Handle, WM_COMBO_ESC, 0, 0);
        return 0;
        case VK_RETURN:
          SendMessage(spf->Handle, WM_COMBO_ENTER, 0, 0);
        return 0;
      }
    break;

    case WM_SYSKEYUP:

      if ((lParam & 0x20000000) && wParam == 'X') // ALT-X pressed?
        return 0;

    break;

    case WM_KEYUP:
    case WM_CHAR:
      switch (wParam)
      {
        case VK_TAB:
        case VK_ESCAPE:
        case VK_RETURN:
          return 0;
      }
  }

  // Call default handler for the combo's edit-control
  try
  {
    if (spf && spf->OldComboEditProc)
    {
      LRESULT lr = CallWindowProcW(FARPROC(spf->OldComboEditProc), hwnd,
                                              uMsg, wParam, lParam);

      return lr;
    }
    else
      return 0;
  }
  catch(...) {}

  return 0;
}
//---------------------------------------------------------------------------
String __fastcall TPlayForm::ComboBoxU_GetString(HWND hwndCtl)
{
  return utils->WideToUtf8(ComboBoxW_GetString(hwndCtl));
}
//---------------------------------------------------------------------------
WideString __fastcall TPlayForm::ComboBoxW_GetString(HWND hwndCtl)
{
  WideString wText;

  if (hwndCtl)
  {
    int len = ComboBoxW_GetTextLength(hwndCtl);
    wchar_t* buf = new wchar_t[len+1];
    ComboBoxW_GetText(hwndCtl, buf, len+1);
    wText = WideString(buf, len);
    delete [] buf;
  }

  return wText;
}
//---------------------------------------------------------------------------
WideString __fastcall TPlayForm::EditBoxW_GetString(HWND hwndCtl)
{
  WideString wText;

  if (hwndCtl)
  {
    int len = EditW_GetTextLength(hwndCtl);
    wchar_t* buf = new wchar_t[len+1];
    EditW_GetText(hwndCtl, buf, len+1);
    wText = WideString(buf, len);
    delete [] buf;
  }

  return wText;
}
//---------------------------------------------------------------------------
String __fastcall TPlayForm::ComboBoxU_GetString(HWND hwndCtl, int index)
{
  return utils->WideToUtf8(ComboBoxW_GetString(hwndCtl, index));
}
//---------------------------------------------------------------------------
WideString __fastcall TPlayForm::ComboBoxW_GetString(HWND hwndCtl, int index)
{
  int len = ComboBoxW_GetLBTextLen(hwndCtl, index);
  wchar_t* lpszBuffer = new wchar_t[len+1];
  ComboBoxW_GetLBText(hwndCtl, index, lpszBuffer);
  WideString s = lpszBuffer;
  delete [] lpszBuffer;
  return s;
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::ComboBoxU_AddString(HWND hwndCtl, String sUtf8)
{
  ComboBoxW_AddString(hwndCtl, utils->Utf8ToWide(sUtf8).c_bstr());
}
//---------------------------------------------------------------------------
void __fastcall TPlayForm::ComboBoxU_SetText(HWND hwndCtl, String sUtf8)
{
  ComboBoxW_SetText(hwndCtl, utils->Utf8ToWide(sUtf8).c_bstr());
}
//----------------------------------------------------------------------------
void __fastcall TPlayForm::HexToUnicode(void)
{
  // user has typed Ctrl-X and expects us to convert a string of hex-digits
  // into a Unicode character... which, God willing, we shall attempt to do.
  WideString wTemp =
    ComboBoxW_GetString(m_hCombo).SubString(1, ComboBoxW_SelStart(m_hCombo));

#if DEBUG_ON
    dts->CWrite("\r\nwTemp: " + String(wTemp) + "\r\n");
#endif

  int len = wTemp.Length();

  if (!len)
    return;

  String sHex;
  wchar_t c;
  int hexCount = 0;

  for (int ii = len; ii > 0; ii--)
  {
    c = towlower(wTemp[ii]);

    if (iswdigit(c) || (c >= 'a' && c <= 'f'))
    {
      sHex = sHex.Insert((char)c, 1);

      if (++hexCount >= 5)
        break;
    }
    else
      break;
  }

  // We will need the 0-based start index of the digits in the edit-box to
  // select and replace with the Unicode char...
  int startIdx = len - sHex.Length();

  sHex = sHex.Insert("0x0", 1); // ToInDef needs to know it's a hex-string!

  wchar_t sUni[2] = { 0 };

  sUni[0] = (wchar_t)sHex.ToIntDef(-1);

  if (c >= ' ')
  {
    EditW_SetSel(m_hComboEdit, startIdx, len);
    EditW_ReplaceSel(m_hComboEdit, sUni);
  }
}
//---------------------------------------------------------------------------

