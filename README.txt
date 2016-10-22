The installer used is NSIS (nullsoft scriptable install system):

https://sourceforge.net/projects/nsis/

There is a seperate installer for the English/French/Spanish versions, all named YahCoLoRiZe.nsi

I also use the UAC script and put the UAC.dll files in C:\Program Files (x86)\NSIS\Plugins\x86-ansi (and x86-unicode)

(Using UAC got around the problem that after install, the first-run of your program would not
be at the proper user privlege level in Windows 10. They may have fixed the issue in the latest
version, 3.0???)

After installing NSIS in Windows, you right-click on an nsi file and choose "Compile NSIS Script" to build.

You should set-up the project directory-structure to look like this: (the various files are in misc-zips)

YahCoLoRiZe
    A_English
        Package -> Colorize.exe, License.rtf, License.txt
        DefaultStrings.cpp
        YahCoLoRiZe.nsi
        Colorize.ico
        FileAssociation.nsh
        UAC.nsh
    A_French
        Package -> Colorize.exe, License.rtf, License.txt
        DefaultStrings.cpp
        YahCoLoRiZe.nsi
        Colorize.ico
        FileAssociation.nsh
        UAC.nsh
    A_Spanish
        Package -> Colorize.exe, License.rtf, License.txt
        DefaultStrings.cpp
        YahCoLoRiZe.nsi
        Colorize.ico
        FileAssociation.nsh
        UAC.nsh
    ComponentDemo -> asciiart.txt, manual.txt, tech.txt
    ComponentDLLs -> Colorize.chm, Colorize.dll, Colorize.ini, ColorizeMan.rtf,Hunspell.dll, smileys.txt, YahCoLoRiZe.txt
    ComponentEnglishDict -> en-US.aff, en_US.dic
    ComponentFrenchDict -> fr_FR.aff, fr_FR.dic
    ComponentSmileys -> (a bunch of .jpg smiley-image files)
    ComponentScripts -> colorize.ops, Colorize.tcl, Readme.txt
    ComponentSpanishDict -> es_ES.aff, es_ES.dic
    (source-code files)

To build the project for a particular language, copy DefaultStrings.cpp for the language into the main source-code (root) directory.

The project file for C++-Builder is Colorize.bpr. After building, Colorize.exe will be generated in the root directory.

Copy Colorize.exe into the appropriate Package sub-folder for the chosen langusge.

Then I usually use a batch-file to sign/timestamp the .exe, hand-edit the .nsi file to bump the revision up, then right-click the nsi script-file and chose "Compile NSIS Script", then I sign and timestamp the installer.

Note: the installer does a nifty two-loop trick to automatically sign the Uninstaller.exe file, so you will need to make sure to entery the password for your signing-key for KEY_PLAZZ. Set KEY_PATH to the full path of your .pfx signing-key file.

- Scott Swift (2016)