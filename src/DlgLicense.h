//---------------------------------------------------------------------------
#ifndef DlgLicenseH
#define DlgLicenseH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TLicenseForm : public TForm
{
__published:  // IDE-managed Components
  TEdit *Edit1;
  TEdit *Edit2;
  TLabel *Label1;
  TLabel *Label2;
  TButton *ButtonOK;
  TButton *ButtonCancel;
  void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
private:  // User declarations
public:    // User declarations
  __fastcall TLicenseForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TLicenseForm *LicenseForm;
//---------------------------------------------------------------------------
#endif
