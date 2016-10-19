//---------------------------------------------------------------------------
#ifndef DlgBlendH
#define DlgBlendH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TBlendChoiceForm : public TForm
{
__published:  // IDE-managed Components
  TPanel *Blend0;
  TPanel *Blend1;
  TPanel *Blend2;
  TPanel *Blend3;
  TPanel *Blend4;
  TPanel *Blend5;
  TPanel *Blend6;
  TPanel *Blend7;
  TPanel *Blend8;
  TPanel *Blend9;
  TPanel *OK;
  void __fastcall OKClick(TObject *Sender);
  void __fastcall PresetPanelClick(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:  // User declarations
  void __fastcall PaintPanels( BlendColor * BC );

  BlendColor * BC;

public:    // User declarations
  __fastcall TBlendChoiceForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TBlendChoiceForm *BlendChoiceForm;
//---------------------------------------------------------------------------
#endif
