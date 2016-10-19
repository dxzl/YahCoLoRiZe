//---------------------------------------------------------------------------
#ifndef DlgWingEditH
#define DlgWingEditH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Menus.hpp>
#include <CheckLst.hpp>
#include <ExtCtrls.hpp>
#include "MyCheckLst.hpp"
#include "..\TaeRichEdit\TaeRichEdit.h"
//---------------------------------------------------------------------------
#define SPI_GETFOCUSBORDERHEIGHT 0x2010 // SystemParameterwInfo
#define R2_XORPEN 7 // SetROP2

// Edit1 font defaults
#define FONT_DEF_CHARSET ANSI_CHARSET
#define FONT_DEF_COLOR clWindowText
#define FONT_DEF_PITCH fpFixed
#define FONT_DEF_SIZE 14
#define FONT_DEF_FONT "Courier New"

// Edit Types (stored in SourceEditID)
#define ET_NONE 0
#define ET_EDITTOPBOTTOM 1
#define ET_EDITLEFT 2
#define ET_EDITRIGHT 3
#define ET_EDITLISTLEFT 4
#define ET_EDITLISTRIGHT 5
#define ET_ADDWING 6

// ReplaceItem types
#define RI_NONE  0
#define RI_TRUE  1
#define RI_FALSE 2

#define WING_EXPORT_ID "YahCoLoRiZe wing-export identifier!"
#define WING_EXPORT_CONST 6 // # strings before wings

#define WM_CHANGEUISTATE (0x127)
#define UIS_SET 1
#define UISF_HIDEFOCUS (0x1)

//---------------------------------------------------------------------------
// Forward references...
class TDTSColor;
//---------------------------------------------------------------------------

