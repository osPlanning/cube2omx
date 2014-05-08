/* tppmatrix.cpp
 *
 * TPPMatrix class implementation.
 *
 * @author  Tim Heier
 * @version 1.0, 3/02/00
 *
 */

#include "tppmatrix.h"
#include <time.h>
#include <limits.h>

using namespace std;

// declaring global function pointers
pFunc_FileInquire       pf_FileInquire;
pFunc_TppMatOpenIP      pf_TppMatOpenIP;
pFunc_TppMatOpenOP      pf_TppMatOpenOP;
pFunc_TppMatClose       pf_TppMatClose;
pFunc_TppMatPos         pf_TppMatPos;
pFunc_TppMatGetPos      pf_TppMatGetPos;
pFunc_TppMatSet         pf_TppMatSet;
pFunc_TppMatMatResize   pf_TppMatMatResize;
pFunc_TppMatReadNext    pf_TppMatReadNext;
pFunc_TppMatReadDirect  pf_TppMatReadDirect;
pFunc_TppMatReadSelect  pf_TppMatReadSelect;
pFunc_TppMatWriteRow    pf_TppMatWriteRow;

static bool loadedDll=false;

/**
 * Method to load the dll and set the function call addresses based on TPP/Cube version.
 *
 */
void tppInitDllNative ()
{
	if(loadedDll)
		return;

    // Link DLL
	HMODULE hMod = LoadLibrary("tppdlibx.dll");

	if (hMod==NULL) {
		fprintf(stderr, "\n\n## TPPDLIBX.DLL not found.  Check your PATH and license.\n");
		exit(2);
	}
	// assign function pointers

	if(GetProcAddress(hMod,"_FileInquire")!=NULL){ //pre-cube 4

		pf_FileInquire       = (pFunc_FileInquire) GetProcAddress(hMod,"_FileInquire");
		pf_TppMatOpenIP      = (pFunc_TppMatOpenIP) GetProcAddress(hMod,"_TppMatOpenIP");
		pf_TppMatOpenOP      = (pFunc_TppMatOpenOP) GetProcAddress(hMod,"_TppMatOpenOP");
		pf_TppMatClose       = (pFunc_TppMatClose) GetProcAddress(hMod,"_TppMatClose");
		pf_TppMatPos         = (pFunc_TppMatPos) GetProcAddress(hMod,"_TppMatPos");
		pf_TppMatGetPos      = (pFunc_TppMatGetPos) GetProcAddress(hMod,"_TppMatGetPos");
		pf_TppMatSet         = (pFunc_TppMatSet) GetProcAddress(hMod,"_TppMatSet");
		pf_TppMatMatResize   = (pFunc_TppMatMatResize) (void*)GetProcAddress(hMod,"_TppMatResize");
		pf_TppMatReadNext    = (pFunc_TppMatReadNext) GetProcAddress(hMod,"_TppMatReadNext");
		pf_TppMatReadDirect  = (pFunc_TppMatReadDirect) GetProcAddress(hMod,"_TppMatReadDirect");
		pf_TppMatReadSelect  = (pFunc_TppMatReadSelect) GetProcAddress(hMod,"_TppMatReadSelect");
		pf_TppMatWriteRow    = (pFunc_TppMatWriteRow) GetProcAddress(hMod,"_TppMatWriteRow");
	}else{ //cube 4 or later
		pf_FileInquire       = (pFunc_FileInquire) GetProcAddress(hMod,"FileInquire");
		pf_TppMatOpenIP      = (pFunc_TppMatOpenIP) GetProcAddress(hMod,"TppMatOpenIP");
		pf_TppMatOpenOP      = (pFunc_TppMatOpenOP) GetProcAddress(hMod,"TppMatOpenOP");
		pf_TppMatClose       = (pFunc_TppMatClose) GetProcAddress(hMod,"TppMatClose");
		pf_TppMatPos         = (pFunc_TppMatPos)   GetProcAddress(hMod,"TppMatPos");
		pf_TppMatGetPos      = (pFunc_TppMatGetPos)GetProcAddress(hMod,"TppMatGetPos");
		pf_TppMatSet         = (pFunc_TppMatSet) GetProcAddress(hMod,"TppMatSet");
		pf_TppMatMatResize   = (pFunc_TppMatMatResize) (void*)GetProcAddress(hMod,"TppMatResize");
		pf_TppMatReadNext    = (pFunc_TppMatReadNext) GetProcAddress(hMod,"TppMatReadNext");
		pf_TppMatReadDirect  = (pFunc_TppMatReadDirect) GetProcAddress(hMod,"TppMatReadDirect");
		pf_TppMatReadSelect  = (pFunc_TppMatReadSelect) GetProcAddress(hMod,"TppMatReadSelect");
		pf_TppMatWriteRow    = (pFunc_TppMatWriteRow) GetProcAddress(hMod,"TppMatWriteRow");
	}

	loadedDll=true;
}


