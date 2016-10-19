//---------------------------------------------------------------------------
#ifndef DlgStripH
#define DlgStripH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//#include "..\TaeRichEdit\TaeRichEdit.h"
#include "Main.h"
//---------------------------------------------------------------------------

#define RADIO_OFFSET 4 // Start offset into STRIPDLG[]
#define RADIO_COUNT 13 // Number of items

class TStripModeForm : public TForm
{
__published:  // IDE-managed Components
  TRadioGroup *RadioGroup;
  TButton *OkButton;
  TButton *CancelButton;
  void __fastcall OkButtonClick(TObject *Sender);
  void __fastcall CancelButtonClick(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:  // User declarations

  int stripMode;

public:    // User declarations
  __fastcall TStripModeForm(TComponent* Owner);

  __property int StripMode = { read = stripMode, write = stripMode,
                                                  default = STRIP_ALL };
};
//---------------------------------------------------------------------------
extern PACKAGE TStripModeForm *StripModeForm;
//---------------------------------------------------------------------------
#endif
