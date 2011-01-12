
This package contains the files needed to use YASM with Microsoft Visual
Studio 2010 (VC++):

  1. VSYASM - version of YASM designed for use with Visual Studio 2010
  2. vsyasm.xml, vsyasm.props and vsyasm.targets - the files needed to
     integrate VSYASM into Microsoft Visual Studio 2010 (VC++)

To use these files you need to do the following:

  1. Put the VSYASM executable file (vsyasm.exe) in the Visual Studio
     binary directory, which is typically at:

          C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin

     Alternatively, if you wish to put vsyasm.exe elsewhere, the
     environment YASMPATH can be set to the directory path (including
     the final backslash) where vsyasm.exe is placed.

  2. place the three files vsyasm.xml, vsyasm.props and vsyasm.targets
     in the directory alongside the other files with which VSYASM will
     be used.

  3. After a project that includes YASM assembler files is opened in
     the Visual Studio 2010 IDE, right click on the project in the
     solution explorer and select "Build Customisations". If vsyasm is
     offered as an option in the resulting list you can then select it;
     if not, use the "Find Existing" button and the resulting file
     dialogue to navigate to the vsyasm.targets that you stored earlier,
     select it to add it to the list and then select it from the list.

  4. Once you have done this, right clicking on the project in the
     solution explorer and selecting "Properties" will bring up a
     dialogue with a new item "Yasm Assembler" that will allow you
     to configure Yasm for building any assembler files added to
     the project.

If you have many projects that use VSYASM, you can put the three files
mentioned above into MSBUILD's build customisation directory which is
typically at:

C:\Program Files (x86)\MSBuild\Microsoft.Cpp\v4.0\BuildCustomizations

VSYASM will then always be available in the Build Customisations
dialogue. An alternative way of doing this is to put these files in a
convenient location and then add the path to this location to the
"Build Customisations Search Path" item under "VC++ Project Settings"
in the Visual Studio 2010 Options dialogue.

Using VSYASM

In a Visual Studio project with assembler source code files, Yasm
settings are entered in the "Yasm Assembler" item in the projects
Property Dialogue. The items available correspond with those
available on Yasm's command line and are mostly self explanatory
but one item -- "Object Filename" -- does need further explanation.

If the "Object Filename" item refers to a directory (the default),
The Visual Studio IDE will collect all the assembler files in the
project together as a batch and invoke VSYASM in multiple file mode.

In order to assemble files one at a time it is necessary to change
the "Object Filename" item to givw the name of an output file such
as, for example, "$(IntDir)%(Filename).obj".
