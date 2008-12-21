/*
	RAW2C Convertor
	This program converts RAW image files (as in without headers, 
	compression or palette data) into a C array which is easier to
	use in your GameBoy Advance games.

	Usage: raw2c image.raw

	After successfully calling this program, a file called output.c
	will be created with the proper code.

	Author: Jason Fuerstenberg (jfuerstenberg AT hotmail DOT com)
*/


/* include files. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*	global variables. */
char usage[] = "Usage: raw2c <image_file> <output_file> <name>"; // standard error message
char out_header[64]; // standard code for arrays
char out_footer[] = "\n};\n"; // end of code
FILE *in = 0; // the pointer to the raw file
FILE *out = 0; // the pointer to the c file
long in_len = 0; // the length of the raw file
unsigned char *in_buf = 0; // the buffer to hold the raw file data
char temp[5] = ""; // temporary placeholder of converted bytes (raw > ascii)
long loop; // loop counter

/* Application entry point. */
int main(int argc, char *argv[])
{
	/* 
		Check that the command line included the raw image filename.
		If not exit the usage text.
	*/
	if(argc != 4)
	{
		printf(usage);
		return 0;
	}

	/*
		Open the raw file.
	*/
	in = fopen(argv[1], "rb");

	/*
		If the file pointer is null the file must not exist.
		Display an error message and exit.
	*/
	if(in == 0)
	{
		printf("The file specified could not be found!");
		return;
	}

	/*
		Get the file's size.
	*/
	fseek(in, 0, SEEK_END);
	in_len = ftell(in);
	fseek(in, 0, SEEK_SET);

	/*
		If the file's size is empty return an error message and close the file.
	*/
	if(in_len == 0)
	{
		printf("The file specified cannot be empty!");
		fclose(in);
		return;
	}

	/*
		Create the memory buffer and read the file in and close it.
	*/
	in_buf = malloc(in_len);
	fread(in_buf, in_len, 1, in);
	fclose(in);

	/*
		Open the output file.
	*/
  out = fopen(argv[2], "w");

	/*
		If the output file could not be opened we cannot continue.
		Show error message.
	*/
	if(out == 0)
	{
		printf("Cannot create the output file!");
		free(in_buf);
		return;
	}

	/*
		Write the header to the file.
	*/
  if( strstr(argv[3], ".") ) *strstr(argv[3], ".") = '\0';
  sprintf(out_header, "unsigned char %s[] = {\n", argv[3]);
	fwrite(out_header, strlen(out_header), 1, out);

	/*
		Iterate through the all the bytes in the raw data and convert
		each to ascii followed by a comma.
	*/
	for(loop = 0; loop < in_len; loop++)
	{
		if(loop == in_len - 1)
		{
			sprintf(temp, "0x%02x\0", in_buf[loop]);
		}
		else
		{
			sprintf(temp, "0x%02x,\0", in_buf[loop]);
		}

		// limit lines to 16 items:
		if(!(loop % 16))
		{
			fwrite("\n", strlen("\n"), 1, out);
		}

		fwrite(temp, strlen(temp), 1, out);
	}

	/*
		Write the footer and close the file.
	*/
	fwrite(out_footer, strlen(out_footer), 1, out);
	fclose(out);

	/*
		Inform user of success.
	*/
	printf("Conversion suceeded!\n");
	free(in_buf);

  return 0;
}
