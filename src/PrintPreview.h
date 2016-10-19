//---------------------------------------------------------------------------
#ifndef PrintPreviewH
#define PrintPreviewH

#include <Classes.hpp>
//---------------------------------------------------------------------------

class TPrintPreview
{
  private:
    
    TDTSColor* dts; // easy pointer to main form...

  public:
    __fastcall TPrintPreview(TComponent* Owner); // constructor
    __fastcall ~TPrintPreview(void); // destructor

    bool __fastcall Preview(TTaeRichEdit* re);
};
//---------------------------------------------------------------------------
extern TPrintPreview* PrintPreview;
//---------------------------------------------------------------------------
#endif