//--------------------------------------------------------------------

//Constructor - default
TPPMatrix::TPPMatrix()
{
	tppInitDllNative ();

	_fileOpen = false;
	_nTables = 0;
	_nZones = 0;
	_mode = 0;

    for (int i=0; i < MAX_TABLES; i++)
        _rowPos[i] = NULL;
}

//Destructor
TPPMatrix::~TPPMatrix()
{
    _fileOpen = false;

    free(_rowptr);
    free(_matlist->buffer);
}


//--------------------------------------------------------------------

void TPPMatrix::openFile(char *fileName)
{

	int i=0;
    int table, origin;
    char *pLicenseFile=NULL;

 	i=pf_FileInquire(fileName, &_matlist);
 	if (i<0) {
        cout << "**TPPMatrix: File not found / could not open: " << fileName << endl;
        exit(2);
 	}

	// We're looping for MAX_DLL_ATTEMPTS because Citilabs DLL can
 	// timeout due to retardation and bad design
	for (int attempts = 0; attempts<MAX_DLL_ATTEMPTS; attempts++) {
		// call the dll
		if ((i = pf_TppMatOpenIP(_matlist, pLicenseFile, 2)) <= 0) {
			// If this is a just a license timeout issue, take a nap and try again
			if (i == -33) {
				cout <<"TP+ -33 DLL Timeout: Retrying " << fileName << endl;
				Sleep(2000);
				continue;
			} else {
				printErrorCode(i);
				exit(i);
			}
		}
		// Success!  exit attempt loop.
		break;
	}

    //Position to beginning
    if (pf_TppMatPos(_matlist, 0)==0) {
        cout << "**TPPMatrix: Could not position file, " << fileName << endl;
        throw FileOpenException();
    }

   _fileOpen = true;

    //Set class attributes
    _nTables = _matlist->mats;
    _nZones  = _matlist->zones;

    //Used by class methods
    _rowptr = (double *) malloc( ((_nZones+3)*sizeof(double)) );

    //Work space used by TppMatXXX functions
   _matlist->buffer = malloc (_matlist->bufReq);


    readTableNames();

    //Store row locations
    while ( pf_TppMatReadNext(1, _matlist, _rowptr)!=0 ) {
        table  = _matlist->rowMat;
        origin = _matlist->rowOrg;

        if (table > MAX_TABLES) {
            cout << "**TPPMatrix: More than 500 tables in matrix" << endl;
            throw MatrixReadException();
        }

        if (_rowPos[table] == NULL) {
            _rowPos[table] = (DWORD *) malloc ((_matlist->zones+3) * sizeof(DWORD));
        }

		if(origin>_matlist->zones || origin<=0){
            cout << "**TPPMatrix: Read zone "<<origin<<" which is greater than "<<_matlist->zones<<" in matrix" << endl;
            throw MatrixReadException();
		}

		if(_matlist->rowpos<0 || _matlist->rowpos>ULONG_MAX){
			 cout<<"Error, current position is "<<_matlist->rowpos<<"\n";
		     throw MatrixReadException();
		}

        _rowPos[table][origin] = _matlist->rowpos;
        pf_TppMatReadNext(-2, _matlist, _rowptr);
    }
}


//--------------------------------------------------------------------
void TPPMatrix::readTableNames()
{

    //Store table names
    char tempbuf[255];
    char* c = (char *) _matlist->Mnames;

    for (int i=1; i <= _matlist->mats; i++) {

        sprintf (tempbuf, "%s", c);
        _tableName[i] = (char *) malloc( (strlen(tempbuf)+1) *sizeof(char));

        strcpy (_tableName[i], tempbuf);
        c = c + strlen(c) + 1;
    }
}


//--------------------------------------------------------------------
//
char* TPPMatrix::getTableName(int table)
{

    char* c = "";
	if (table >= MAX_TABLES || table < 0)
        c = "";
    else
        c = _tableName[table];
    return c;
}


//--------------------------------------------------------------------
double* TPPMatrix::allocateRowBuffer()
{
    return ( (double *) malloc ((getZones()+3) * sizeof(double)) );
}


//--------------------------------------------------------------------
int TPPMatrix::getTables(void)
{
    return _matlist->mats;
}


//--------------------------------------------------------------------
int TPPMatrix::getZones(void)
{
    return _nZones;
}


//--------------------------------------------------------------------
//Row values are stored in rowptr[1]
//

void TPPMatrix::getRow(int table, int row, double *rowptr)
{

    if (_rowPos[table] == NULL) {
        cout << "**TPPMatrix: Table=" << table << " not indexed" << endl;
        throw MatrixReadException();
    }

    if (! pf_TppMatReadDirect (_matlist, _rowPos[table][row], rowptr) ) {
        cout << "**TPPMatrix: Could not read table=" << table << " row=" << row << endl;
        throw MatrixReadException();
    }
}


