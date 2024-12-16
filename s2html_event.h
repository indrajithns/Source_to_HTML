/*
 * Header for Event Management for Source Code Analyzer
 *
 * This file defines structures and constants for managing events during source code parsing.
 * It supports events like preprocessor directives, keywords, constants, strings, and comments.
 *
 * Constants:
 * - Event types and the maximum size for event data.
 *
 * Enum (pevent_e):
 * - Defines various event types the parser can detect.
 *
 * Structure (pevent_t):
 * - Holds event details such as type, properties, and content.
 *
 * Functions:
 * - get_parser_event: Fetches the next event from the file.
 */

#ifndef S2HTML_EVENT_H
#define S2HTML_EVENT_H

#define USER_HEADER_FILE		1
#define STD_HEADER_FILE			2
#define RES_KEYWORD_DATA		3
#define RES_KEYWORD_NON_DATA	4

#define PEVENT_DATA_SIZE	1024

typedef enum
{
	PEVENT_NULL,
	PEVENT_PREPROCESSOR_DIRECTIVE,
	PEVENT_RESERVE_KEYWORD,
	PEVENT_NUMERIC_CONSTANT,
	PEVENT_STRING,
	PEVENT_HEADER_FILE,
	PEVENT_REGULAR_EXP,
	PEVENT_SINGLE_LINE_COMMENT,
	PEVENT_MULTI_LINE_COMMENT,
	PEVENT_ASCII_CHAR,
	PEVENT_EOF
} pevent_e;

typedef struct
{
	pevent_e type;      // event type
	int property;        // property associated with data
	int length;          // data length
	char data[PEVENT_DATA_SIZE];  // parsed string data
} pevent_t;

/********** function prototypes **********/

pevent_t *get_parser_event(FILE *fp);

#endif
/**** End of file ****/
