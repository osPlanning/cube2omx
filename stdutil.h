/*   stdutil.h

     Standard Utilities.

     Copyright (c) Tim Heier 1992
     All Rights Reserved.
*/

#include <stdlib.h>
//#include <math.h>
#include <stdio.h>
#include <string.h>
//#include <time.h>
#include <ctype.h>
//#include <time.h>
//#include <float.h>
// #include <unistd.h>

#define TAB            '\t'                       /* common characters */
#define FORMFEED       '\f'
#define BELL           '\007'
#define NEWLINE        '\n'

#define FOREVER        for(;;)                    /* syntactic extenions */
#define AND            &&
#define OR             ||
#define NOT            !

#define ESC             27                        /* key definitions */
#define HOMEKEY        327                        /* key definitions */
#define ENDKEY         335
#define UPKEY          328
#define DOWNKEY        336
#define PGUPKEY        329
#define PGDNKEY        337
#define LEFTKEY        331
#define INSKEY         338
#define RIGHTKEY       333
#define DELKEY         339
#define CTRLLEFTKEY    371
#define CTRLRIGHTKEY   372
#define CTRLEND        373
#define F1             315
#define F2             316
#define F3             317
#define F4             318
#define F5             319
#define F6             320
#define F7             321
#define F8             322
#define F9             323
#define F10            324
#define MIN(X,Y)       ((X) < (X) ? (X) : (Y))    /* minimum of X and Y */
#define MAX(X,Y)       ((X) < (Y) ? (Y) : (X))    /* maximum of X and Y */
#define ABS(X)         ((X) < 0) ? -(X) : (X))    /* absolute value */
#define INRANGE(X,Y,Z) ((X >= (Y) && (X) <= (Z))  /* is X < Y and >Z? */

#define SUCCESS  1                                /* constants */
#define FAIL     0
#define TRUE     1
#define FALSE    0
#define PI       3.14159265

#define TERROR_EXIT  -1     /* used by terror() */
#define TERROR_NULL   0

int   get_int(char message[]);
int   get_long(char message[]);
float get_float(char message[]);
int   file_exists(char filename[]);    /* returns: 1=exists 0=!exists */
				       /* mode = I,O message = prompt */
int   file_copy(char *source, char *dest);
int   get_line(char line[], int max);  /* reads until a new line saving MAX */
				       /* reads from a file until a new line saving MAX */
int   fget_line(char line[], int max, FILE *fp);
int   str_index(char *string, char *sub_string);
void  pause_exit(void);                /* pauses programs and then exits */
void  disk_error(void);                /* displays error and pauses */
long  getlong(char *buf, int start, int length);  /* parses a char string */
float getfloat(char *buf, int start, int length); /* parses a char string */
void  mkupper(char *dstr, char *ostr); /* converts a string to upper case */
void  terror(char *message,int errlevel, char *ErrorFile);

