//---------------------------------------------------------------------------
#ifndef DlgFontSizeH
#define DlgFontSizeH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TFontSizeForm : public TForm
{
__published:  // IDE-managed Components
  TTrackBar *TrackBar1;
  TLabel *Label1;
  TLabel *Label2;
  TLabel *Label3;
  TLabel *Label4;
  TButton *ButtonOk;
  TCheckBox *CheckBox1;
  TButton *ButtonCancel;
  void __fastcall CheckBox1Click(TObject *Sender);
  void __fastcall TrackBar1Change(TObject *Sender);
  void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall FormShow(TObject *Sender);

private:  // User declarations
  int fontSize;

  TDTSColor* dts; // easy pointer to main form...

public:    // User declarations
  __fastcall TFontSizeForm(TComponent* Owner);

  __property int FontSize = { read = fontSize, write = fontSize };
};
//---------------------------------------------------------------------------
extern PACKAGE TFontSizeForm *FontSizeForm;
//---------------------------------------------------------------------------
#endif
