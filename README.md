cube2omx
========

A simple matrix converter for Citilabs Cube matrices to/from OMX.

OMX Website:
------------
https://sites.google.com/site/openmodeldata/


Notes
-----

Cube2OMX.exe is a command-line Windows executable that converts any Citilabs Cube matrix to OMX, and vice-versa.   You can use this to convert existing Cube skims, trip tables, etc., to OMX format.

Cube matrices will be written out to the OMX file in the same order, stored by name, and an extra attribute "CUBE_MAT_NUMBER" will be attached to each table. OMX files created by this utility can be converted back to Cube, and the matrix order will be retained.

To convert from OMX to Cube, every table in the OMX file must have a CUBE_MAT_NUMBER attribute associated with it, from 1 to the number of tables. The converter will fail with an error message if things are not set up correctly.


REQUIREMENTS

* Windows (XP or higher)
* Requires a valid Citilabs Cube license
* Requires Citilabs TPPDLIBX.DLL file to be in the system PATH

USAGE

cube2omx.exe  [filename1] [filename2] ...
* File type will be autodetected; OMX files will be converted to Cube, and vice-versa.
* OMX files will be named filename.omx
* Cube files will be named filename.mat

