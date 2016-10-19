//---------------------------------------------------------------------------
#ifndef DlgAlternateH
#define DlgAlternateH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TAlternateForm : public TForm
{
__published:  // IDE-managed Components
  TRadioGroup *RadioGroup;
  TCheckBox *SkipSpacesCheckBox;
  TButton *ButtonOk;
  TButton *ButtonCancel;
  TPanel *AlternatePanelColorA;
  TPanel *AlternatePanelColorB;
  TLabel *LabelColorA;
  TLabel *LabelColorB;
  void __fastcall FormShow(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall AlternatePanelColorAMouseDown(TObject *Sender,
                    TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall AlternatePanelColorBMouseDown(TObject *Sender,
                      TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
private:  // User declarations
  int colorA, colorB, mode;
  String dlgCaption;
  bool bSkipSpaces, bShowColors;
protected:
  void __fastcall SetColorB(int val);
  void __fastcall SetColorA(int val);
public:    // User declarations
  __fastcall TAlternateForm(TComponent* Owner);

  __property int ColorA = { read = colorA, write = SetColorA };
  __property int ColorB = { read = colorB, write = SetColorB };
  __property int Mode = { read = mode, write = mode };
  __property bool SkipSpaces = { read = bSkipSpaces, write = bSkipSpaces };
  __property bool ShowColors = { read = bShowColors, write = bShowColors };
  __property String DlgCaption = { read = dlgCaption, write = dlgCaption };
};
//---------------------------------------------------------------------------
extern PACKAGE TAlternateForm *AlternateForm;
//---------------------------------------------------------------------------
#endif
