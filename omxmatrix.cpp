/* omxmatrix.cpp
 *
 * HDF5/OMX Matrix helper routines
 *
 * @author Billy Charlton, PSRC
 */

#include <cstdlib>
#include <cstring>
#include <ctime>

#include "omxmatrix.h"

using namespace std;

void printCurrentTime();

// ###########################################################################
// OMXMatrix:  C++ Helper class to read/write TP+ style matrix tables
// ---------------------------------------------------------------------------

OMXMatrix::OMXMatrix() {
    _fileOpen = false;
    _nTables = 0;
    _nZones = 0;
    _memspace = -1;

    for (int t=0; t < MAX_TABLES; t++) {
        _dataset[t] = -1;
        _dataspace[t] = -1;
        _tableName[t] = NULL;
        _tableNumber[t] = NULL;
    }
}

//Destructor
OMXMatrix::~OMXMatrix()
{
    // double check: release datasets and memory
    for (int t=0; t<MAX_TABLES; t++) {
        if (_dataset[t] != -1) {
            H5Dclose(_dataset[t]);
            _dataset[t] = -1;
        }
        if (_dataspace[t] != -1) {
            H5Sclose(_dataspace[t]);
            _dataspace[t] = -1;
        }

        if (_tableName[t] != NULL) free(_tableName[t]);
        if (_tableNumber[t] != NULL) free(_tableNumber[t]);
    }

    if (_memspace > -1 ) {
        H5Sclose(_memspace);
        _memspace = -1;
    }

    // Close H5 file handles
    if (_fileOpen==true) {
        H5Fclose(_h5file);
    }

    _fileOpen = false;
}

//Write/Create operations ---------------------------------------------------

