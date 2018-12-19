//---------------------------------------------------------------------------
#ifndef ConvertToHLTH
#define ConvertToHLTH
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------
class TConvertToHLT
{
private:
  void __fastcall ReplaceColortbl(void);
  int __fastcall ScanForString(char CompareStr[], TMemoryStream * MS_Temp,
                             bool bWrite, bool bShowStatus);
  int __fastcall ScanForCtrlK(TMemoryStream * MS_Temp);

  TMemoryStream* ms;
  TMemoryStream* MS_Temp;
  TTaeRichEdit* re;

  TDTSColor* dts; // easy pointer to main form...

public:
  __fastcall TConvertToHLT(TComponent *Owner, TMemoryStream *MS, TTaeRichEdit *RE);
  __fastcall ~TConvertToHLT(void);

  int __fastcall Execute(bool bShowStatus);
};
//---------------------------------------------------------------------------
 extern TConvertToHLT* ConvertToHLT;
//---------------------------------------------------------------------------
#endif
