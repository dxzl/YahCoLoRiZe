//---------------------------------------------------------------------------
#ifndef DlgFavColorsH
#define DlgFavColorsH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
//---------------------------------------------------------------------------
class TFavColorsForm : public TForm
{
  friend class TFgBgColorsForm;

__published:	// IDE-managed Components
  TPanel *Panel1;
  TListBox *ListBox;
  TButton *ButtonAddCurrent;
  TButton *ButtonAddNew;
  TButton *ButtonOk;
  TButton *ButtonCancel;
  TLabel *Label1;
  TButton *ButtonDel;
  TMainMenu *MainMenu;
  TMenuItem *File1;
  TMenuItem *Import1;
  TMenuItem *Export1;
  void __fastcall ListBoxDrawItem(TWinControl *Control, int Index,
          TRect &Rect, TOwnerDrawState State);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall ButtonAddCurrentClick(TObject *Sender);
  void __fastcall ButtonAddNewClick(TObject *Sender);
  void __fastcall ButtonDelClick(TObject *Sender);
  void __fastcall FormDestroy(TObject *Sender);
  void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
  void __fastcall FormCreate(TObject *Sender);
  void __fastcall ListBoxClick(TObject *Sender);
  void __fastcall Import1Click(TObject *Sender);
  void __fastcall Export1Click(TObject *Sender);

private:	// User declarations
  void __fastcall Import(void);
  void __fastcall Export(void);
  bool __fastcall ReadIniFile(void);
  int __fastcall ReadColorsFromIni(TIniFile* pIni);
  bool __fastcall WriteColorsToIni(void);

  TFgBgColorsForm* pSCF;
  TDTSColor* dts; // easy pointer to main form...
  int focusIndex, fgColor, bgColor;
  bool bChanged;

public:		// User declarations
  __fastcall TFavColorsForm(TComponent* Owner);

  __property int FgColor = {read = fgColor};
  __property int BgColor = {read = bgColor};
};
//---------------------------------------------------------------------------
extern PACKAGE TFavColorsForm *FavColorsForm;
//---------------------------------------------------------------------------
#endif
