#include <io.h>
#include "stdutil.h"


/*************************************************************************/
int get_int(char message[])
{
  char inbuf[20];
  int number;

  printf("\n%s",message);

/* sscanf will return -1 if a null string character is read */

  while ( sscanf(gets(inbuf),"%d",&number) <= 0 )
	  printf("%s",message);

  return(number);
}

/*************************************************************************/
int get_long(char message[])
{
  char inbuf[20];
  int number;

  printf("\n%s",message);

/* sscanf will return -1 if a null string character is read */

  while ( sscanf(gets(inbuf),"%d",&number) <= 0 )
     printf("%s",message);

  return(number);
}

/*************************************************************************/
float get_float(char message[])
{
  char inbuf[20];
  float number;

  printf("\n%s",message);

/* sscanf will return -1 if a null string character is read */

  while ( sscanf(gets(inbuf),"%f",&number) <= 0 ) {
     printf("%s",message);
  }
  return(number);
}

/*************************************************************************/
int file_exists(char filename[])
/*
  "access" function return codes:
		0 = file exists
		-1 = file does not exists
  file_exists return codes:
		1 = file exits
		0 = file does not exists
*/
{
  if (access(filename,0) == 0)
    return(1);
  else
    return(0);
}

/*************************************************************************/
int file_copy(char *source, char *dest)
/*
  Copy "source" to "dest" - OVERWRITE "dest" if it exists.
*/
{
  FILE *sfp=NULL;
  FILE *dfp=NULL;
  int c;

  if((sfp = fopen(source,"r")) == NULL)
     printf("\nCouldn't open %s for reading\n\n", source);
  else {
     if ((dfp = fopen(dest, "w")) == NULL)
	printf("\nCouldn't open %s for writing\n\n", dest);
     else {
	while ((c = getc(sfp)) != EOF)  /* do copy */
	   putc(c, dfp);
	(void) fclose(dfp);
     }
     (void) fclose(sfp);
  }
  return sfp && dfp;
}

/*************************************************************************/
int get_line (char line[], int max)
/* Read characters (unitl a new line) saving them in an array.  The
   trailing newline is replaced with a NULL character.  The number  of
	characters read is returned or EOF when the end of the input is
   reached.
*/
{
  int c;
  int i = 0;

  while ( ((c = getc(stdin)) != '\n') && (c != EOF) )
     if (i < max-1)
	line[i++] = c;

  line[i] = '\0';
  if (c == EOF)
     return EOF;
  return i;
}

/*************************************************************************/
int fget_line (char line[], int max, FILE *fp)
/* Read characters from a FILE (unitl a new line) saving them in an array.
	The trailing newline is replaced with a NULL character.  The number  of
   characters read is returned or EOF when the end of the input is
   reached.
*/
{
  int c;
  int i = 0;

  while ( ((c = getc(fp)) != '\n') && (c != EOF) )
     if (i < max-1)
	line[i++] = c;

  line[i] = '\0';
  if (c == EOF)
     return EOF;
  return i;
}

/*************************************************************************/
int str_index(char *string, char *sub_string)
/* finds the first occurance of a sub-string in a character string.
   returns:   -1  = no match
             >=0  = the starting postion of first match
*/
{
  int index = 0, len_sub;
  int result = -1;

  len_sub = (int)strlen(sub_string);
  while ( (string[index] != '\0') &&
	  ((result = strncmp(&string[index], sub_string, len_sub)) != 0) ) {
     index++;
     }
     return ( (result != 0) ? -1 : index);
}

/*************************************************************************/
void pause_exit(void)
/* pauses program execution and then exits to operating system */
{
  printf("\npress any key...");
  getc(stdin);
  printf("\n");
  exit(2);
}

/*************************************************************************/
void disk_error(void)
{
  printf("\n");
  printf("   ERROR WRITING TO CURRENT DISK!!\n");
  printf("        DISK COULD BE FULL\n");

  pause_exit();
}

/*************************************************************************/
float getfloat(char *buf, int start, int length)
{
char newbuf[80+1];

if( (length <= 0) || (length > 80) || (((int)strlen(buf)-start+1) < length) )
   return(0);

strncpy(newbuf,&buf[start],length);
newbuf[length]='\0';

return (float)(atof(newbuf));
}

/*************************************************************************/
long getlong(char *buf, int start, int length)
{
char newbuf[80+1];

if( (length <= 0) || (length > 80) || (((int)strlen(buf)-start+1) < length) )
   return(0);

strncpy(newbuf,&buf[start],length);
newbuf[length]='\0';

return (atol(newbuf));
}

/*********************************************************************/
void terror(char *message,int errlevel, char *ErrorFile)
{
FILE *fptr=NULL;

printf("error: %s\n",message);

if(ErrorFile != NULL) {
   fptr = fopen(ErrorFile,"a");
   fprintf(fptr,"error: %s\n",message);
   }
fclose(fptr);

if(errlevel < 0)
   exit(-1);

return;
}

/*********************************************************************/
void mkupper(char *dstr, char *ostr)
{
int length, i;

length = (int) strlen(ostr);
for (i=0; i<length; i++) {
   dstr[i] = toupper(ostr[i]);
   }

return;
}
