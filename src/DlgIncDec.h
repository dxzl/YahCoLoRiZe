//---------------------------------------------------------------------------
#ifndef DlgIncDecH
#define DlgIncDecH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>

// EFFECT_INC_FGBG constants used in EffectParm3
// from RadioGroupBox...
#define EM_FG   0
#define EM_BG   1
#define EM_FGBG 2

#define INCDEC_RED    17
#define INCDEC_GREEN  -17
#define INCDEC_BLUE   17
//---------------------------------------------------------------------------

class TIncDecForm : public TForm
{
__published:  // IDE-managed Components
  TEdit *EditRed;
  TEdit *EditGreen;
  TEdit *EditBlue;
  TUpDown *UpDownRed;
  TUpDown *UpDownGreen;
  TUpDown *UpDownBlue;
  TButton *ButtonOk;
  TButton *ButtonCancel;
  TLabel *RedLabel;
  TLabel *GreenLabel;
  TLabel *BlueLabel;
  TRadioGroup *RadioGroup;
  TCheckBox *SkipSpacesCheckBox;
  TEdit *EditOffset;
  TUpDown *UpDownOffset;
  TLabel *OffsetLabel;
  void __fastcall FormShow(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:  // User declarations
  int redDelta, greenDelta, blueDelta, offset, mode, paletteSize;
  bool bEnaEdits, bSkipSpaces;
public:    // User declarations
  __fastcall TIncDecForm(TComponent* Owner);

  __property int RedDelta = { read = redDelta, write = redDelta };
  __property int GreenDelta = { read = greenDelta, write = greenDelta };
  __property int BlueDelta = { read = blueDelta, write = blueDelta };
  __property int Offset = { read = offset, write = offset };
  __property int Mode = { read = mode, write = mode };
  __property int PaletteSize = { write = paletteSize };
  __property bool EnableEdits = { read = bEnaEdits, write = bEnaEdits };
  __property bool SkipSpaces = { read = bSkipSpaces, write = bSkipSpaces };
};
//---------------------------------------------------------------------------
extern PACKAGE TIncDecForm *IncDecForm;
//---------------------------------------------------------------------------
#endif
