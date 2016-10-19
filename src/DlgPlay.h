//---------------------------------------------------------------------------
#ifndef DlgPlayH
#define DlgPlayH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <Mask.hpp>
#include "TaeRichEdit.h"
//---------------------------------------------------------------------------

// We export key-press info from the edit-control in the Unicode combobox
// to our main message-loop via SendMessage...
#define WM_COMBO_TAB (WM_USER)
#define WM_COMBO_ESC (WM_USER + 1)
#define WM_COMBO_ENTER (WM_USER + 2)
#define WM_COMBO_ALT_X (WM_USER + 3)

#define PLAY_DLG_MIN_BYTES 200
#define PLAY_DLG_MAX_BYTES 4000
//---------------------------------------------------------------------------
class TPlayForm : public TForm
{
  friend class TDTSColor;

__published:  // IDE-managed Components
  TEdit *MainInt;
  TLabel *LabelBurstTime;
  TLabel *LabelRooms1;
  TLabel *Label4;
  TCheckBox *UseDLL;
  TLabel *LabelFile;
  TButton *ResetButton;
  TRadioGroup *LineBurst;
  TTrackBar *MaxBytes;
  TLabel *LabelMaxBytes;
  TLabel *LabelMin;
  TLabel *LabelMax;
  TLabel *LabelBytes;
  TCheckBox *UseFile;
  TLabel *LabelBurstInt;
  TEdit *BurstInt;
  TCheckBox *StripFont;
  TButton *DeleteButton;
  TCheckBox *PadSpaces;
  TEdit *NSEdit;
  TButton *ButtonOk;
  TButton *CancelButton;
  TLabel *PaddingLabel;
  TCheckBox *SendUtf8CheckBox;
  TButton *AddButton;
  void __fastcall MainIntChange(TObject *Sender);
  void __fastcall BurstIntChange(TObject *Sender);
  void __fastcall ResetButtonClick(TObject *Sender);
  void __fastcall MaxBytesChange(TObject *Sender);
  void __fastcall UseDLLClick(TObject *Sender);
  void __fastcall UseFileClick(TObject *Sender);
  void __fastcall SendUtf8Click(TObject *Sender);
  void __fastcall PadSpacesClick(TObject *Sender);
  void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
  void __fastcall FormClick(TObject *Sender);
  void __fastcall ButtonOkClick(TObject *Sender);
  void __fastcall CancelButtonClick(TObject *Sender);
  void __fastcall KillBeepKeyPress(TObject *Sender, char &Key);
  void __fastcall DeleteButtonClick(TObject *Sender);
  void __fastcall StripFontClick(TObject *Sender);
  void __fastcall LineBurstClick(TObject *Sender);
  void __fastcall FormCreate(TObject *Sender);
  void __fastcall AddButtonClick(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall FormDestroy(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);

protected:
  static LRESULT CALLBACK ComboEditCallbackProc(HWND hwnd, UINT uMsg,
                                              WPARAM wParam, LPARAM lParam);

  void __fastcall CustomMessageHandler(TMessage &msg);

  // Property getters/setters
  WideString __fastcall GetCurrentRoomW(void);

private:  // User declarations
  WideString __fastcall EditBoxW_GetString(HWND hwndCtl);
  WideString __fastcall ComboBoxW_GetString(HWND hwndCtl);
  WideString __fastcall ComboBoxW_GetString(HWND hwndCtl, int index);
  String __fastcall ComboBoxU_GetString(HWND hwndCtl, int index);
  String __fastcall ComboBoxU_GetString(HWND hwndCtl);
  void __fastcall ComboBoxU_AddString(HWND hwndCtl, String sUtf8);
  void __fastcall ComboBoxU_SetText(HWND hwndCtl, String sUtf8);

  void __fastcall HexToUnicode(void);
  void __fastcall LoadRoomList(void);
  void __fastcall PrintCaption(int Value);
  void __fastcall SetStripFont(bool bOn);
  void __fastcall SetUseFile(bool bOn);
  void __fastcall SetSendUtf8(bool bOn);
  void __fastcall SetUseDLL(bool bOn);
  void __fastcall SetPadSpaces(bool bOn);
  void __fastcall SetTempFilePath(bool bOn);
  void __fastcall SetBurst(bool bOn);
  void __fastcall ProcessUi(void);
  void __fastcall SaveSettings(void);
  bool __fastcall AddRoom(WideString sRoom);
  bool __fastcall AddRoom(void);
  bool __fastcall DelRoom(WideString sRoom);
  void __fastcall DisableEventHandlers(void);
  void __fastcall EnableEventHandlers(void);

  String Version, SaveRoom;
  int TempBurstInterval, TempTime, TempMaxBytes;
  bool bChannelChanged, bListFull, bHaveRooms;

  bool bSendUtf8, bPadSpaces, bStripFont, bUseFile, bUseDLL;

  TDTSColor* dts; // easy pointer to main form...

  HWND m_hCombo; // combo-box handle
  HWND m_hComboEdit; // edit control in the combo-box
  HWND m_hPathEdit; // file-path edit handle
  WNDPROC m_oldComboEditProc;
  HFONT m_hFont;

  // Added to intercept Unicode Rooms COMBOBOX Enter KeyPress
  Controls::TWndMethod OldWinProc;

public:    // User declarations
  __fastcall TPlayForm(TComponent* Owner);
  __fastcall ~TPlayForm();

  __property bool HaveRooms = { read = bHaveRooms };
  __property bool SendUtf8 = { read = bSendUtf8 };
  __property WideString CurrentRoomW = { read = GetCurrentRoomW };
  __property WNDPROC OldComboEditProc = { read = m_oldComboEditProc };
};
//---------------------------------------------------------------------------
extern PACKAGE TPlayForm *PlayForm;
//---------------------------------------------------------------------------
#endif
