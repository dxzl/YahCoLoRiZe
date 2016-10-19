//---------------------------------------------------------------------------
#ifndef DlgFgBgColorsH
#define DlgFgBgColorsH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TFgBgColorsForm : public TForm
{
__published:	// IDE-managed Components
  TPanel *Panel1;
  TButton *ButtonOk;
  TButton *ButtonCancel;
  TPanel *SetColors2BgPanel;
  TPanel *SetColors2FgPanel;
  TLabel *BgLabel;
  TLabel *FgLabel;

  void __fastcall FormShow(TObject *Sender);
  void __fastcall SetColors2FgPanelMouseDown(TObject *Sender,
                    TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall SetColors2BgPanelMouseDown(TObject *Sender,
                      TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
protected:
  void __fastcall SetFgColor(int val);
  void __fastcall SetBgColor(int val);

private:	// User declarations
  int fgColor, bgColor;

  TDTSColor* dts; // easy pointer to main form...
  
public:		// User declarations
  __fastcall TFgBgColorsForm(TComponent* Owner);

  __property int FgColor = { read = fgColor, write = SetFgColor };
  __property int BgColor = { read = bgColor, write = SetBgColor };
};
//---------------------------------------------------------------------------
extern PACKAGE TFgBgColorsForm *FgBgColorsForm;
//---------------------------------------------------------------------------
#endif
