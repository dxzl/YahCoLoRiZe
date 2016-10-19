//---------------------------------------------------------------------------
#ifndef UndoH
#define UndoH

#include <Classes.hpp>
#include "ThreadOnChange.h"
//---------------------------------------------------------------------------
#define MAX_UNDO 1000

// Undo Modes
#define UNDO_NONE              0 // no operation
#define UNDO_CUT               1 // cut text
#define UNDO_PASTE             2 // paste text
#define UNDO_DEL               3 // remove char
#define UNDO_INS               4 // add char
#define UNDO_PROCESS           5 // pressed Process or Show
#define UNDO_EFFECT_ORIG_TEXT  6 // step 1 save orig selected text in String s
#define UNDO_EFFECT_NEW_TEXT   7 // step 2 is the range of modified text to cut
#define UNDO_FG_COLOR          8
#define UNDO_BG_COLOR          9
#define UNDO_WING_COLOR       10
#define UNDO_SHOW             11
#define UNDO_REPLACE          12
#define UNDO_CLEAR            13
// these are for a short-and-sweet undo that uses minimal memory
// I use these in the character-processing thread. Must be 100+
#define US_START              100
#define UNDO_DEL_SHORT        (US_START+0)
#define UNDO_INS_SHORT        (US_START+1)

//---------------------------------------------------------------------------
// Struct
//---------------------------------------------------------------------------
// Type should be the first int in any structure permutation because
// we check Type to determine which pointer type to cast to!
struct UNDOITEM
{
  int Type;
  int Index;
  int Length;
  bool bChain; // if set, we auto undo the next object up the chain...
  ONCHANGEW oc;
  WideString s;

  // constructor
  __fastcall UNDOITEM(int Type, int Index, int Length, ONCHANGEW oc,
                                                 WideString s, bool bChain);
};

// Type should be the first int in any structure permutation because
// we check Type to determine which pointer type to cast to!
struct UNDOSHORT
{
  friend class TStringsW;

  int type;
  int view;
  int len;
  TStringsW* p;
  TPoint pos;
  TPoint loc;
  wchar_t c;
  bool bInsert;
  bool bChain;

  // constructor
  __fastcall UNDOSHORT(int Type, int View, int Len, void* P, TPoint Pos,
                              TPoint Loc, wchar_t C, bool bInsert, bool bChain);
};
//---------------------------------------------------------------------------
// Class
//---------------------------------------------------------------------------
class TTOCUndo
{
private:
  bool __fastcall DoUndo(void);

  TList* pTList; // Pointer to a TList of UNDOITEM struct pointers

public:
  __fastcall TTOCUndo(void); // constructor
  __fastcall ~TTOCUndo(void); // destructor

  void __fastcall Clear(void);
  void __fastcall ClearMost(void);
  int __fastcall GetItemsCount(void);
  void __fastcall Add(int Type, int View, int Len, void* P, TPoint Pos,
                     TPoint Loc, wchar_t C, bool bInsert, bool bChain=false);
  void __fastcall Add(int Type, int Index, int Length, ONCHANGEW oc,
                                       WideString s, bool bChain=false);
  bool __fastcall Delete(int idx);
  void __fastcall Undo(void);

  __property TList* UndoList = {read = pTList, write = pTList};
  __property int Count = {read = GetItemsCount};
};
//---------------------------------------------------------------------------
extern TTOCUndo* TOCUndo;
//---------------------------------------------------------------------------
#endif