//--------------------------------------------------------------------
double TPPMatrix::getValue(int table, int row, int j)
{
    if (j < 0 || j > _nZones) {
        cout << "**TPPMatrix: Could not read table=" << table <<
                " row=" << row << " j=" << j << endl;
        throw MatrixReadException();
    }

    if (!pf_TppMatPos(_matlist, 0)) {
        cout << "**TPPMatrix: Could not postion file" << endl;
        throw MatrixReadException();
    }

    if (! pf_TppMatReadSelect (_matlist, row, table, &_rowptr[1]) ) {
        cout << "**TPPMatrix: Could not read table=" << table <<
                " row=" << row << endl;
        throw MatrixReadException();
    }

    return _rowptr[j];
}


//--------------------------------------------------------------------
void TPPMatrix::createFile(int tables, int zones, const char** matName,
                            const char* fileName)
{
    time_t ttime = time(NULL);
    char *pLicenseFile=NULL;

    if (_fileOpen == true)
        throw InvalidOperationException();

    _mode = CREATE_FILE;
    _nTables = tables;
    _nZones  = zones;

	_matlist = (MATLIST *) malloc ( sizeof (MATLIST) );

	int returnValue = pf_TppMatSet(&_matlist, TPP, fileName, zones, tables);

	if(returnValue==0){
		cout<<"Error attempting to set matrix in "<<fileName<<"\n";
		throw InvalidOperationException();
	}

	/* - Allocate a buffer for the file records
       - Set the precision for storing at 2 decimal places
         optional Mspecs is 'D' and 'S' for full precision if necessary
       - Establish a name for each matrix - not absolutely necessary
    */

    _matlist->buffer = malloc(_matlist->bufReq);
    for (int i=0; i<tables;i++) _matlist->Mspecs[i] = 'D';


    for (char* b=(char*)_matlist->Mnames, i=1; i<=tables;i++)
    	b += 1 + sprintf(b,"%s",matName[i-1]);

	/* Open the Op file */
	int returnCode=0;

	// We're looping for MAX_DLL_ATTEMPTS because Citilabs DLL can
 	// timeout due to retardation and bad design
	for (int attempts = 0; attempts<MAX_DLL_ATTEMPTS; attempts++) {
		// call the dll
		if ((returnCode = pf_TppMatOpenOP (_matlist, "File ID", "tppOpen()", &ttime, pLicenseFile, 2)) <= 0) {
			// If this is a just a license timeout issue, take a nap and try again
			if (returnCode == -33) {
				cout <<"TP+ -33 DLL Timeout: Retrying " << fileName << endl;
				Sleep(2000);
				continue;
			} else {
				printErrorCode(returnCode);
				exit(returnCode);
			}
		}
		// Success!  exit attempt loop.
		break;
	}

    pf_TppMatMatResize(&_matlist);
}


//--------------------------------------------------------------------
/*
 * This method should be called in a loop of zones, tables such as:
 *
 *    for (zones)
 *       for (tables)
 *          m->writeRow()
 *
 */
void TPPMatrix::writeRow(int table, int row, double *rowptr)
{
    // That '2' means two decimal places.  Why not three, or twenty?
    pf_TppMatWriteRow (_matlist, row, table, 'D', rowptr);
}


//--------------------------------------------------------------------
void TPPMatrix::closeFile()
{
    if (_mode == CREATE_FILE)
        pf_TppMatClose(_matlist);

    _fileOpen = false;
}


void TPPMatrix::printErrorCode(int error){

		switch(error){
			case -1:
				printf("TPP Error: %i - Not recognized",error);
            	break;
            case -11:
               printf("TPP Error: %i - License file not found",error);
            	break;
            case -12:
               printf("TPP Error: %i - Invalid License file",error);
            	break;
            case -13:
               printf("TPP Error: %i - Registry Entry not found",error);
            	break;
            case -14:
               printf("TPP Error: %i - License expired - tppdlibx.dll",error);
            	break;
			case -15:
			   printf("TPP Error: %i - Dongle driver Missing",error);
            	break;
            case -16:
               printf("TPP Error: %i - Dongle not found",error);
            	break;
            case -17:
               printf("TPP Error: %i - Dongle Test failed Authentication",error);
            	break;
            case -18:
               printf("TPP Error: %i - License Mismatch Lookingfor=maxzones, fnd=maxnodes",error);
            	break;
            case -20:
               printf("TPP Error: %i - Maximum zone numbers allowed",error);
            case -33:
               printf("TPP Error: %i - License file problem",error);
			break;
			default:
				printf("TPP Error: %i - not recognized",error);
				break;
		}
}
