/*
 * Header for converting source code to HTML format.
 *
 * Constants:
 * - HTML_OPEN: Marks opening HTML tags.
 * - HTML_CLOSE: Marks closing HTML tags.
 *
 * Functions:
 * - html_begin: Adds opening HTML tags.
 * - html_end: Adds closing HTML tags.
 * - source_to_html: Converts source code to HTML and writes it.
*/

#ifndef S2HTML_CONV_H
#define S2HTML_CONV_H

#define HTML_OPEN	1
#define HTML_CLOSE	0

/********** function prototypes **********/

void html_begin(FILE *dest_fp, int type); // Adds the opening HTML tags to the file.
void html_end(FILE *dest_fp, int type);   // Adds the closing HTML tags to the file.
void source_to_html(FILE *fp, pevent_t *event); // Converts source code events to HTML format and writes to the file.

#endif


