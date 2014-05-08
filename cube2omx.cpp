#include <cstdlib>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>

#include <hdf5.h>
#include <hdf5_hl.h>

#include "tppmatrix.h"
#include "omxmatrix.h"

int convertMat2h5(char *);
int convertH5toMat(char *);
string get_h5_name(char *filename);
string get_tpp_name(char *filename);

int copy_data(TPPMatrix*, OMXMatrix*, int, int, vector<string>&);
int copy_data(OMXMatrix*, TPPMatrix*, int, int);

hid_t _memspace = -1;
hid_t _dataspace = -1;

int main(int argc, char* argv[])
{
    // Get cmdline parameters
    // for each input .mat file
    cout << "\nCube MAT/OMX Converter (built " << __DATE__ << ")\n";
    int errors = 0;

    if (argc==1) {
		cout << "\nUsage:  cube2omx.exe  [filename1] [filename2] ...\n";
		cout << "        - Files ending in .omx or .OMX will be converted from OMX to Cube\n";
		cout << "        - Files ending in anything else will be converted from Cube to OMX\n\n";
		exit(0);
    }

    for (int i=1; i<argc; i++) {
        char *tpfilename = argv[i];
        printf("\n\nConverting %s to ",tpfilename);

        // Figure out which way we're converting:
        string str(tpfilename);
        int v;

        if (((size_t)str.find(".omx")==string::npos) &&
            ((size_t)str.find(".OMX")==string::npos)) {
            v = convertMat2h5(tpfilename);
        } else {
            v = convertH5toMat(tpfilename);
        }

        if (v!=0) {
            printf(">> Failed to convert %s.",tpfilename);
            errors += v;
        }
    }

    printf("\nDone; %d errors and %d of %d completed.\n",errors,argc-errors-1,argc-1);
}

int convertMat2h5(char *filename) {
    int rows, cols, tables, rtn;
    TPPMatrix *matrix;
    OMXMatrix *omx;

    vector<string> matNames;

    try {
        // try to open file
        matrix = new TPPMatrix();
        matrix->openFile(filename);

        // get tp+ parameters such as zones, tables, names.
        rows = cols = matrix->getZones();

        tables = matrix->getTables();
        for (int t=1; t<=tables; t++) {
            string name(matrix->getTableName(t));
            matNames.push_back(name);
        }

        // Create OMX file
        string h5_name = get_h5_name(filename);
        omx = new OMXMatrix();
        omx->createFile(tables, rows, cols, matNames, h5_name);

        // Copy data
        rtn = copy_data(matrix, omx, rows, tables, matNames);

        // All done
        matrix->closeFile();
        omx->closeFile();

    } catch (TPPMatrix::FileOpenException&) {
        printf("Can't open %s.",filename);
        return 1;
    }

    return 0;
}

int convertH5toMat(char *filename) {
    int zones, tables, rtn;
    TPPMatrix *tpp;
    OMXMatrix *omx;
    const char* tnames[MAX_TABLES];

    try {
        // Open h5 file and get dimensions, table names
        omx = new OMXMatrix();
        omx->openFile(filename);

        tables = omx->getTables();
        zones  = omx->getRows();

        for (int t=1; t<=tables; t++) {
            tnames[t-1]=omx->getTableName(t).c_str();
        }

        // create TPP file
        string tppname = get_tpp_name(filename);
        tpp = new TPPMatrix();
        tpp->createFile(tables, zones, tnames, tppname.c_str());

        // Copy data
        rtn = copy_data(omx, tpp, zones, tables);

        /* Close the files. */
        tpp->closeFile();
        omx->closeFile();

    } catch (TPPMatrix::FileOpenException&) {
        printf("Can't open %s.",filename);
        return 1;
    }

    return 0;
}

// Copy from HDF5 to TPP:
int copy_data(OMXMatrix *omx, TPPMatrix *matrix, int zones, int tables) {

    // Set up some scratch space for reading row data
    double *rowdata = matrix->allocateRowBuffer();

    int row;
    printf("\n");

    string tableNames[MAX_TABLES+1];
    for (int t = 1; t <=tables; t++) {
        tableNames[t] = omx->getTableName(t);
    }

    // Loop on all rows
    for (row=1;row<=zones;row++) {
        if (row % 127 ==0) printf("Zone: %d\r",row);

        // Loop for each table
        for (int t=1;t<=tables;t++) {
            // Grab a row of data
            omx->getRow(tableNames[t], row, rowdata);

            // And write it to h5
            matrix->writeRow(t, row, rowdata);
        }
    }

    printf("Zone: %d\n",row-1);
    free(rowdata);
    return 0;
}

// Copy from TPP to HDF5:
int copy_data(TPPMatrix *matrix, OMXMatrix *omx, int zones, int tables, vector<string> &matNames) {
    double*     rowdata;

    // Set up some scratch space for reading row data
    rowdata = matrix->allocateRowBuffer();

    // Loop for each row
    int col;
    for (col=1;col<=zones;col++) {
        if (col %47 == 1) printf("\r%d tables:  zone %d     ",tables, col);
        for (int t=1; t<=tables; t++) {
            // Grab a row of data
            try {
                matrix->getRow(t, col, rowdata);
            } catch (TPPMatrix::MatrixReadException&) {
                    fprintf(stderr, "ERROR: Can't read table row %d in table %d!\n", col, t);
                    exit(2);
            }

            // And write it to h5
            omx->writeRow(matNames[t-1], col, rowdata);
        }
    }

    // Clean up
    printf("\r%d tables:  zone %d     \n",tables, col-1);

    free(rowdata);
    return 0;
}


// Replace .mat with .h5 in filename
string get_h5_name(char *filename) {

    string str(filename);
    size_t dot = str.rfind('.');

    str.erase(dot,4);
    str.insert(dot,".omx");
    printf("%s\n",str.c_str());

    return str;
}

// Replace .mat with .h5 in filename
string get_tpp_name(char *filename) {

    string str(filename);
    size_t dot = str.rfind('.');

    str.erase(dot,4);
    str.insert(dot,".mat");
    printf("%s\n",str.c_str());

    return str;
}

