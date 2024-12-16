/*  
 * Source-to-HTML Conversion Functions
 *
 * This file contains functions to convert source code into HTML with syntax highlighting. 
 * It includes functions to:
 * 1. `html_begin`: Adds the opening HTML tags.
 * 2. `html_end`: Adds the closing HTML tags.
 * 3. `source_to_html`: Converts source code elements into HTML with styling.
*/

#include <stdio.h>
#include "s2html_event.h"
#include "s2html_conv.h"

/* html_begin function definition */

/* Writes the beginning HTML structure to the file (DOCTYPE, HTML, HEAD, BODY tags). */
void html_begin(FILE *dest_fp, int type) /* type => not used, but can be used to add different HTML tags */
{
    fprintf(dest_fp, "<!DOCTYPE html>\n");
    fprintf(dest_fp, "<html lang=\"en-US\">\n");
    fprintf(dest_fp, "<head>\n");
    fprintf(dest_fp, "<title>%s</title>\n", "sode2html");
    fprintf(dest_fp, "<meta charset=\"UTF-8\">\n");
    fprintf(dest_fp, "<link rel=\"stylesheet\" href=\"styles.css\">\n");
    fprintf(dest_fp, "</head>\n");
    fprintf(dest_fp, "<body style=\"background-color:lightgrey;\">\n");
    fprintf(dest_fp, "<pre>\n");
}

/* html_end function definition */

/* Writes the closing HTML tags to the file (BODY, HTML). */
void html_end(FILE *dest_fp, int type) /* type => not used, but can be used to add different HTML tags */
{
    fprintf(dest_fp, "</pre>\n");
    fprintf(dest_fp, "</body>\n");
    fprintf(dest_fp, "</html>\n");
}


/* source_to_html function definition */

/* Converts event data into HTML format and writes it to the file. */
void source_to_html(FILE *fp, pevent_t *event)
{
#ifdef DEBUG
    printf("%s", event->data);  // Debug output to console
#endif

    switch(event->type)
    {
        case PEVENT_PREPROCESSOR_DIRECTIVE:
            fprintf(fp,"<span class=\"preprocess_dir\">%s</span>", event->data);
            break;
        case PEVENT_MULTI_LINE_COMMENT:
        case PEVENT_SINGLE_LINE_COMMENT:
            fprintf(fp,"<span class=\"comment\">%s</span>", event->data);
            break;
        case PEVENT_STRING:
            fprintf(fp,"<span class=\"string\">%s</span>", event->data);
            break;
        case PEVENT_HEADER_FILE:
            if(event->property == USER_HEADER_FILE)
                fprintf(fp,"<span class=\"header_file\">%s</span>", event->data);
            else
                fprintf(fp,"<span class=\"header_file\">&lt;%s&gt;</span>", event->data);
            break;
        case PEVENT_REGULAR_EXP:
        case PEVENT_EOF:
            fprintf(fp,"%s", event->data);
            break;
        case PEVENT_NUMERIC_CONSTANT:
            fprintf(fp,"<span class=\"numeric_constant\">%s</span>", event->data);
            break;
        case PEVENT_RESERVE_KEYWORD:
            if(event->property == RES_KEYWORD_DATA)
            {
                fprintf(fp,"<span class=\"reserved_key1\">%s</span>", event->data);
            }
            else
            {
                fprintf(fp,"<span class=\"reserved_key2\">%s</span>", event->data);
            }
            break;
        case PEVENT_ASCII_CHAR:
            fprintf(fp,"<span class=\"ascii_char\">%s</span>", event->data);
            break;
        default:
            printf("Unknown event\n");
            break;
    }
}

