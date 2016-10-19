//---------------------------------------------------------------------------
#ifndef PASTESTRUCTH
#define PASTESTRUCTH

#include <Classes.hpp>
//---------------------------------------------------------------------------

struct PASTESTRUCT
{
  int delta;
  int lines;
  int error;

  __fastcall PASTESTRUCT(void); // constructor

  void __fastcall Clear(void);
};
//---------------------------------------------------------------------------
#endif
