//---------------------------------------------------------------------------
#ifndef DlgReplaceTextH
#define DlgReplaceTextH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Menus.hpp>
#include "ThreadOnChange.h"
#include "..\TaeRichEdit\TaeRichEdit.h"
//---------------------------------------------------------------------------
#define ID_FIND    0
#define ID_REPLACE 1
#define ID_NONE   -1

// prevDirection flags
#define DIR_REVERSE 0
#define DIR_FORWARD 1
#define DIR_UNSET  -1

//---------------------------------------------------------------------------
class TReplaceTextForm : public TForm
{
__published:  // IDE-managed Components
  TButton *ButtonFindUp;
  TButton *ButtonReplace;
  TButton *ButtonReplaceAll;
  TLabel *LabelFind;
  TCheckBox *CheckBoxCaseSensitive;
  TCheckBox *CheckBoxWholeWord;
  TLabel *LabelReplaceWith;
  TButton *ButtonCancel;
  TButton *ButtonFindDown;
  TCheckBox *CheckBoxWrapAround;
  TLabel *LabelPressF3F4;
  TTaeRichEdit *EditFind;
  TTaeRichEdit *EditReplace;
  TPopupMenu *ReplaceDlgPopupMenu;
  TMenuItem *Clear1;
  TMenuItem *N10;
  TMenuItem *MenuEditCut;
  TMenuItem *MenuEditCopy;
  TMenuItem *MenuEditPastePlain;
  TMenuItem *N15;
  TMenuItem *MenuEditSelectAll;
  TMenuItem *MenuEditPasteColor;
  TMainMenu *ReplaceDlgMainMenu;
  TMenuItem *MainMenuInsert;
  TMenuItem *ColorCode1;
  TMenuItem *FontSizeCode1;
  TMenuItem *BoldCtrlB1;
  TMenuItem *InverseVidio;
  TMenuItem *UnderlineCtrlU1;
  TMenuItem *PlainTextCtrlO1;
  TMenuItem *Push1;
  TMenuItem *Pop1;
  TMenuItem *MainMenuHelp;
  TButton *ButtonToggleView;
  TLabel *LabelViewMode;
  TMenuItem *MenuEditOptimize;
  TMenuItem *MenuEditClear;
  TMenuItem *MenuEditToggleView;
  TMenuItem *N1;
  TMenuItem *MainMenuEdit;
  TMenuItem *MainMenuEditClear;
  TMenuItem *N2;
  TMenuItem *MainMenuEditCut;
  TMenuItem *MainMenuEditCopy;
  TMenuItem *MainMenuEditPasteColor;
  TMenuItem *MainMenuEditPastePlain;
  TMenuItem *N3;
  TMenuItem *MainMenuEditSelectAll;
  TMenuItem *MainMenuEditToggleView;
  TMenuItem *MainMenuEditOptimize;
  TMenuItem *N4;
  TMenuItem *N5;
  TMenuItem *Undo1;
  TMenuItem *MenuEditUndo;
  TMenuItem *FontTypeCode1;
  TMenuItem *PageBreak1;
  void __fastcall ButtonReplaceAllClick(TObject *Sender);
  void __fastcall ButtonFindUpClick(TObject *Sender);
  void __fastcall ButtonFindDownClick(TObject *Sender);
  void __fastcall ButtonCancelClick(TObject *Sender);
  void __fastcall CheckBoxCaseSensitiveClick(TObject *Sender);
  void __fastcall CheckBoxWholeWordClick(TObject *Sender);
  void __fastcall CheckBoxWrapAroundClick(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
  void __fastcall MenuEditClearClick(TObject *Sender);
  void __fastcall MenuEditCutClick(TObject *Sender);
  void __fastcall MenuEditCopyClick(TObject *Sender);
  void __fastcall MenuEditPastePlainClick(TObject *Sender);
  void __fastcall MenuEditSelectAllClick(TObject *Sender);
  void __fastcall MenuEditPasteColorClick(TObject *Sender);
  void __fastcall EditKeyPress(TObject *Sender, char &Key);
  void __fastcall EditKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
  void __fastcall ReplaceDlgPopupMenuPopup(TObject *Sender);
  void __fastcall ButtonClearReplaceClick(TObject *Sender);
  void __fastcall ButtonClearFindClick(TObject *Sender);
  void __fastcall ButtonReplaceClick(TObject *Sender);
  void __fastcall ColorCode1Click(TObject *Sender);
  void __fastcall FontSizeCode1Click(TObject *Sender);
  void __fastcall BoldCtrlB1Click(TObject *Sender);
  void __fastcall InverseVidioClick(TObject *Sender);
  void __fastcall UnderlineCtrlU1Click(TObject *Sender);
  void __fastcall PlainTextCtrlO1Click(TObject *Sender);
  void __fastcall Push1Click(TObject *Sender);
  void __fastcall Pop1Click(TObject *Sender);
  void __fastcall EditFindEnter(TObject *Sender);
  void __fastcall EditReplaceEnter(TObject *Sender);
  void __fastcall FormCreate(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall MainMenuHelpClick(TObject *Sender);
  void __fastcall ButtonToggleViewClick(TObject *Sender);
  void __fastcall MenuEditOptimizeClick(TObject *Sender);
  void __fastcall MainMenuEditClearClick(TObject *Sender);
  void __fastcall MainMenuEditSelectAllClick(TObject *Sender);
  void __fastcall MenuEditUndoClick(TObject *Sender);
  void __fastcall Undo1Click(TObject *Sender);
  void __fastcall EditFindExit(TObject *Sender);
  void __fastcall EditReplaceExit(TObject *Sender);
  void __fastcall MainMenuEditClick(TObject *Sender);
  void __fastcall FontTypeCode1Click(TObject *Sender);
  void __fastcall PageBreak1Click(TObject *Sender);
//  void __fastcall EditChange(TObject *Sender, DWORD dwChangeType,
//          void *pvCookieData, bool &AllowChange);
  void __fastcall EditChange(TObject *Sender);

//protected:
//  void __fastcall WMUpdateHandler(TMessage &Msg);

private:  // User declarations

  // Common menu handlers
  void __fastcall HClear(TTaeRichEdit* re);
  void __fastcall HUndo(TTaeRichEdit* re);
  void __fastcall HSelectAll(TTaeRichEdit* re);
  void __fastcall HMenuPopup(TTaeRichEdit* re);

  void __fastcall ViewIrc(TTaeRichEdit* re);
  void __fastcall ViewRtf(TTaeRichEdit* re);
  bool __fastcall SingleReplace(WideString ReplaceStr, int &deltaLen);
  void __fastcall SetMenuInsertItemsState(bool bEnable);
  void __fastcall SetFocusId(void);
  void __fastcall GetFocusedCtrl(TTaeRichEdit* &re);
  void __fastcall GetFocusedCtrl(TTaeRichEdit* &re, int &idx);
  void __fastcall EmbraceSelection(int i1, int i2 = -1);
  void __fastcall SetViewState(int NewView);
  void __fastcall ReplaceText(bool bReplaceAll);
  bool __fastcall ReplaceSelected(WideString S);
  int __fastcall ReplaceAll(void);
  int __fastcall ReplaceAll(TStringsW* sl);
  int __fastcall FindNext(bool bForward);
  bool __fastcall FindOne(WideString FindStr, TStringsW* sl);
  void __fastcall InitControls(void);
  void __fastcall InitCaptions(void);
  PASTESTRUCT __fastcall EditPaste(void);
  PASTESTRUCT __fastcall EditPaste(WideString S);
  PASTESTRUCT __fastcall EditCutCopy(bool bCut, bool bCopy);
  bool __fastcall ProcessOC(TTaeRichEdit* re, ONCHANGEW oc);
  void __fastcall InitCurrentFindPos(bool bForward);
  void __fastcall IrcInsert(TTaeRichEdit* re, WideString S1, WideString S2);
  void __fastcall SetView(TTaeRichEdit* re, int NewView);
  void __fastcall SetFormDesign(bool bMultiLine);
  bool __fastcall BothEditBoxesAreSingleLine(void);
  bool __fastcall InsertEffectChars(TTaeRichEdit* re, WideString S1, WideString S2);

  bool bWholeWord, bMatchCase, bWrapAround;
  bool bEnableReplace, bAbort, bReplacing;
  bool bFindFirst;
  bool bUnicodeKeySequence;

  unsigned short cbHTML, cbRTF; // Registered clipboard formats

  // We init EditFind with the main edit's selected text if the text
  // has no cr/lfs in it... so we clear this flag if more than one line
  // was selected... this allows us to have a large selected zone to search
  // in OR select just a few words and have that automatically appear in
  // EditFind...
  bool bUseSelected, bMultiLine;

  int focusId, prevDirection;

  int BeginningOfText, EndingOfText, CurrentFindPos;
  int PrevFindLength, PrevSelStart;

  TSearchTypes2 Flags;

  WideString EditString[2];

  TDTSColor* dts; // easy pointer to main form...

public:    // User declarations
  __fastcall TReplaceTextForm(TComponent* Owner);
  int __fastcall GetFindStrippedLen(void);
  int __fastcall GetReplaceStrippedLen(void);
  WideString __fastcall GetFindStrippedStr(void);
  WideString __fastcall GetReplaceStrippedStr(void);
  int __fastcall FrReplaceAll(WideString FindStr, WideString ReplaceStr);
  void __fastcall FrFind(bool bForward);
  void __fastcall FrReset(bool bSetFindFirst=false);
  void __fastcall FrClose(void);
  void __fastcall ChangeView();
  void __fastcall ChangeView(int View);

  __property bool FindFirst = {read = bFindFirst};
  __property bool WholeWord = {read = bWholeWord, write = bWholeWord};
  __property bool MatchCase = {read = bMatchCase, write = bMatchCase};
  __property bool WrapAround = {read = bWrapAround, write = bWrapAround};
  __property bool EnableReplace = {read = bEnableReplace, write = bEnableReplace};
  __property WideString FindCodesStr = {read = EditString[ID_FIND]};
  __property WideString ReplaceCodesStr = {read = EditString[ID_REPLACE]};
  __property WideString FindStrippedStr = {read = GetFindStrippedStr};
  __property WideString ReplaceStrippedStr = {read = GetReplaceStrippedStr};
  __property int FindStrippedLen = {read = GetFindStrippedLen};
  __property int ReplaceStrippedLen = {read = GetReplaceStrippedLen};

//BEGIN_MESSAGE_MAP
//    //add message handler for EN_UPDATE
//    // NOTE: All of the TWM* types are in messages.hpp!
//    VCL_MESSAGE_HANDLER(EN_UPDATE, TMessage, WMUpdateHandler)
//END_MESSAGE_MAP(TComponent)
};
//---------------------------------------------------------------------------
extern PACKAGE TReplaceTextForm *ReplaceTextForm;
//---------------------------------------------------------------------------
#endif
