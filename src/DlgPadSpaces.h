//---------------------------------------------------------------------------
#ifndef DlgPadSpacesH
#define DlgPadSpacesH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Mask.hpp>

// Constants used in EffectParm3
// from RadioGroupBox...
#define PS_PAD_R        0
#define PS_PAD_L        1
#define PS_PAD_LR       2
#define PS_CENTER       3
#define PS_L_JUSTIFY    4
#define PS_R_JUSTIFY    5
#define PS_STRIP_R      6
#define PS_STRIP_L      7
#define PS_STRIP_LR     8
//---------------------------------------------------------------------------
class TPadSpacesForm : public TForm
{
__published:	// IDE-managed Components
  TPanel *PanelMain;
  TMaskEdit *MaskEditWidth;
  TLabel *LabelWidth;
  TRadioGroup *RadioGroupMode;
  TCheckBox *CheckBoxAutoWidth;
  TButton *ButtonRefresh;
  TButton *ButtonOk;
  TButton *ButtonCancel;
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall CheckBoxAutoWidthClick(TObject *Sender);
private:	// User declarations
  int FMode, FWidth;
  bool FAuto;
public:		// User declarations
  __fastcall TPadSpacesForm(TComponent* Owner);

  __property int Mode = {read = FMode, write = FMode};
  __property int Width = {read = FWidth, write = FWidth};
  __property bool Auto = {read = FAuto, write = FAuto};
};
//---------------------------------------------------------------------------
extern PACKAGE TPadSpacesForm *PadSpacesForm;
//---------------------------------------------------------------------------
#endif
