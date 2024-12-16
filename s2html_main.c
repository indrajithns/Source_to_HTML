/*
 * Source Code to HTML Conversion Program
 *
 * This program reads a source code file, parses its content, and converts it into an HTML file.
 * The program checks if the source file exists and generates an HTML output with the same name as 
 * the source file (or a provided name) but with a ".html" extension.
 *
 * Functions:
 * - html_begin: Adds the opening HTML tags to the destination file.
 * - html_end: Adds the closing HTML tags to the destination file.
 * - get_parser_event: Retrieves the next parsing event from the source file.
 * - source_to_html: Converts parsed source code events into HTML and writes to the destination file.
 *
 * Error Handling:
 * - The program verifies the existence of the source file and the ability to create the output file.
 * - It checks if the correct arguments are provided for input and output file handling.
*/

#include <stdio.h>
#include "s2html_event.h"
#include "s2html_conv.h"

int main (int argc, char *argv[])
{
    FILE *sfp, *dfp; // source and destination file descriptors 
    pevent_t *event;
    char dest_file[100];

    // Check if file name is provided
    if(argc < 2)
    {
        printf("\nError!!! Please Enter File Name And Mode\n");
        printf("Usage: <executable> <file name> \n");
        printf("Example_1 : ./a.out test.c\n\n");
        printf("Example_2 : ./a.out test.txt\n\n");
        return 1;
    }
    
    #ifdef DEBUG
    printf("File To Be Opened : %s\n", argv[1]);
    #endif

    // Open source file
    if(NULL == (sfp = fopen(argv[1], "r")))
    {
        printf("Error!!! File %s Could Not Be Opened\n", argv[1]);
        return 2;
    }

    // Check for output file name, default to source file name with .html extension
    if (argc > 2)
    {
        sprintf(dest_file, "%s.html", argv[2]);
    }
    else
    {
        sprintf(dest_file, "%s.html", argv[1]);
    }

    // Open destination file
    if (NULL == (dfp = fopen(dest_file, "w")))
    {
        printf("Error!!! Could Not Create %s Output File\n", dest_file);
        return 3;
    }

    // Write HTML starting tags
    html_begin(dfp, HTML_OPEN);

    // Read source file, convert to HTML, and write to destination file
    do
    {
        event = get_parser_event(sfp);
        source_to_html(dfp, event);
    } while (event->type != PEVENT_EOF);

    // Write HTML ending tags
    html_end(dfp, HTML_CLOSE);
    
    // Output success message
    printf("\nOutput File %s Generated\n\n", dest_file);

    // Close files
    fclose(sfp);
    fclose(dfp);

    return 0;
}
