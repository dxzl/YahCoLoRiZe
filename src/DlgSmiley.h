//---------------------------------------------------------------------------
#ifndef DlgSmileyH
#define DlgSmileyH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
#include <ImgList.hpp>
#include <ToolWin.hpp>
#include <jpeg.hpp>
//---------------------------------------------------------------------------
class TSmileyForm : public TForm
{
__published:  // IDE-managed Components
  void __fastcall FormClose(TObject* Sender, TCloseAction &Action);
  void __fastcall FormKeyDown(TObject* Sender, WORD &Key,
          TShiftState Shift);
  void __fastcall FormClick(TObject* Sender);
  void __fastcall FormCreate(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
private:  // User declarations
  String sTitle;
  TStringsW* EmoteStrings;
  TStringsW* TempStrings;
public:    // User declarations
  __fastcall TSmileyForm(TComponent* Owner);
  void __fastcall MouseClick(TObject* Sender);

  __property String SmileyTitle = {read = sTitle, write = sTitle};
};
//---------------------------------------------------------------------------
extern PACKAGE TSmileyForm *SmileyForm;
//---------------------------------------------------------------------------
#endif
