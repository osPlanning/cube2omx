cube2omx
========

A simple matrix converter for Citilabs Cube matrices to/from OMX.

OMX Website:
------------
https://sites.google.com/site/openmodeldata/

Cube2OMX.exe is a command-line Windows executable that converts any Citilabs Cube matrix to OMX, and vice-versa.   You can use this to convert existing Cube skims, trip tables, etc., to OMX format.

Cube matrices will be written out to the OMX file in the same order as they are found; however, OMX does not have the notion of a matrix "number".  OMX matrices must be referenced using the table names that are carried over from Cube. OMX files created by this utility can be converted back to Cube, and the matrix order will be retained.

OMX files created using any of the language APIs do not have a notion of a matrix number; they only have names. Therefore it is critical that any Cube scripts which reference files created using this utility must reference tables by name, not by number.  For example, use MI.1.IVT, not MI.1.1.


REQUIREMENTS

* Windows (XP or higher)
* Requires a valid Citilabs Cube license
* Requires Citilabs TPPDLIBX.DLL file to be in the system PATH

USAGE

cube2omx.exe  [filename1] [filename2] ...
* Files ending in .omx or .OMX will be converted from OMX to Cube
* Files ending in anything else will be converted from Cube to OMX

