#include <cstdlib>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>

#define BOOST_USE_WINDOWS_H 1
#include <boost/thread.hpp>

#include <hdf5.h>
#include <hdf5_hl.h>

#include "tppmatrix.h"
#include "../util/h5matrix.h"

#include "stdutil.h"
#include "define.h"

int h5_init_tables(hid_t, int, int);
int h5_write_row(hid_t, char*, int, int, double*);

int convertMat2h5(char *);
int convertH5toMat(char *);
string get_h5_name(char *filename);
string get_tpp_name(char *filename);

int copy_data(TPPMatrix*, hid_t, int, int);
int copy_data(H5Matrix*, TPPMatrix*, int, int);

hid_t _memspace = -1;
hid_t _dataspace = -1;

int main(int argc, char* argv[])
{
	// Get cmdline parameters
	// for each input .mat file
	printf("TP+ MAT/HDF5 Converter\n");
	int errors = 0;

	for (int i=1; i<argc; i++) {
		char *tpfilename = argv[i];
		printf("\n\nConverting %s to ",tpfilename);

		// Figure out which way we're converting:
		string str(tpfilename);
		int v;

		if (((size_t)str.find(".h5")==string::npos) &&
		    ((size_t)str.find(".H5")==string::npos)) {
	        v = convertMat2h5(tpfilename);
	    } else {
            v = convertH5toMat(tpfilename);
	    }

		if (v!=0) {
			printf(">> Failed to convert %s.",tpfilename);
			errors += v;
		}
	}

	printf("\nDone; %d errors out of %d attempted.\n",errors,argc-1);
}

int convertMat2h5(char *filename) {
    int zones, tables, rtn;
    TPPMatrix *matrix;

	try {
		// try to open file
		matrix = new TPPMatrix();
		matrix->openFile(filename);

		// get tp+ parameters such as zones, tables, names.
		zones = matrix->getZones();
		tables = matrix->getTables();

		// Create h5 file and set dimensions, table names
		string h5_name = get_h5_name(filename);
		hid_t h5file = H5Fcreate(h5_name.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
		rtn = h5_init_tables(h5file,zones,tables);

		// Copy data
		rtn = copy_data(matrix,h5file,zones,tables);

		/* Close the files. */
		H5Sclose(_memspace);
		H5Sclose(_dataspace);
        H5Fclose(h5file);
        matrix->closeFile();
        _memspace = _dataspace = -1;

	} catch (TPPMatrix::FileOpenException) {
		printf("Can't open %s.",filename);
		return 1;
	}

	return 0;
}

int convertH5toMat(char *filename) {
    int zones, tables, rtn;
    TPPMatrix *tpp;
    char* tnames[500];

    try {
        // Open h5 file and get dimensions, table names
        H5Matrix *h5file = new H5Matrix();
        h5file->openFile(filename);

        tables = h5file->getTables();
        zones  = h5file->getZones();

        for (int t=0; t<h5file->getTables(); t++) {
            tnames[t]=h5file->getTableName(t+1);
        }

        // create TPP file
        string tppname = get_tpp_name(filename);
        tpp = new TPPMatrix();
        tpp->createFile(tables, zones, tnames, (char*)tppname.c_str());

        // Copy data
        rtn = copy_data(h5file, tpp, zones, tables);

        /* Close the files. */
        tpp->closeFile();
        h5file->closeFile();

    } catch (TPPMatrix::FileOpenException) {
        printf("Can't open %s.",filename);
        return 1;
    }

    return 0;
}

// Copy from HDF5 to TPP:
int copy_data(H5Matrix *h5file, TPPMatrix *matrix, int zones, int tables) {

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
            h5file->getRow(t, row, rowdata);

            // And write it to h5
            matrix->writeRow(t, row, rowdata);
        }
    }
    printf("Zone: %d\n",row-1);
    free(rowdata);
    return 0;
}

