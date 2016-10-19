//---------------------------------------------------------------------------
#ifndef DlgMorphH
#define DlgMorphH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TMorphForm : public TForm
{
__published:  // IDE-managed Components
  TButton *OkButton;
  TButton *CancelButton;
  TGroupBox *MorphGroupBox;
  TLabel *FromLabel;
  TLabel *ToLabel;
  TLabel *MaxColorsLabel;
  TPanel *MorphFgFromPanel;
  TPanel *MorphBgFromPanel;
  TPanel *MorphFgToPanel;
  TPanel *MorphBgToPanel;
  TCheckBox *FgCheckBox;
  TCheckBox *BgCheckBox;
  TEdit *MaxColorsEdit;
  TUpDown *MaxColorsUpDown;
  void __fastcall FgCheckBoxClick(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall MorphFgFromPanelMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall MorphBgFromPanelMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall MorphFgToPanelMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall MorphBgToPanelMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall BgCheckBoxClick(TObject *Sender);
  void __fastcall FormKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
private:  // User declarations
  String FTitle;
  bool fgChecked, bgChecked, FHideMaxColors;
  int fgToColor, fgFromColor;
  int bgToColor, bgFromColor;
  int maxColors;
protected:
  void __fastcall SetFgToColor(int val);
  void __fastcall SetBgToColor(int val);
  void __fastcall SetFgFromColor(int val);
  void __fastcall SetBgFromColor(int val);
public:    // User declarations
  __fastcall TMorphForm(TComponent* Owner);

  __property int FgToColor = { read = fgToColor, write = SetFgToColor };
  __property int FgFromColor = { read = fgFromColor, write = SetFgFromColor };
  __property int BgToColor = { read = bgToColor, write = SetBgToColor };
  __property int BgFromColor = { read = bgFromColor, write = SetBgFromColor };
  __property bool FgChecked = { read = fgChecked, write = fgChecked };
  __property bool BgChecked = { read = bgChecked, write = bgChecked };
  __property bool HideMaxColors = { read = FHideMaxColors, write = FHideMaxColors };
  __property int MaxColors = { read = maxColors, write = maxColors };
  __property String Title = { read = FTitle, write = FTitle };
};
//---------------------------------------------------------------------------
extern PACKAGE TMorphForm *MorphForm;
//---------------------------------------------------------------------------
#endif
