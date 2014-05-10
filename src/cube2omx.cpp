#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>

#include <hdf5.h>
#include <hdf5_hl.h>

#include "tppmatrix.h"
#include "omxmatrix.h"

int convertMat2h5(char *);
int convertH5toMat(char *);
string get_new_extension(char *filename, const char *ext);

int generateCubeOrder(map<int,string> &lookup, OMXMatrix* omx, int tables, const char* tnames[]);

int copy_data(TPPMatrix*, OMXMatrix*, int, int, vector<string>&);
int copy_data(OMXMatrix*, TPPMatrix*, int, int, map<int,string>&);

bool isOMX(char*);

hid_t _memspace = -1;
hid_t _dataspace = -1;

int main(int argc, char* argv[])
{
    // Get cmdline parameters
    // for each input .mat file
    cout << "\nCube MAT/OMX Converter (built " << __DATE__ << " " << __TIME__ << ")\n";
    int errors = 0;

    if (argc==1) {
		cout << "\nUsage:  cube2omx.exe  [filename1] [filename2] ...\n";
		cout << "        - Valid OMX files will be converted to Cube format\n";
		cout << "        - Cube files will be converted to OMX\n";
		cout << "        - Output files will have .omx or .mat extension\n\n";
		exit(0);
    }

    for (int i=1; i<argc; i++) {
        char *tpfilename = argv[i];
        printf("\n\nConverting %s ",tpfilename);

	// Make sure we can open it
	ifstream file(tpfilename, ifstream::in);
	if (!file) {
		fprintf(stderr, "\n** Cannot find/open %s\n", tpfilename);
		errors++;
		continue;
	} else {
		file.close();
	}

        // Figure out which way we're converting:
        bool is_omx = isOMX(tpfilename);
        int v;

        if (is_omx) {
            printf("to Cube: ");
            v = convertH5toMat(tpfilename);
        } else {
            printf("to OMX: ");
            v = convertMat2h5(tpfilename);
        }

        if (v != 0) {
            printf("\n>> Failed to convert %s.",tpfilename);
            errors += v;
        }
    }

    printf("\nDone; %d errors and %d of %d completed.\n",errors,argc-errors-1,argc-1);
}


bool isOMX(char *filename) {
    htri_t answer = H5Fis_hdf5(filename);
    if (answer<=0) return false;

    // It's HDF5; is it OMX?
    hid_t f = H5Fopen(filename,H5F_ACC_RDONLY,H5P_DEFAULT);
    herr_t exists = H5LTfind_attribute(f, "OMX_VERSION");

    //don't actually care what OMX version it is, yet...
    //char version[255];
    //int status = H5LTget_attribute_string(f,"/","OMX_VERSION", version);
    H5Fclose(f);

    if (exists==0)  {
        fprintf(stderr,"\n** %s is HDF5, but is not a valid OMX file.\n",filename);
        exit(2);
    }

    return true;
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
        string h5_name = get_new_extension(filename, ".omx");
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

int generateCubeOrder(map<int,string> &lookup, OMXMatrix* omx, int tables, const char* tnames[]) {
    // Make sure there is EXACTLY one table for each CUBE_MAT_NUMBER in the
    // table range. Fail if there are dupes or missing numbers.
    boolean quit = false;

    for (int i=0; i<tables;i++) {
        string tablename(tnames[i]);
        int cubenum = omx->getCubeNumber(tablename);

        if (cubenum<1) {
            fprintf(stderr, "\n** Table %s does not have required CUBE_MAT_NUMBER attribute",tnames[i]);
            quit = true;
            continue;
        }
        if (lookup.count(cubenum)>0) {
            fprintf(stderr, "\n** Table %s has duplicate CUBE_MAT_NUMBER attribute: %s (%d)",tnames[i],lookup[cubenum].c_str(), cubenum);
            quit = true;
            continue;
        }

        lookup[cubenum] = tablename;
    }

    if (quit) return -1;

    // And finally make sure we're contiguous
    bool okay = true;
    for (int i=1; i<=tables;i++) {
        if (lookup.count(i)==0) {
            fprintf(stderr, "\n** CUBE_MAT_NUMBER %d is missing from table range 1-%d",i,tables);
            okay = false;
        }
    }
    if (okay) return 0;
    return -1;
}

int convertH5toMat(char *filename) {
    int zones, tables, rtn;
    TPPMatrix *tpp;
    OMXMatrix *omx;
    const char* tnames_native[MAX_TABLES];     // OMX doesn't have any idea about matrix 'order'
    map<int,string> tnames_cube_lookup; // Cube needs things in a specific order.
    const char* tnames_cube_order[MAX_TABLES];

    // Open h5 file and get dimensions, table names
    omx = new OMXMatrix();
    omx->openFile(filename);

    tables = omx->getTables();
    zones  = omx->getRows();

    for (int t=1; t<=tables; t++) {
        tnames_native[t-1]=omx->getTableName(t).c_str();
    }

    // Verify and set up Cube matrix order from CUBE_MAT_NUMBER attributes
    int status = generateCubeOrder(tnames_cube_lookup, omx, tables, tnames_native);
    if (status<0) return 1;

    for (int i=0; i<tables;i++) {
        tnames_cube_order[i] = tnames_cube_lookup[i+1].c_str();
    }

    // create TPP file
    try {
        string tppname = get_new_extension(filename, ".mat");
        tpp = new TPPMatrix();
        tpp->createFile(tables, zones, tnames_cube_order, tppname.c_str());

    } catch (TPPMatrix::FileOpenException&) {
        printf("Can't open %s.",filename);
        return 1;
    }

    // Copy data
    rtn = copy_data(omx, tpp, zones, tables, tnames_cube_lookup);

    /* Close the files. */
    tpp->closeFile();
    omx->closeFile();

    return 0;
}

// Copy from HDF5 to TPP:
int copy_data(OMXMatrix *omx, TPPMatrix *matrix, int zones, int tables, map<int,string> &order) {

    // Set up some scratch space for reading row data
    double *rowdata = matrix->allocateRowBuffer();

    int row;
    printf("\n");

    // Loop on all rows
    for (row=1;row<=zones;row++) {
        if (row % 127 ==0) printf("Zone: %d\r",row);

        // Loop for each table
        for (int t=1;t<=tables;t++) {
            // Grab a row of data
            omx->getRow(order[t], row, rowdata);

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


// Replace extension .mat with .h5 in filename, for example
string get_new_extension(char *filename, const char* ext) {

    string str(filename);
    unsigned found = str.find_last_of('.');

    string newname = str.substr(0,found) + ext;
    printf("%s\n",newname.c_str());

    return newname;
}