void OMXMatrix::createFile(int tables, int zones, char** tableNames, char* fileName) {
    _fileOpen = true;
    _mode = MODE_CREATE;
    _nZones = zones;
    _nTables = tables;

    // Create the physical file
    _h5file = H5Fcreate(fileName, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (0 > _h5file) {
        fprintf(stderr, "ERROR: Could not create file %s.\n", fileName);
    }

    // Initialize file properties
    H5LTset_attribute_int(_h5file, "/", "tables", &tables, 1);
    H5LTset_attribute_int(_h5file, "/", "zones", &zones, 1);

    // Create the datasets
    init_tables(tableNames);
}

void OMXMatrix::writeRow(int table, int row, double *rowdata) {
    if ((_dataset[table] == -1) || (table > _nTables)) {
            throw NoSuchTableException();
    }

    hsize_t count[2], offset[2];

    count[0] = 1;
    count[1] = _nZones;

    offset[0] = row-1;
    offset[1] = 0;

    if (_memspace <0 ) {
        _memspace = H5Screate_simple(2,count,NULL);
    }
    if (_dataspace[table] <0 ) _dataspace[table] = H5Dget_space (_dataset[table]);

    H5Sselect_hyperslab (_dataspace[table], H5S_SELECT_SET, offset, NULL, count, NULL);

    if (0 > H5Dwrite(_dataset[table], H5T_NATIVE_DOUBLE, _memspace, _dataspace[table], H5P_DEFAULT, rowdata)) {
        fprintf(stderr, "ERROR: writing table %d, row %d\n", table,row);
        exit(2);
    }
}

//Read/Open operations ------------------------------------------------------

void OMXMatrix::openFile(char *filename) {
    // Try to open the existing file
    _h5file = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (_h5file < 0) {
        fprintf(stderr, "ERROR: Can't find or open file %s",filename);
        exit(2);
    }

    // OK, it's open and it's HDF5;
    // Now query some nice things about the file.
    _fileOpen = true;
    _mode = MODE_READ;

    herr_t status = 0;
    status += H5LTget_attribute_int(_h5file, "/", "tables", &_nTables);
    status += H5LTget_attribute_int(_h5file, "/", "zones", &_nZones);

    if (status < 0) {
        fprintf(stderr, "ERROR: %s doesn't have table/zone attributes", filename);
        exit(2);
    }

    readTableNames();
}

int OMXMatrix::getZones() {
    return _nZones;
}
int OMXMatrix::getTables() {
    return _nTables;
}

char* OMXMatrix::getTableName(int table) {
    return _tableName[table];
}

double* OMXMatrix:: allocateRowBuffer() {
    double *rowBuffer = (double *) malloc ( (getZones()+3) * sizeof(double));
    if (rowBuffer == NULL) {
        fprintf(stderr, "ERROR: Couldn't allocate memory for rowdata\n");
        exit(2);
    }

    return rowBuffer;
}

void OMXMatrix::getRow (int table, int row, void *rowptr) {
    hsize_t data_count[2], data_offset[2];

    // First see if we've opened this table already
    if (_dataset[table] == -1) {
        // Does this table exist?
        if (table > _nTables) {
            throw MatrixReadException() ;
        }
        _dataset[table] = openDataset(table);
    }

    data_count[0] = 1;
    data_count[1] = _nZones;  // Need data to be nZones long
    data_offset[0] = row-1;
    data_offset[1] = 0;

    // Create dataspace if necessary.  Don't do every time or we'll run OOM.
    if (_dataspace[table] == -1) {
        _dataspace[table] = H5Dget_space (_dataset[table]);
    }

    // Define MEMORY slab   (using data_count since we don't want to read zones+1 values!)
    if (_memspace < 0) {
        _memspace = H5Screate_simple(2, data_count, NULL);
    }

    // Define DATA slab
    if (0 > H5Sselect_hyperslab (_dataspace[table], H5S_SELECT_SET, data_offset, NULL, data_count, NULL)) {
        fprintf(stderr, "ERROR: Couldn't select DATA subregion for table %d, subrow %d.\n",table,row);
        exit(2);
    }

    // Read the data!
    if (0 > H5Dread(_dataset[table], H5T_NATIVE_DOUBLE, _memspace, _dataspace[table],
            H5P_DEFAULT, rowptr)) {
        fprintf(stderr, "ERROR: Couldn't read table %d, subrow %d.\n",table,row);
        exit(2);
    }
}

void OMXMatrix::closeFile() {
    for (int t=0; t<MAX_TABLES; t++) {
        if (_dataset[t] != -1) {
            H5Dclose(_dataset[t]);
            _dataset[t] = -1;
        }
        if (_dataspace[t] != -1) {
            H5Sclose(_dataspace[t]);
            _dataspace[t] = -1;
        }
    }

    if (_memspace > -1 ) {
        H5Sclose(_memspace);
        _memspace = -1;
    }

    if (_fileOpen==true) {
        H5Fclose(_h5file);
    }
    _fileOpen = false;
}

// ---- Private functions ---------------------------------------------------

void printCurrentTime() {
    time_t timeStart = time(NULL);
    struct tm *timeinfo;
    timeinfo = localtime(&timeStart);
    printf("Time: %s",asctime(timeinfo));
    return;
}

hid_t OMXMatrix::openDataset(int table) {
    char tname[10];

    sprintf (tname, "%d", table);

    hid_t data = H5Dopen(_h5file, tname, H5P_DEFAULT);
    if (data < 0) {
        throw InvalidOperationException();
    }

    return data;
}

void OMXMatrix::readTableNames() {
    char buf[255];
    char tname[10];

    for (int i=1; i <= _nTables; i++) {
        // Convert table number to text, And pull the table name attribute
        sprintf (tname, "%d", i);
        _tableNumber[i] = (char*) malloc(20*sizeof(char));

        strcpy(_tableNumber[i], "/");
        strcat(_tableNumber[i], tname);

        // clear out buffer
        for (int k=0;k<255; k++) buf[k] = '\0';

        herr_t status = H5LTget_attribute_string(_h5file, tname, "name", buf);

        if (status >= 0) { // No biggie if we don't get a table name
            _tableName[i] = (char *) calloc(255, sizeof(char));
            strcpy (_tableName[i], buf);
        }
    }
}

void OMXMatrix::init_tables (char** tableNames) {
    hsize_t     dims[2]={_nZones,_nZones};
    hid_t       plist;
    herr_t      rtn;
    char        tname[10];
    hsize_t     chunksize[2];
    double      fillvalue[1];

    fillvalue[0] = 0.0;
    chunksize[0] = 1;
    chunksize[1] = _nZones;

    hid_t   dataspace = H5Screate_simple(2,dims, NULL);

    // Use a row-chunked, zip-compressed data format:
    plist = H5Pcreate(H5P_DATASET_CREATE);
    rtn = H5Pset_chunk(plist, 2, chunksize);
    rtn = H5Pset_deflate(plist, 7);
    rtn = H5Pset_fill_value(plist, H5T_NATIVE_DOUBLE, &fillvalue);

    // Loop on all TP+ tables
    for (int t=1;t<=_nTables;t++) {

        // Create a dataset for each table
        sprintf (tname, "%d", t);
        _dataset[t] = H5Dcreate2(_h5file, tname, H5T_NATIVE_DOUBLE,dataspace,
                               H5P_DEFAULT,plist,H5P_DEFAULT);

        H5LTset_attribute_int(_h5file, tname, "zones", &_nZones, 1);
        H5LTset_attribute_string(_h5file, tname, "name", tableNames[t-1]);

        if (_dataset[t]<0) {
            fprintf(stderr, "Error creating dataset %s",tname);
            exit(2);
        }
    }

    rtn = H5Pclose(plist);
    rtn = H5Sclose(dataspace);
}