class TWingEditForm : public TForm
{
__published:  // IDE-managed Components
  TMainMenu *MainMenu1;
  TMenuItem *Insert1;
  TMenuItem *ColorCode1;
  TMenuItem *Wingding1;
  TMenuItem *Left1Wing;
  TMenuItem *Left2Wing;
  TMenuItem *Wing1;
  TMenuItem *Left4Wing;
  TMenuItem *Right1Wing;
  TMenuItem *Right2Wing;
  TMenuItem *Right4Wing;
  TLabel *Label5;
  TLabel *Label6;
  TMenuItem *Right5Wing;
  TMenuItem *Left5Wing;
  TMenuItem *Wing2;
  TMenuItem *Wing3;
  TMenuItem *Left3Wing;
  TMenuItem *Right3Wing;
  TLabel *Label7;
  TLabel *Label8;
  TCheckBox *TopBottomBorders;
  TCheckBox *SideBorders;
  TButton *ExitButton;
  TLabel *Label9;
  TMenuItem *BoldCtrlB1;
  TMenuItem *InverseVidio;
  TMenuItem *PlainTextCtrlO1;
  TMenuItem *UnderlineCtrlU1;
  TPanel *EditWingPanel;
  TLabel *Label10;
  TMenuItem *LeftWing6;
  TMenuItem *RightWing6;
  TMenuItem *LeftWing7;
  TMenuItem *RightWing7;
  TMenuItem *View1;
  TMenuItem *MenuViewIrc;
  TMenuItem *N9;
  TMenuItem *N8;
  TMenuItem *N11;
  TMenuItem *ImportFromMainDesignWindow1;
  TListBox *Rwlb;
  TPopupMenu *WingsPopupMenu;
  TMenuItem *AddNewWingPair1;
  TMenuItem *ImportWingFromMainWindow1;
  TMenuItem *DeleteWingPair1;
  TMenuItem *N12;
  TMenuItem *EditSelectedWing1;
  TMenuItem *N13;
  TMenuItem *N1;
  TMenuItem *ExportToTextFile1;
  TMenuItem *ImportFromTextFile1;
  TMenuItem *N2;
  TMenuItem *N3;
  TMenuItem *Exit1;
  TMenuItem *N4;
  TMenuItem *ClearEntireList1;
  TMenuItem *Quit1;
  TButton *Button2;
  TButton *Button3;
  TMenuItem *FontSizeCode1;
  TMenuItem *Copy1;
  TMenuItem *Paste1;
  TMenuItem *N5;
  TMenuItem *Push1;
  TMenuItem *Pop1;
  TMenuItem *N6;
  TTaeRichEdit *TopBottomEdit;
  TTaeRichEdit *LeftSideEdit;
  TTaeRichEdit *RightSideEdit;
  TTaeRichEdit *Edit1;
  TPopupMenu *WingEditorPopupMenu;
  TMenuItem *MenuEditCopy;
  TMenuItem *MenuEditPaste;
  TMenuItem *MenuItem9;
  TMenuItem *MenuEditSave;
  TMenuItem *MenuEditQuit;
  TMenuItem *MenuEditSelectAll;
  TMenuItem *MenuEditToggle;
  TMenuItem *N7;
  TButton *QuitButton;
  TMenuItem *MenuViewRtf;
  TMenuItem *MenuEditCut;
  TMenuItem *Undo1;
  TMenuItem *Undo2;
  TMenuItem *N10;
  TMenuItem *N14;
  TMenuItem *N15;
  TMenuItem *MenuEditDelete;
  TMenuItem *N16;
  TMenuItem *AddTextEffext1;
  TMenuItem *Color1;
  TMenuItem *FontSize1;
  TMenuItem *Bold1;
  TMenuItem *Italics1;
  TMenuItem *Underline1;
  TMenuItem *Help1;
  TMenuItem *Clear1;
  TPopupMenu *ColorCopyPasteMenu;
  TMenuItem *ShowColorMenuItem;
  TMenuItem *N20;
  TMenuItem *CopyColorMenuItem;
  TMenuItem *PasteColorMenuItem;
  TMenuItem *MenuItem1;
  TMenuItem *MenuItem2;
  TMenuItem *MenuItem3;
  TMenuItem *MenuItem4;
  TMenuItem *Optimize1;
  TMenuItem *N17;
  TMenuItem *FontType1;
  TMenuItem *FontTypeCode1;
  TPanel *WingListBoxPanel;
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall ColorCode1Click(TObject *Sender);
  void __fastcall WingClick(TObject *Sender);
  void __fastcall BoldCtrlB1Click(TObject *Sender);
  void __fastcall InverseVidioClick(TObject *Sender);
  void __fastcall PlainTextCtrlO1Click(TObject *Sender);
  void __fastcall UnderlineCtrlU1Click(TObject *Sender);
  void __fastcall MenuViewRtfClick(TObject *Sender);
  void __fastcall MenuViewIrcClick(TObject *Sender);
  void __fastcall ImportFromMainDesignWindow1Click(TObject *Sender);
  void __fastcall ListBoxDblClick(TObject *Sender);
  void __fastcall AddNewWingPair1Click(TObject *Sender);
  void __fastcall DeleteWingPair1Click(TObject *Sender);
  void __fastcall EditSelectedWing1Click(TObject *Sender);
  void __fastcall WingsPopupMenuPopup(TObject *Sender);
  void __fastcall WingEditorPopupMenuPopup(TObject *Sender);
  void __fastcall ImportFromTextFile1Click(TObject *Sender);
  void __fastcall ExportToTextFile1Click(TObject *Sender);
  void __fastcall Exit1Click(TObject *Sender);
  void __fastcall ClearEntireList1Click(TObject *Sender);
  void __fastcall Quit1Click(TObject *Sender);
  void __fastcall Button2Click(TObject *Sender);
  void __fastcall Button3Click(TObject *Sender);
  void __fastcall WingsListBoxKeyDown(TObject *Sender, WORD &Key,
                                                        TShiftState Shift);
  void __fastcall FontSizeCode1Click(TObject *Sender);
  void __fastcall FormClick(TObject *Sender);
  void __fastcall Copy1Click(TObject *Sender);
  void __fastcall Paste1Click(TObject *Sender);
  void __fastcall Push1Click(TObject *Sender);
  void __fastcall Pop1Click(TObject *Sender);
  void __fastcall TopBottomEditMouseDown(TObject *Sender,
                    TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall LwlbMouseDown(TObject *Sender, TMouseButton Button,
                                        TShiftState Shift, int X, int Y);
  void __fastcall RwlbMouseDown(TObject *Sender, TMouseButton Button,
                                          TShiftState Shift, int X, int Y);
  void __fastcall LwlbKeyPress(TObject *Sender, char &Key);
  void __fastcall RwlbKeyPress(TObject *Sender, char &Key);
  void __fastcall RwlbClick(TObject *Sender);
  void __fastcall LwlbClick(TObject *Sender);
  void __fastcall MenuEditQuitClick(TObject *Sender);
  void __fastcall MenuEditSaveClick(TObject *Sender);
  void __fastcall MenuEditToggleClick(TObject *Sender);
  void __fastcall MenuEditSelectAllClick(TObject *Sender);
  void __fastcall Undo1Click(TObject *Sender);
  void __fastcall Undo2Click(TObject *Sender);
  void __fastcall Edit1KeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
  void __fastcall MenuEditPasteClick(TObject *Sender);
  void __fastcall MenuEditCutClick(TObject *Sender);
  void __fastcall MenuEditCopyClick(TObject *Sender);
  void __fastcall MenuEditDeleteClick(TObject *Sender);
  void __fastcall FormCreate(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall Edit1MouseDown(TObject *Sender, TMouseButton Button,
                                          TShiftState Shift, int X, int Y);
  void __fastcall ListBoxExit(TObject *Sender);
  void __fastcall Help1Click(TObject *Sender);
  void __fastcall EditWingPanelMouseDown(TObject *Sender, TMouseButton Button,
                                            TShiftState Shift, int X, int Y);
  void __fastcall Clear1Click(TObject *Sender);
  void __fastcall Optimize1Click(TObject *Sender);
  void __fastcall FontTypeCode1Click(TObject *Sender);
  void __fastcall LwlbDrawItem(TWinControl *Control, int Index,
                                const TRect &Rect, TOwnerDrawState State);
  void __fastcall LwlbMeasureItem(TWinControl *Control, int Index, int &Height);
  void __fastcall RwlbDrawItem(TWinControl *Control, int Index,
                                const TRect &Rect, TOwnerDrawState State);
  void __fastcall RwlbMeasureItem(TWinControl *Control, int Index, int &Height);
  void __fastcall FormDestroy(TObject *Sender);
  void __fastcall LeftSideEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall RightSideEditMouseDown(TObject *Sender,
          TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall FormActivate(TObject *Sender);
  void __fastcall Edit1KeyPress(TObject *Sender, char &Key);
//  void __fastcall EditChange(TObject *Sender,
//              DWORD dwChangeType, void *pvCookieData, bool &AllowChange);
  void __fastcall EditChange(TObject *Sender);
  
private:  // User declarations
//  HFONT __fastcall MakeFont(TListBox* lb);
  void __fastcall CloseOpenSaveDialogs(void);
  bool __fastcall Convert(WideString wIn, const TRect &rect, TCanvas* p);
  bool __fastcall ProcessOC(TTaeRichEdit* re, ONCHANGEW oc);
  void __fastcall ReplaceItem(void* p, String S, int riFlag=RI_NONE);
  void __fastcall AddItems(String Sl, String Sr, bool bChecked=true);
  void __fastcall AddItems(String S, bool bChecked=true);
  WideString __fastcall GetItem(void* p);
  void __fastcall DeleteItems(int idx);
  void __fastcall AddNewWingPair(void);
  PASTESTRUCT __fastcall EditPaste(void);
  PASTESTRUCT __fastcall EditPaste(WideString S);
  PASTESTRUCT __fastcall EditCutCopy(bool bCut, bool bCopy);
  void __fastcall SaveAll(void);
  void __fastcall ShowEdit(String Text, int Type); // UTF-8 string in
  void __fastcall ShowEdit(WideString Text, int Type); // UTF-16 string in
  void __fastcall HideEdit(void);
  void __fastcall DeleteWingPair(void);
  void __fastcall SaveWingEditBoxes(void);
  bool __fastcall LoadWingEditBoxes(void);
  bool __fastcall LoadBorderRichEditBoxes(WideString tb,
                                            WideString lb, WideString rb);
  void __fastcall InsertInEditBox(TMenuItem* P);
  WideString __fastcall ChangeBackgroundColor(WideString Text);
  int __fastcall SaveEdit(void);
  void __fastcall SetMenuInsertItemsState(bool bEnable);
  void __fastcall SetView(int NewView);
  void __fastcall ChangeBorderColors(int Color);
  void __fastcall SetBorderColor(void);
  void __fastcall EmbraceSelection(int i1, int i2 = -1);
//  void __fastcall DrawFocusRect(TCanvas* c, HPEN hpen, TRect &r);
//  HPEN __fastcall CreateFocusPen();

  TMyCheckListBox* Lwlb;

  bool bLeftSelected, bHideCaret, bDoubleclick;
  bool bUnicodeKeySequence;

  bool bSaveFileDialogOpen, bOpenFileDialogOpen;

  unsigned short cbHTML, cbRTF; // Registered clipboard formats

  int OldLength, SourceEditID, CurrentIndex;

  WideString SavedEditString;

  WideString TopBottomEditString, LeftEditString, RightEditString;
  WideString EditString;

//  HFONT g_LwlbHFont, g_RwlbHFont;

  TDTSColor* dts; // easy pointer to main form...

  // Property vars
  int borderBgColor;

//protected:

public:    // User declarations

  __fastcall TWingEditForm(TComponent* Owner);

  // Property handlers
  void __fastcall SetBorderBgColor(int Color);
  int __fastcall GetBorderBgColor(void);

  // Properties
//  __property HFONT LwlbFontHandle = {read = g_LwlbHFont};
//  __property HFONT RwlbFontHandle = {read = g_RwlbHFont};
  __property int BorderBgColor = {read = GetBorderBgColor, write = SetBorderBgColor, nodefault};
};
//---------------------------------------------------------------------------
extern PACKAGE TWingEditForm *WingEditForm;
//---------------------------------------------------------------------------
#endif
