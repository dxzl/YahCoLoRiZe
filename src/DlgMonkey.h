//---------------------------------------------------------------------------
#ifndef DlgMonkeyH
#define DlgMonkeyH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
//---------------------------------------------------------------------------
class TMonkeyForm : public TForm
{
__published:  // IDE-managed Components
  TTimer *Timer1;
  TPanel *RevPanel;
  TImage *Image1;
  void __fastcall Image1Click(TObject *Sender);
  void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall Timer1Timer(TObject *Sender);
private:  // User declarations
  int EndRight, EndBottom;
  int * pPalSave;
  double xInc, yInc;
  double left, top;
  bool bReverse;
public:    // User declarations
  __fastcall TMonkeyForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMonkeyForm *MonkeyForm;
//---------------------------------------------------------------------------
#endif
