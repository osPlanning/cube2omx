cube2omx
========

A simple matrix converter for Citilabs Cube matrices to/from OMX.

OMX Website:
------------
https://github.com/osPlanning/omx


Notes
-----

Cube2OMX.exe is a command-line Windows executable that converts any Citilabs Cube matrix to OMX, and vice-versa.   You can use this to convert existing Cube skims, trip tables, etc., to OMX format.

Cube matrices will be written out to the OMX file in the same order, stored by name, and an extra attribute "CUBE_MAT_NUMBER" will be attached to each table. OMX files created by this utility can be converted back to Cube, and the matrix order will be retained.

To convert from OMX to Cube, every table in the OMX file must have a CUBE_MAT_NUMBER attribute associated with it, from 1 to the number of tables. The converter will fail with an error message if things are not set up correctly.


REQUIREMENTS

* Windows (XP or higher)
* Requires a valid Citilabs Cube license
* Requires Citilabs TPPDLIBX.DLL file to be in the system PATH (i.e. C:\Program Files (x86)\Citilabs\CubeVoyager)

USAGE

`cube2omx.exe  [filename1] [filename2] ...`
* File type will be autodetected; OMX files will be converted to Cube, and vice-versa.
* OMX files will be named filename.omx
* Cube files will be named filename.mat

TROUBLESHOOTING
* If it cannot find TPPLIBX.DLL, then make sure your path is correct by trying to run cube voyager from the command line `> voyager.exe <some script name>.s`

BUILDING FROM SOURCE
--------------------

Built using MinGW (GCC) toolchain:
http://www.mingw.org/

Additional required libraries:
* HDF5, HDF5_HL, and SZIP: http://www.hdfgroup.org/HDF5/
* zlib: http://www.zlib.net/

Once your toolchain is set up you can run make using the supplied
Makefile, or run the following all-in-one command:

`g++ -static-libgcc *.cpp -lhdf5_hl -lhdf5 -lsz -lz -o cube2omx.exe`


