//---------------------------------------------------------------------------
#ifndef DlgFontTypeH
#define DlgFontTypeH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
//---------------------------------------------------------------------------
#define WIN32LISTPOPULATION false
//---------------------------------------------------------------------------

class TFontTypeForm : public TForm
{
__published:  // IDE-managed Components
  TButton *ButtonOk;
  TButton *ButtonCancel;
  TListBox *FontTypeListBox;
  TLabel *FontTypeLabel;
  TPopupMenu *PopupMenu1;
  TMenuItem *Copy1;
  void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall FormCreate(TObject *Sender);
  void __fastcall Copy1Click(TObject *Sender);
  void __fastcall FormDestroy(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
private:  // User declarations
  void __fastcall PopulateListBox(void);
  int fontType;

  TStringList* g_sl;

  TDTSColor* dts; // easy pointer to main form...

public:    // User declarations
  __fastcall TFontTypeForm(TComponent* Owner);

  __property int FontType = { read = fontType, write = fontType };
};
//---------------------------------------------------------------------------
extern PACKAGE TFontTypeForm *FontTypeForm;
//---------------------------------------------------------------------------
#endif
