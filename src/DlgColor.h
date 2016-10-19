//---------------------------------------------------------------------------
#ifndef DlgColorH
#define DlgColorH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------

// Constants for special button's Tag #
// Corresponds to COLORDIALOGMSG[] in DefaultStrings.cpp
#define P_BEGIN        100
#define P_VBLEND      (P_BEGIN+0)
#define P_RGB         (P_BEGIN+1)
#define P_HBLEND      (P_BEGIN+2)
#define P_RANDOM      (P_BEGIN+3)
#define P_TRANSPARENT (P_BEGIN+4)
#define P_CANCEL      (P_BEGIN+5)
//---------------------------------------------------------------------------
class TColorForm : public TForm
{
__published:  // IDE-managed Components
  TPanel *ColorPanel;
  TPanel *ButtonsPanel;
  void __fastcall ColorPanelClick(TObject *Sender);
  void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
  void __fastcall PanelEnter(TObject *Sender);
  void __fastcall PanelExit(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall FormMouseMove(TObject *Sender, TShiftState Shift, int X,
          int Y);
  void __fastcall FormCreate(TObject *Sender);
private:  // User declarations
  int dlgColor, randRGB, callerID, initColor;

  TList* panels;

public:    // User declarations
  __fastcall TColorForm(TComponent* Owner);

  __property int DlgColor = { read = dlgColor, write = dlgColor };
  __property int InitColor = { read = initColor, write = initColor };
  __property int RandRGB = { read = randRGB, write = randRGB };
  __property int CallerID = { read = callerID, write = callerID };
};
//---------------------------------------------------------------------------
extern PACKAGE TColorForm *ColorForm;
//---------------------------------------------------------------------------
#endif
