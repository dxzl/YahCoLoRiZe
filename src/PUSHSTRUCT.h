//---------------------------------------------------------------------------
#ifndef PushstructH
#define PushstructH

#include <Classes.hpp>
//---------------------------------------------------------------------------

struct PUSHSTRUCT
{
  int fg;
  int bg;
  int fontSize;
  int fontType;
  unsigned int index; // place to keep an index...
  int crlfs; // count of CR/LFs
  int pushCounter; // # pushes
  bool bBold;
  bool bUnderline;
  bool bItalics; // reverse for mIRC chat
  bool bFGfast;
  bool bBGfast;
  bool bFontSizefast;
  bool bFontTypefast;
  bool bPush;
  bool bPop;

  __fastcall PUSHSTRUCT(void); // constructor

  void __fastcall ClearColors(void);
  void __fastcall ClearFont(void);
  void __fastcall ClearStyle(void);
  void __fastcall Clear(void);
  bool __fastcall IsClear(void);
};
//---------------------------------------------------------------------------
#endif
