Source Code to HTML Conversion Tool

Overview

The Source Code to HTML Conversion Tool is a C-based program that reads a source code file, parses its content, and generates an HTML file. It provides an efficient way to display source code with syntax highlighting or formatting for web platforms, catering to developers and educators.

Features

Input and Output:

Reads source files (e.g., .c, .txt) and generates .html files.

Default output filename derived from input with a .html extension or customizable via command-line arguments.

Error Handling:

Displays detailed error messages for file access or input issues.

Extensible Design:

Modular architecture allows easy updates to parsing or HTML generation logic.

HTML Syntax Highlighting:

Automatically wraps code elements (e.g., keywords, strings, comments) with appropriate HTML tags.

File Structure

1. s2html_main.c

Entry point of the program.

Handles command-line arguments, file I/O, and overall program flow.

Delegates parsing and HTML conversion tasks to modular components.

2. s2html_event.h / s2html_event.c

Defines and implements the parsing logic for source code files.

Extracts components like keywords, strings, comments, and more as pevent_t structures.

3. s2html_conv.h / s2html_conv.c

Contains functions to translate parsed events into HTML elements.

Adds appropriate HTML tags for formatting code elements.

Key Functions

html_begin(FILE *dfp, const char *open_tag)

Adds the starting tags to the HTML file.

html_end(FILE *dfp, const char *close_tag)

Adds the ending tags to the HTML file.

get_parser_event(FILE *sfp)

Reads the next part of the source file (like keywords, strings, or comments).

source_to_html(FILE *dfp, pevent_t *event)

Converts the parsed part of the source file into HTML and writes it to the output file.

Usage

Run the program using the following syntax:

$ ./s2html <source_file> [output_file_prefix]

Compilation

Compile the program using:

$ gcc s2html_main.c s2html_event.c s2html_conv.c -o s2html -I.

Running the Program

Run the executable with the required arguments:

Convert a file to HTML:

$ ./s2html test.c

Output: test.c.html

Specify a custom output file name:

$ ./s2html test.txt output_file

Output: output_file.html

Example Code

Using test.c and test.txt as inputs:

$ ./s2html test.c
Output: test.c.html

$ ./s2html test.txt custom_output
Output: custom_output.html

Debug Mode
