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

#include "Blender.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------
// Blender class functions
//---------------------------------------------------------------------------
__fastcall TBlendEngine::TBlendEngine(void)
// constructor
{
  // Make a private buffer big enough
  EBC = new BlendColor[MAXBLENDCOLORS*2];

  // Initialize
  // (they are set later in Main.cpp in CreatBlendEngine())
  BlendEnabledCount = 0;
  Threshold = 1;
}
//----------------------------------------------------------------------------
int __fastcall TBlendEngine::Init(int RealLength)
// Call one time for the first char
// Returns the first blend-color to write
{
  // EnabledBlendColors - array of BlendColor structs
  // BlendEnabledCount - number of blend-colors enabled (in array)
  // RGBThreshold - min amount one color must change by to
  //                print a new color-string
  // OnProcessCheckBox - run blender first when Process pressed
  // ResolveToPaletteCheckBox - Resolve to code-generated palette
  // BlendScopeRadioButtons->ItemIndex (0-4)
  // BlendDirectionRadioButtons->ItemIndex (0-2)

  if (BlendEnabledCount < 2 || RealLength < 2)
  {
    SectionSize = -1; // disabled
    return(BLEND_ERR1); // error
  }

  if (RealLength >= BlendEnabledCount)
  {
    SectionSize = (RealLength/(BlendEnabledCount-1))-1;

    if (SectionSize <= 0) SectionSize = 1;

    // Active "buttons" to advance by per section
    BlendColorStep = 1;
  }
  else
  {
    SectionSize = 1; // 1 char per section

    // Active "buttons" to advance by per char
    BlendColorStep = BlendEnabledCount/(RealLength-1);

    if (BlendColorStep <= 0)
      BlendColorStep = 1;
  }

//  ShowMessage("SectionSize=" + String(SectionSize)
//                + " BlendStep=" + String(BlendColorStep) +
//                 + " EnableCount=" + String(BlendEnabledCount) +
//                    " RealLength=" + String(RealLength));

  BlendColorIndex = 1;
  SectionMarker = 0;

  // Get first two color-buttons in [0] and [1]
  for (int jj = 0 ; jj < 2 ; jj++)
    SetRGB(jj, EBC[jj]);

  // How much color to add per character (float)
  // within a particular section. Can be a +/-
  // fraction of 1.
  ComputeRGBSteps();

  // Initial output color
  OutRed = (double)Red[0];
  OutGreen = (double)Green[0];
  OutBlue = (double)Blue[0];

  // Form output string for first char
  BlendColor BC;

  BC.red = Red[0];
  BC.green = Green[0];
  BC.blue = Blue[0];

  // Init Last to this preset's color
  BCLast = BC;

  int Value = -utils->BlendColorToRgb(BC);

  return Value;
}
//----------------------------------------------------------------------------
int __fastcall TBlendEngine::Blend(int idx)
// Call for remaining chars
//
// idx should be the "real index" into the string of chars you
// are blending, NOT the physical index into the buffer (which
// includes all the formatting data)
{
  if (SectionSize < 0) // enabled to blend?
    return(BLEND_ERR1); // error (no color has this as an index)

  if (idx-SectionMarker > SectionSize)
  {
    // Next section
    BlendColorIndex += BlendColorStep; // next color-button

    if (BlendColorIndex >= BlendEnabledCount)
      BlendColorIndex = BlendEnabledCount-1;

    // Shift array
    Red[0] = Red[1];
    Green[0] = Green[1];
    Blue[0] = Blue[1];

    // Get next button's color in [1]
    SetRGB(1, EBC[BlendColorIndex]);

    // How much color to add per character (float)
    // within a particular section. Can be a +/-
    // fraction of 1.
    ComputeRGBSteps();

    // Output color
    OutRed = (double)Red[0];
    OutGreen = (double)Green[0];
    OutBlue = (double)Blue[0];

    // Set a marker at section-boundary
    SectionMarker = idx;
  }
  else
  {
    // Next Computed output RGB color
    OutRed += RedStep;
    OutGreen += GreenStep;
    OutBlue += BlueStep;
  }

  // Limit checking...
  CheckLimits();

  // Form output string for this char
  BlendColor BC;
  BC.red = (int)OutRed;
  BC.green = (int)OutGreen;
  BC.blue = (int)OutBlue;

  // Compare this new color to the one for the
  // last char... if any of the three colors have
  // changed more than RGBThreshold, write this
  // color-code...
  int Value;

  if (ColorChangeAboveThreshold(BC))
  {
    Value = -utils->BlendColorToRgb(BC);

    // Present becomes Last...
    BCLast = BC;
  }
  else
    Value = NO_COLOR;

  return Value;
}
//----------------------------------------------------------------------------
__fastcall TBlendEngine::~TBlendEngine(void)
// destructor
{
  delete [] EBC;
}
//----------------------------------------------------------------------------
void __fastcall TBlendEngine::ComputeRGBSteps(void)
{
  double delta = Red[1]-Red[0];
  RedStep = delta/SectionSize;
  delta = Green[1]-Green[0];
  GreenStep = delta/SectionSize;
  delta = Blue[1]-Blue[0];
  BlueStep = delta/SectionSize;
}
//----------------------------------------------------------------------------
void __fastcall TBlendEngine::CheckLimits(void)
{
  if (OutRed > 255.)
    OutRed = 255.;
  if (OutGreen > 255.)
    OutGreen = 255.;
  if (OutBlue > 255.)
    OutBlue = 255.;

  if (OutRed < 0.)
    OutRed = 0.;
  if (OutGreen < 0.)
    OutGreen = 0.;
  if (OutBlue < 0.)
    OutBlue = 0.;
}
//----------------------------------------------------------------------------
void __fastcall TBlendEngine::SetRGB(int idx, BlendColor BC)
{
  Red[idx] = BC.red;
  Green[idx] = BC.green;
  Blue[idx] = BC.blue;
}
//----------------------------------------------------------------------------
bool __fastcall TBlendEngine::ColorChangeAboveThreshold(BlendColor BC)
// Return true if any color has changed by more than
// the Threshold parameter
{
  int C;

  if (SectionSize == 1)
    C = 1;
  else
    C = Threshold;

  if (abs(BC.red - BCLast.red) >= C ||
      abs(BC.green - BCLast.green) >= C ||
         abs(BC.blue - BCLast.blue) >= C)
    return true;

  return false;
}
//---------------------------------------------------------------------------
// End Blender Functions
//---------------------------------------------------------------------------
