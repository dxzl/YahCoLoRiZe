//---------------------------------------------------------------------------
#ifndef DlgChooseDictH
#define DlgChooseDictH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TChooseDictForm : public TForm
{
__published:  // IDE-managed Components
  TListBox *DictListBox;
  TButton *OkButton;
  void __fastcall OkButtonClick(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall DictListBoxClick(TObject *Sender);
  void __fastcall DictListBoxDrawItem(TWinControl *Control, int Index,
          TRect &Rect, TOwnerDrawState State);
private:  // User declarations
  void __fastcall CheckFile(WideString wDictPath, WideString wFile);

  WideString wDict;
  int count; // number of dictionaries

  TDTSColor* dts; // easy pointer to main form...

public:    // User declarations
  __fastcall TChooseDictForm(TComponent* Owner);

  __property WideString Dict = {read = wDict, write = wDict};
  __property int NumDictionaries = {read = count};
};
//---------------------------------------------------------------------------
extern PACKAGE TChooseDictForm *ChooseDictForm;
//---------------------------------------------------------------------------
#endif
