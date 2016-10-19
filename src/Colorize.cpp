//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USERES("Colorize.res");
USEFORM("Main.cpp", DTSColor);
USEFORM("DlgWingEdit.cpp", WingEditForm);
USEFORM("DlgMonkey.cpp", MonkeyForm);
USEFORM("DlgPlay.cpp", PlayForm);
USEFORM("DlgSmiley.cpp", SmileyForm);
USEFORM("DlgColor.cpp", ColorForm);
USEFORM("DlgBlend.cpp", BlendChoiceForm);
USEFORM("DlgWebExport.cpp", WebExportForm);
USEFORM("DlgFontType.cpp", FontTypeForm);
USEFORM("DlgAbout.cpp", AboutForm);
USEFORM("DlgLicense.cpp", LicenseForm);
USEFORM("DlgStrip.cpp", StripModeForm);
USEFORM("DlgIncDec.cpp", IncDecForm);
USEFORM("DlgAlternate.cpp", AlternateForm);
USEFORM("DlgSetColors.cpp", SetColorsForm);
USEFORM("DlgMorph.cpp", MorphForm);
USEFORM("DlgReplaceText.cpp", ReplaceTextForm);
USEFORM("DlgSpellCheck.cpp", SpellCheckForm);
USEFORM("DlgShowDict.cpp", ShowDictForm);
USEFORM("DlgChooseDict.cpp", ChooseDictForm);
USEFORM("DlgFontSize.cpp", FontSizeForm);
USEFORM("FormSFDlg.cpp", SFDlgForm);
USEFORM("FormOFSSDlg.cpp", OFSSDlgForm);
USEFORM("TaePreviewFrm.cpp", TaePreviewForm);
USEUNIT("ConvertToIRC.cpp");
USEUNIT("Effects.cpp");
USEUNIT("DefaultStrings.cpp");
USEUNIT("Blender.cpp");
USEUNIT("Optimizer.cpp");
USEUNIT("Playback.cpp");
USEUNIT("ThreadOnChange.cpp");
USEUNIT("ConvertToHLT.cpp");
USEUNIT("ConvertToYahoo.cpp");
USEUNIT("Pushstruct.cpp");
USEUNIT("Utils.cpp");
USEUNIT("Paltalk.cpp");
USEUNIT("PaltalkClass.cpp");
USEUNIT("Play.cpp");
USEUNIT("ConvertToRTF.cpp");
USEUNIT("PASTESTRUCT.cpp");
USEUNIT("Undo.cpp");
USEUNIT("MyCheckLst.pas");
USEUNIT("ConvertToHTML.cpp");
USEUNIT("ConvertFromHTML.cpp");
USEUNIT("StringsW.cpp");
USEFILE("ComboBoxW.h");
USERC("Colorize.rc");
USEUNIT("VerInfo.cpp");
USEUNIT("PrintPreview.cpp");
USEFILE("Resource.h");
USEFORM("DlgUnderline.cpp", UnderlineForm);
USEFORM("DlgFavColors.cpp", FavColorsForm);
USEFORM("DlgFgBgColors.cpp", FgBgColorsForm);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  try
  {
    Application->Initialize();
    Application->Name = "DTSColor"; // Name used to reference the main class in our code
    Application->Title = "YahCoLoRiZe"; // Product name
    Application->CreateForm(__classid(TDTSColor), &DTSColor);
     Application->Run();
  }
  catch (Exception &exception)
  {
    Application->ShowException(&exception);
  }
  return 0;
}
//---------------------------------------------------------------------------

