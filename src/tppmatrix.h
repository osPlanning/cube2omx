/* tppmatrix.h
 *
 * TPPMatrix class defination.
 *
 * @author  Tim Heier
 * @version 1.0, 2/05/00
 *
 * @author Joel Freedman
 * @verson 2.0, 5/03/07
 */

#include "cubeio.h"

#include <iostream>
#include <string>
//#include <dir>

using namespace std;

#define  CREATE_FILE  1
#define  MAX_DLL_ATTEMPTS 5

#define  MAX_TABLES  500
#define  HCHAR  char
#define  BYTE   unsigned char
#define  UCHAR  unsigned char
#define  USHORT unsigned short
#define  ULONG  unsigned long
#define  SHORT  short
#define  LONG   long
#define  INT    int
#define  UINT   unsigned int
#define  WORD   unsigned short
#define  DWORD  unsigned long

#define  SL     sizeof(LONG)
#define  SS     sizeof(SHORT)
#define  SD     sizeof(double)
#define  SI     sizeof(INT)


//--------------------------------------------------------------------
//TP+ Matrix Class Definition

class TPPMatrix
{

//--------------------------------------------------------------------
public:
    TPPMatrix();
    virtual  ~TPPMatrix();

    //Existing file operations
    void     openFile(char *fileName);
    int      getZones();
    int      getTables();
    void     getRow (int table, int row, double *rowptr);
    double   getValue(int table, int row, int j);
    char*    getTableName(int table);

    //New file operations
    void     createFile(int tables, int zones, const char** matName,
                        const char* fileName);
    void     writeRow(int table, int row, double *rowptr);
    void     closeFile();

    //Helper methods
    double*  allocateRowBuffer();

    //Nested exception classes
    class    FileOpenException { };
    class    MatrixReadException { };
    class    InvalidOperationException { };

//--------------------------------------------------------------------
private:
    //Data

	MATLIST* _matlist ;
    int      _nZones;
    int      _nTables;
    int      _mode;
    bool     _fileOpen;
    double*  _rowptr;
    char*    _tableName[MAX_TABLES+1];
    DWORD*   _rowPos[MAX_TABLES+1];

    //Methods
    void readTableNames();
    void printErrorCode(int error);
};
