
Source Code to HTML Conversion Tool

Introduction

The Source Code to HTML Conversion Tool is a C-based program that reads a source code file and generates an HTML file. It converts the code into a well-formatted HTML document with syntax highlighting, making it easier to present and share code online. This tool supports multiple source code file types like .c, .cpp, and .txt, and is perfect for developers, educators, and anyone needing to display code in a user-friendly format.

Key Features

- Syntax Highlighting: Automatically formats code elements (keywords, strings, comments) with HTML tags.
- Input Flexibility: Supports different file types (e.g., .c, .txt) as input.
- Custom Output: Allows specifying a custom output file name.
- Modular Design: Easy to extend for future updates or additional features.
- Error Handling: Provides informative error messages for file issues.


Usage:

Compilation

To compile the program, run:

>> gcc s2html_main.c s2html_event.c s2html_conv.c -o s2html -I.

Running the Program

Run the program with the following command:

>> ./s2html <source_file> [output_file_prefix]

- Convert a file to HTML:

>> ./s2html test.c

- Specify a custom output file name:

>> ./s2html test.txt output_file
