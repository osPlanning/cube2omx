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
    _nRows = 0;
    _nCols = 0;
    _memspace = -1;
}

//Destructor
OMXMatrix::~OMXMatrix()
{
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

void OMXMatrix::createFile(int tables, int rows, int cols, vector<string> &tableNames, string fileName) {
    _fileOpen = true;
    _mode = MODE_CREATE;

    _nRows = rows;
    _nCols = cols;
    _nTables = tables;

    // Create the physical file
    _h5file = H5Fcreate(fileName.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (0 > _h5file) {
        fprintf(stderr, "ERROR: Could not create file %s.\n", fileName.c_str());
    }

    // Build SHAPE attribute
    const int shape[2] = {rows, cols};

    // Write file attributes
    H5LTset_attribute_string(_h5file, "/", "OMX_VERSION", "0.2");
    H5LTset_attribute_int(_h5file, "/", "SHAPE", &shape[0], 2);
   
    // save the order that matrices are written
    hid_t plist = H5Pcreate (H5P_GROUP_CREATE);
    H5Pset_link_creation_order(plist, H5P_CRT_ORDER_TRACKED);
   
    // Create folder structure
    H5Gcreate(_h5file, "/data", NULL, plist, NULL);
    H5Gcreate(_h5file, "/lookup", NULL, plist, NULL);
    
    H5Pclose(plist);
    
    // Create the datasets
    init_tables(tableNames);
}

void OMXMatrix::writeRow(string table, int row, double *rowdata) {
    if (_tableLookup.count(table)==0) {
            throw NoSuchTableException();
    }

    hsize_t count[2], offset[2];

    count[0] = 1;
    count[1] = _nCols;

    offset[0] = row-1;
    offset[1] = 0;

    if (_memspace <0 ) _memspace = H5Screate_simple(2,count,NULL);

    if (_dataspace.count(table)==0) {
        _dataspace[table] = H5Dget_space(_dataset[table]);
    }

    H5Sselect_hyperslab (_dataspace[table], H5S_SELECT_SET, offset, NULL, count, NULL);

    if (0 > H5Dwrite(_dataset[table], H5T_NATIVE_DOUBLE, _memspace, _dataspace[table], H5P_DEFAULT, rowdata)) {
        fprintf(stderr, "ERROR: writing table %s, row %d\n", table.c_str(), row);
        exit(2);
    }
}

//Read/Open operations ------------------------------------------------------

void OMXMatrix::openFile(string filename) {
    // Try to open the existing file
    _h5file = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    if (_h5file < 0) {
        fprintf(stderr, "ERROR: Can't find or open file %s",filename.c_str());
        exit(2);
    }

    // OK, it's open and it's HDF5;
    // Now query some things about the file.
    _fileOpen = true;
    _mode = MODE_READ;

    int shape[2];

    herr_t status = 0;
    status += H5LTget_attribute_int(_h5file, "/", "SHAPE", &shape[0]);
    if (status < 0) {
        fprintf(stderr, "ERROR: %s doesn't have SHAPE attribute\n", filename.c_str());
        exit(2);
    }
    _nRows = shape[0];
    _nCols = shape[1];

    readTableNames();
}

int OMXMatrix::getRows() {
    return _nRows;
}

int OMXMatrix::getCols() {
    return _nCols;
}

int OMXMatrix::getTables() {
    return _nTables;
}

string OMXMatrix::getTableName(int table) {
    return _tableName[table];
}

void OMXMatrix::getRow (string table, int row, void *rowptr) {
    hsize_t data_count[2], data_offset[2];

    // First see if we've opened this table already
    if (_dataset.count(table)==0) {
        // Does this table exist?
        if (_tableLookup.count(table)==0) {
            throw MatrixReadException() ;
        }
        _dataset[table] = openDataset(table);
    }

    data_count[0] = 1;
    data_count[1] = _nCols;
    data_offset[0] = row-1;
    data_offset[1] = 0;

    // Create dataspace if necessary.  Don't do every time or we'll run OOM.
    if (_dataspace.count(table)==0) {
        _dataspace[table] = H5Dget_space(_dataset[table]);
    }

    // Define MEMORY slab (using data_count since we don't want to read zones+1 values!)
    if (_memspace < 0) {
        _memspace = H5Screate_simple(2, data_count, NULL);
    }

    // Define DATA slab
    if (0 > H5Sselect_hyperslab (_dataspace[table], H5S_SELECT_SET, data_offset, NULL, data_count, NULL)) {
        fprintf(stderr, "ERROR: Couldn't select DATA subregion for table %s, subrow %d.\n",
                table.c_str(),row);
        exit(2);
    }

    // Read the data!
    if (0 > H5Dread(_dataset[table], H5T_NATIVE_DOUBLE, _memspace, _dataspace[table],
            H5P_DEFAULT, rowptr)) {
        fprintf(stderr, "ERROR: Couldn't read table %s, subrow %d.\n",table.c_str(),row);
        exit(2);
    }
}

void OMXMatrix::closeFile() {
    for(map<string,hid_t>::iterator iterator = _dataset.begin(); iterator != _dataset.end(); iterator++) {
        H5Dclose(iterator->second);
    }

    for(map<string,hid_t>::iterator iterator = _dataspace.begin(); iterator != _dataspace.end(); iterator++) {
        H5Sclose(iterator->second);
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

hid_t OMXMatrix::openDataset(string table) {

    string tname = "/data/" + table;
    
    hid_t dataset = H5Dopen(_h5file, tname.c_str(), H5P_DEFAULT);
    if (dataset < 0) {
        throw InvalidOperationException();
    }

    return dataset;
}

/*
 * Group traversal function. Build list of tablenames from this.
 */
herr_t _leaf_info(hid_t loc_id, const char *name, const H5L_info_t *info, void *opdata)
{
    OMXMatrix *m = (OMXMatrix *) opdata;

    m->_nTables++;
    m->_tableName[m->_nTables] = name;
    m->_tableLookup[name] = m->_nTables;
    return 0;
}

/* Read table names.  Sets number of tables in file, too. */
void OMXMatrix::readTableNames() {

    _nTables = 0;
    _tableLookup.clear();
    _dataset.clear();
    _dataspace.clear();
    unsigned flags = 0;

    hid_t datagroup = H5Gopen(_h5file, "/data", H5P_DEFAULT);

    // if group has creation-order index, use it
    hid_t info = H5Gget_create_plist(datagroup);
    H5Pget_link_creation_order(info, &flags);
    H5Pclose(info);

    if (flags & H5P_CRT_ORDER_TRACKED) {
    	// Call _leaf_info() for every child in /data:
        H5Literate(datagroup, H5_INDEX_CRT_ORDER, H5_ITER_INC, NULL, _leaf_info, this);
    } else {
    	// otherwise just use name order
    	H5Literate(datagroup, H5_INDEX_NAME, H5_ITER_INC, NULL, _leaf_info, this);
    }

    H5Gclose(datagroup);
}



void OMXMatrix::init_tables (vector<string> &tableNames) {

    hsize_t     dims[2]={_nRows,_nCols};
    hid_t       plist;
    herr_t      rtn;
    hsize_t     chunksize[2];
    double      fillvalue[1];

    fillvalue[0] = 0.0;
    chunksize[0] = 1;
    chunksize[1] = _nCols;

    hid_t   dataspace = H5Screate_simple(2,dims, NULL);

    // Use a row-chunked, zip-compressed data format:
    plist = H5Pcreate(H5P_DATASET_CREATE);
    rtn = H5Pset_chunk(plist, 2, chunksize);
    rtn = H5Pset_deflate(plist, 7);
    rtn = H5Pset_fill_value(plist, H5T_NATIVE_DOUBLE, &fillvalue);

    // Loop on all TP+ tables
    for (unsigned int t=0; t<tableNames.size(); t++) {
        string tpath = "/data/" + tableNames[t];
        string tname(tableNames[t]);
        
        // Create a dataset for each table
        _dataset[tname] = H5Dcreate2(_h5file, tpath.c_str(), H5T_NATIVE_DOUBLE,
                                 dataspace, H5P_DEFAULT, plist, H5P_DEFAULT);
        if (_dataset[tname]<0) {
            fprintf(stderr, "Error creating dataset %s",tpath.c_str());
            exit(2);
        }
        
        // Save the something somewhere
        _tableLookup[tname] = t+1;
        int cube_num = t+1;
        H5LTset_attribute_int(_h5file, tpath.c_str(), "CUBE_MAT_NUMBER", &cube_num, 1);
    }

    rtn = H5Pclose(plist);
    rtn = H5Sclose(dataspace);
}

