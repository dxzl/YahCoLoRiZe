//---------------------------------------------------------------------------
#ifndef DlgWebExportH
#define DlgWebExportH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>

#define WD_WIDTH          185
#define WD_HEIGHT         21

#define WD_LEFT_IMAGEURL  8
#define WD_TOP_IMAGEURL   96

#define WD_LEFT_TITLE     208
#define WD_TOP_TITLE      96

#define WD_LEFT_AUTHOR    208
#define WD_TOP_AUTHOR     171

#define WD_LEFT_HOMELINK  208
#define WD_TOP_HOMELINK   246
//---------------------------------------------------------------------------
class TWebExportForm : public TForm
{
__published:  // IDE-managed Components
  TTrackBar *LeftMarginSlider;
  TTrackBar *TopMarginSlider;
  TLabel *LeftMarginLabel;
  TLabel *TopMarginLabel;
  TLabel *LeftLabel;
  TCheckBox *BgImageCheckBox;
  TButton *OkButton;
  TCheckBox *ClipCheckBox;
  TLabel *AuthorLabel;
  TLabel *TopLabel;
  TLabel *HomeUrlLabel;
  TLabel *BgImageLabel;
  TCheckBox *HomeCheckBox;
  TCheckBox *AuthorCheckBox;
  TCheckBox *TitleCheckBox;
  TCheckBox *BgFixedCheckBox;
  TRadioGroup *BgImageRadioGroup;
  TButton *CancelButton;
  TLabel *TitleLabel;
  TEdit *LineHeightEdit;
  TUpDown *LineHeightUpDown;
  TLabel *LineHeightLabel;
  TLabel *ColorPanelLabel;
  TPanel *ColorPanel;
  TRadioGroup *WhiteSpaceRadioGroup;
  TCheckBox *NBSPCheckBox;
  void __fastcall FormShow(TObject *Sender);
  void __fastcall TopMarginSliderChange(TObject *Sender);
  void __fastcall LeftMarginSliderChange(TObject *Sender);
  void __fastcall BgImageCheckBoxClick(TObject *Sender);

  void __fastcall SetControls( bool bSetSaveToClipboardMode );
  void __fastcall ClipCheckBoxClick(TObject *Sender);
  void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
  void __fastcall ColorPanelMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall LineHeightEditChange(TObject *Sender);
  void __fastcall FormCreate(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall AuthorCheckBoxClick(TObject *Sender);
  void __fastcall HomeCheckBoxClick(TObject *Sender);
  void __fastcall FormDestroy(TObject *Sender);
  void __fastcall TitleCheckBoxClick(TObject *Sender);
private:  // User declarations
  void __fastcall ComputeLineHeightLabel(int Val);
  void __fastcall SetColor(void);
  WideString __fastcall EditBoxW_GetString(HWND hwndCtl);

  HWND m_hImageUrlEdit, m_hTitleEdit, m_hAuthorEdit, m_hHomeLinkEdit;
  HFONT m_hFont;


  int lineHeight, bgColor;
  int whiteSpaceStyle, bgImageStyle;
  int topMargin, leftMargin;
  String bgImageUrl, homeButtonUrl;
  String webPageTitle, webPageAuthor;
  bool BNonBreakingSpaces, BBgImage, BBgFixed, BAuthor, BTitle, BHome, BClip;

protected:
  void __fastcall SetBgColor(int val);
  void __fastcall SetLineHeight(int val);
  void __fastcall SetWhiteSpaceStyle(int val);
  void __fastcall SetBgImageStyle(int val);

public:    // User declarations
  __property int LineHeight = {read = lineHeight, write = SetLineHeight};
  __property int BgColor = {read = bgColor, write = SetBgColor};
  __property int LeftMargin = {read = leftMargin, write = leftMargin};
  __property int TopMargin = {read = topMargin, write = topMargin};
  __property int WhiteSpaceStyle = {read = whiteSpaceStyle, write = SetWhiteSpaceStyle};
  __property int BgImageStyle = {read = bgImageStyle, write = SetBgImageStyle};
  __property String BgImageUrl = {read = bgImageUrl, write = bgImageUrl};
  __property String HomeButtonUrl = {read = homeButtonUrl, write = homeButtonUrl};
  __property String WebPageTitle = {read = webPageTitle, write = webPageTitle};
  __property String WebPageAuthor = {read = webPageAuthor, write = webPageAuthor};
  __property bool bNonBreakingSpaces = {read = BNonBreakingSpaces, write = BNonBreakingSpaces};
  __property bool bBgImage = {read = BBgImage, write = BBgImage};
  __property bool bBgFixed = {read = BBgFixed, write = BBgFixed};
  __property bool bAuthor = {read = BAuthor, write = BAuthor};
  __property bool bTitle = {read = BTitle, write = BTitle};
  __property bool bHome = {read = BHome, write = BHome};
  __property bool bClip = {read = BClip, write = BClip};
};
//---------------------------------------------------------------------------
extern PACKAGE TWebExportForm *WebExportForm;
//---------------------------------------------------------------------------
#endif
