//---------------------------------------------------------------------------
// YahCoLoRiZe - Edit, process and send colorized text into IRC chat-rooms
// via various chat-clients such as mIRC, IceCHat and LeafCHat
//
// Author: Scott Swift
//
// Released to GitHub under GPL v3 October, 2016
//
//---------------------------------------------------------------------------
#include <vcl.h>
#include <Controls.hpp>
#include "Main.h"
// Include header files common to all files before this directive!
#pragma hdrstop

#include "Pushstruct.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------
// PUSHSTRUCT struct functions
//---------------------------------------------------------------------------

__fastcall PUSHSTRUCT::PUSHSTRUCT(void)
// constructer
{
  Clear();
}

void __fastcall PUSHSTRUCT::Clear(void)
// Don't clear index! (we use it to pass an index to a function)
{
  ClearColors();
  ClearFont();
  ClearStyle();
  index = 0; // Buffer index
  crlfs = 0; // CR/LF Count
  pushCounter = 0;
}

bool __fastcall PUSHSTRUCT::IsClear(void)
// Don't clear index! (we use it to pass an index to a function)
{
  return fg == NO_COLOR && bg == NO_COLOR && fontSize == -1 && fontType == -1;
}

void __fastcall PUSHSTRUCT::ClearColors(void)
// Don't clear index! (we use it to pass an index to a function)
{
  fg = NO_COLOR;
  bg = NO_COLOR;
  bFGfast = false;
  bBGfast = false;
}

void __fastcall PUSHSTRUCT::ClearFont(void)
// Don't clear index! (we use it to pass an index to a function)
{
  fontSize = -1;
  fontType = -1;
  bFontSizefast = false;
  bFontTypefast = false;
}

void __fastcall PUSHSTRUCT::ClearStyle(void)
// Don't clear index! (we use it to pass an index to a function)
{
  bBold = false;
  bUnderline = false;
  bItalics = false;
  bPush = false;
  bPop = false;
}
//---------------------------------------------------------------------------

