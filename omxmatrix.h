/* omxmatrix.h
 *
 * HDF5/OMX Matrix helper routines
 *
 * @author Billy Charlton, PSRC
 */
#define BOOST_USE_WINDOWS_H 1
#include <boost/thread.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <queue>

#include <hdf5.h>
#include <hdf5_hl.h>

using namespace std;

#define  MODE_READ    0
#define  MODE_CREATE  1

#define  MAX_TABLES  500

//--------------------------------------------------------------------
#ifndef OMXMATRIX_H
#define OMXMATRIX_H

/* OMXMatrix is a drop-in replacement for TPPMatrix.
 * 100% TP+ API Compatible:  Should require no source changes other than replacing
 * all references to TPPMatrix with H5Matrix instead.
 */
class OMXMatrix {
public:
    OMXMatrix();

    virtual  ~OMXMatrix();

    void     openFile(char *fileName);
    void     closeFile();

    //Read/Open operations
    int      getZones();
    int      getTables();
    void     getRow (int table, int row, void *rowptr);  // throws InvalidOperationException, MatrixReadException
    double   getValue(int table, int row, int j);
    char*    getTableName(int table);
//    T2DDoubleArray readEntireTable (int table);  //Not implemented

    //Write/Create operations
    void     createFile(int tables, int zones, char** matName, char* fileName);
    void     writeRow(int table, int row, double* rowptr);

    //Helper methods
    double*  allocateRowBuffer();

    //Nested exception classes
    class    FileOpenException { };
    class    MatrixReadException { };
    class    InvalidOperationException { };
    class    OutOfMemoryException {};
    class    NoSuchTableException {};

//--------------------------------------------------------------------
private:
    //Data

    hid_t    _h5file;
    int      _nZones;
    int      _nTables;
    int      _mode;
    bool     _fileOpen;

    char*    _tableNumber[MAX_TABLES+1];
    char*    _tableName[MAX_TABLES+1];
    hid_t    _dataset[MAX_TABLES+1];
    hid_t    _dataspace[MAX_TABLES+1];
    hid_t    _memspace;

    //Methods
    void    readTableNames();
    void    printErrorCode(int error);
    void    init_tables (char** tableNames);
    hid_t   openDataset(int table);  // throws InvalidOperationException
};

#endif /* OMXMATRIX_H */

