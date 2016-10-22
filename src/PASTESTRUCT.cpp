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

#include "PASTESTRUCT.h"

#pragma package(smart_init)

//---------------------------------------------------------------------------
// PASTESTRUCT struct functions
//---------------------------------------------------------------------------

__fastcall PASTESTRUCT::PASTESTRUCT(void)
// constructer
{
  Clear();
}

void __fastcall PASTESTRUCT::Clear(void)
// Don't clear index! (we use it to pass an index to a function)
{
  delta = 0; // +/- text pasted or cut including cr/lf as two chars
  lines = 0; // +/- cr/lf pairs in the text pasted or cut
  error = 0; // error code for the cut/paste operation
}

//---------------------------------------------------------------------------

