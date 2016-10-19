//---------------------------------------------------------------------------
#ifndef DlgUnderlineH
#define DlgUnderlineH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>

// Constants used in EffectParm3
// from RadioGroupBox...
#define AL_ULCOLOR        0
#define AL_ULBOLD         1
#define AL_ULITALICS      2
#define AL_BOLDITALICS    3
#define AL_BOLDCOLOR      4
#define AL_ITALICSCOLOR   5
#define AL_COLOR          6
//---------------------------------------------------------------------------
class TUnderlineForm : public TForm
{
__published:  // IDE-managed Components
  TRadioGroup *RadioGroup;
  TButton *ButtonOk;
  TButton *ButtonCancel;
  TPanel *UnderlinePanelColorA;
  TLabel *LabelColorA;
  TPanel *UnderlinePanelColorB;
  TLabel *LabelColorB;
  void __fastcall FormShow(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall UnderlinePanelColorAMouseDown(TObject *Sender,
                      TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall UnderlinePanelColorBMouseDown(TObject *Sender,
                        TMouseButton Button, TShiftState Shift, int X, int Y);
private:  // User declarations
  int dlgColorA, dlgColorB, mode;
  String dlgCaption;
protected:
  void __fastcall SetColorA(int val);
  void __fastcall SetColorB(int val);
public:    // User declarations
  __fastcall TUnderlineForm(TComponent* Owner);

  __property int DlgColorA = { read = dlgColorA, write = SetColorA };
  __property int DlgColorB = { read = dlgColorB, write = SetColorB };
  __property int Mode = { read = mode, write = mode };
  __property String DlgCaption = { read = dlgCaption, write = dlgCaption };
};
//---------------------------------------------------------------------------
extern PACKAGE TUnderlineForm *UnderlineForm;
//---------------------------------------------------------------------------
#endif
