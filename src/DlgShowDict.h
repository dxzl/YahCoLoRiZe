//---------------------------------------------------------------------------
#ifndef DlgShowDictH
#define DlgShowDictH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TShowDictForm : public TForm
{
__published:  // IDE-managed Components
  TListBox *DictListBox;
  TButton *DelButton;
  TButton *CloseButton;
  TButton *HelpButton;
  void __fastcall DelButtonClick(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall CloseButtonClick(TObject *Sender);
  void __fastcall HelpButtonClick(TObject *Sender);
  void __fastcall DictListBoxClick(TObject *Sender);
  void __fastcall DictListBoxKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall DictListBoxDrawItem(TWinControl *Control, int Index,
          TRect &Rect, TOwnerDrawState State);
private:  // User declarations
  int OriginalItemCount;
  bool bItemsDeleted;
public:    // User declarations
  __fastcall TShowDictForm(TComponent* Owner);

  __property bool ItemsDeleted = {read = bItemsDeleted};
};
//---------------------------------------------------------------------------
extern PACKAGE TShowDictForm *ShowDictForm;
//---------------------------------------------------------------------------
#endif
