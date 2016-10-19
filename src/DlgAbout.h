//---------------------------------------------------------------------------
#ifndef DlgAboutH
#define DlgAboutH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
//---------------------------------------------------------------------------
class TAboutForm : public TForm
{
__published:  // IDE-managed Components
  TButton *ButtonClose;
  TPanel *Panel1;
  TPanel *Panel2;
  TImage *Image1;
  TImage *Image2;
  TLabel *Label1;
  TLabel *RevLabel;
  TButton *EnterKeyButton;
  void __fastcall EnterKeyButtonClick(TObject *Sender);
  void __fastcall WebSiteClick(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:  // User declarations
  void __fastcall DisplayDaysRemaining( void );
public:    // User declarations
  __fastcall TAboutForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TAboutForm *AboutForm;
//---------------------------------------------------------------------------
#endif
