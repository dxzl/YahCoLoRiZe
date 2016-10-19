//---------------------------------------------------------------------------
#ifndef DlgSetColorsH
#define DlgSetColorsH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TSetColorsForm : public TForm
{
__published:  // IDE-managed Components
  TRadioGroup *RadioGroup;
  TPanel *SetColorsFgPanel;
  TPanel *SetColorsBgPanel;
  TLabel *FgLabel;
  TLabel *BgLabel;
  TButton *ButtonOk;
  TButton *ButtonCancel;
  void __fastcall FormShow(TObject *Sender);
  void __fastcall RadioGroupClick(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall SetColorsFgPanelMouseDown(TObject *Sender,
                    TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall SetColorsBgPanelMouseDown(TObject *Sender,
                      TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
protected:
  void __fastcall SetFgColor(int val);
  void __fastcall SetBgColor(int val);
private:  // User declarations
  void __fastcall SaveSettings( void );

  int fgColor, bgColor, mode;
public:    // User declarations
  __fastcall TSetColorsForm(TComponent* Owner);

  __property int FgColor = { read = fgColor, write = SetFgColor };
  __property int BgColor = { read = bgColor, write = SetBgColor };
  __property int Mode = { read = mode, write = mode };
};
//---------------------------------------------------------------------------
extern PACKAGE TSetColorsForm *SetColorsForm;
//---------------------------------------------------------------------------
#endif
