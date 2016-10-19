//---------------------------------------------------------------------------
#ifndef BlenderH
#define BlenderH

//---------------------------------------------------------------------------
#include <Classes.hpp>

// Need the BlendColor typedef in Main.h
typedef struct
{
  int red;
  int green;
  int blue;
  bool enabled;
} BlendColor;

#include "Utils.h"
//---------------------------------------------------------------------------
//#include "Main.h"
//---------------------------------------------------------------------------

class TBlendEngine
{
  private:

    // need this to avoid sending too much data to yahoo
    // works with RGBThreshold
    BlendColor BCLast;

    int SectionSize, SectionMarker;
    int BlendColorIndex, BlendColorStep;
    int Red[2], Green[2], Blue[2]; // From-To target colors

    // Amount to inc/dec color per "RedCharStep", "GreenCharStep"
    // or "BlueCharStep" chars.
    double OutRed, OutGreen, OutBlue; // Computed RGB to output
    double RedStep, GreenStep, BlueStep;

    void __fastcall ComputeRGBSteps(void);
    void __fastcall CheckLimits(void);
    void __fastcall SetRGB(int idx, BlendColor BC);
    bool __fastcall ColorChangeAboveThreshold(BlendColor BC);

  public:
    __fastcall TBlendEngine(void); // constructor
    __fastcall ~TBlendEngine(void); // destructor
    int __fastcall Init(int RealLength);
    int __fastcall Blend(int idx);

    BlendColor* EBC;
    int BlendEnabledCount;
    int Threshold;
};
//---------------------------------------------------------------------------
extern TBlendEngine* BlendEngine;
extern TBlendEngine* FG_BlendEngine;
extern TBlendEngine* BG_BlendEngine;
#endif