// Copy from TPP to HDF5:
int copy_data(TPPMatrix *matrix, hid_t h5file,int zones,int tables) {
	hid_t  		dataset[500];
	char 		tname[10] = {};
	double* 	rowdata;

	// Set up some scratch space for reading row data
    rowdata = matrix->allocateRowBuffer();

	// Set up datasets for each TP+ table
	for (int t=1;t<=tables;t++) {
        // Get the dataset handle for each table
        itoa(t, tname, 10);
        dataset[t] = H5Dopen(h5file, tname, H5P_DEFAULT);

        // Store the table attributes
        H5LTset_attribute_int(h5file, tname, "zones", &zones, 1);
        H5LTset_attribute_string(h5file, tname, "name", matrix->getTableName(t));
	}

    // Loop for each row
	int col;
    for (col=1;col<=zones;col++) {
        if (col %47 == 1) printf("\r%d tables:  zone %d     ",tables, col);
        for (int t=1;t<=tables;t++) {
            // Grab a row of data
            try {
                matrix->getRow(t, col, rowdata);
            } catch (TPPMatrix::MatrixReadException) {
                    fprintf(stderr, "ERROR: Can't read table row %d in table %d!\n", col, t);
                    exit(2);
            }

            // And write it to h5
            h5_write_row(dataset[t], tname, col, zones, rowdata);
        }
	}

    // Clean up
    printf("\r%d tables:  zone %d     \n",tables, col-1);
    for (int t=1;t<=tables;t++) {
        H5Dclose(dataset[t]);
    }

    free(rowdata);
    return 0;
}

int h5_write_row(hid_t h5dataset, char *tname, int col, int zones, double *rowdata) {

    hsize_t count[2], offset[2], stride[2];

    offset[0] = col-1;
    offset[1] = 0;

    count[0] = 1;
    count[1] = zones;

    stride[0] = stride[1] = 1;

    if (_memspace == -1) {
        _memspace = H5Screate_simple(2,count,NULL);
        _dataspace = H5Dget_space (h5dataset);
    }

    herr_t status = H5Sselect_hyperslab (_dataspace, H5S_SELECT_SET, offset,
                                        stride, count, NULL);
    if (status<0) {
        printf("ERROR: Couldn't select memory slab (time to reboot?)\n");
        exit(2);
    }

    status = H5Dwrite(h5dataset, H5T_NATIVE_DOUBLE, _memspace, _dataspace,
                        H5P_DEFAULT, rowdata);
    if (status<0) {
        printf("ERROR: Couldn't write row %d of table %s", col, tname);
        exit(2);
    }

    return 0;
}

int h5_init_tables (hid_t h5file, int zones,int tables) {
	hsize_t     dims[2]={zones,zones};
	hid_t		plist;
	herr_t      rtn;
	char 		tname[10];
	hsize_t		chunksize[2];
	double		fillvalue[1];

	hid_t     dataset[500];

	fillvalue[0] = 0.0;
	chunksize[0] = 1;
	chunksize[1] = zones;

	hid_t	dataspace = H5Screate_simple(2,dims, NULL);

	// Use a row-chunked, zip-compressed data format:
	plist = H5Pcreate(H5P_DATASET_CREATE);
	rtn = H5Pset_chunk(plist, 2, chunksize);

	rtn = H5Pset_deflate(plist, 7);
	rtn = H5Pset_fill_value(plist, H5T_NATIVE_DOUBLE, &fillvalue);

	printf("%d tables:\n", tables);

	// Set some file attributes for tables and zones
    H5LTset_attribute_int(h5file, "/", "tables", &tables, 1);
    H5LTset_attribute_int(h5file, "/", "zones", &zones, 1);

	// Loop on all TP+ tables
	for (int t=1;t<=tables;t++) {

		// Create a dataset for each table
		itoa(t, tname, 10);
		dataset[t] = H5Dcreate(h5file, tname, H5T_NATIVE_DOUBLE, dataspace,
							   H5P_DEFAULT, plist, H5P_DEFAULT);

		if (dataset[t]<0) {
		    printf("Error creating dataset %s",tname);
		    exit(2);
		}
	    rtn = H5Dclose(dataset[t]);
	}

	rtn = H5Pclose(plist);
	rtn = H5Sclose(dataspace);

	return 0;
}

// Replace .mat with .h5 in filename
string get_h5_name(char *filename) {

    string str(filename);
    size_t dot = str.rfind('.');

    str.erase(dot,4);
    str.insert(dot,".h5");

    printf("%s, ",str.c_str());
    return str;
}

// Replace .mat with .h5 in filename
string get_tpp_name(char *filename) {

    string str(filename);
    size_t dot = str.rfind('.');

    str.erase(dot,3);
    str.insert(dot,".mat");

    printf("%s, ",str.c_str());
    return str;
}
