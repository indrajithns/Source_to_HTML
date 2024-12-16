/*
 *  Event Management for Source-to-HTML Analyzer Function
 *
 * This program analyzes a C source code file to identify keywords, comments, 
 * constants, operators, and strings. It processes the file character by 
 * character and generates events for each identified element.
 *
 * Main features:
 * - Identifies comments, keywords, constants, strings, and operators.
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "s2html_event.h"

#define SIZE_OF_SYMBOLS (sizeof(symbols))
#define SIZE_OF_OPERATORS (sizeof(operators))
#define WORD_BUFF_SIZE	100

/********** Internal states and event of parser **********/
typedef enum
{
	PSTATE_IDLE,
	PSTATE_PREPROCESSOR_DIRECTIVE,
	PSTATE_SUB_PREPROCESSOR_MAIN,
	PSTATE_SUB_PREPROCESSOR_RESERVE_KEYWORD,
	PSTATE_SUB_PREPROCESSOR_ASCII_CHAR,
	PSTATE_HEADER_FILE,
	PSTATE_RESERVE_KEYWORD,
	PSTATE_NUMERIC_CONSTANT,
	PSTATE_STRING,
	PSTATE_SINGLE_LINE_COMMENT,
	PSTATE_MULTI_LINE_COMMENT,
	PSTATE_ASCII_CHAR
}pstate_e;

/********** global variables **********/

/* parser state variable */
static pstate_e state = PSTATE_IDLE;

/* sub state is used only in preprocessor state */
static pstate_e state_sub = PSTATE_SUB_PREPROCESSOR_MAIN;

/* event variable to store event and related properties */
static pevent_t pevent_data;
static int event_data_idx=0;

static char word[WORD_BUFF_SIZE];
static int word_idx=0;


static char* res_kwords_data[] = {"const", "volatile", "extern", "auto", "register",
   						   "static", "signed", "unsigned", "short", "long", 
						   "double", "char", "int", "float", "struct", 
						   "union", "enum", "void", "typedef", ""
						  };

static char* res_kwords_non_data[] = {"goto", "return", "continue", "break", 
							   "if", "else", "for", "while", "do", 
							   "switch", "case", "default","sizeof", ""
							  };

static char operators[] = {'/', '+', '*', '-', '%', '=', '<', '>', '~', '&', ',', '!', '^', '|'};
static char symbols[] = {'(', ')', '{', '[', ':'};

/********** state handlers **********/
pevent_t * pstate_idle_handler(FILE *fd, int ch);
pevent_t * pstate_single_line_comment_handler(FILE *fd, int ch);
pevent_t * pstate_multi_line_comment_handler(FILE *fd, int ch);
pevent_t * pstate_numeric_constant_handler(FILE *fd, int ch);
pevent_t * pstate_string_handler(FILE *fd, int ch);
pevent_t * pstate_header_file_handler(FILE *fd, int ch);
pevent_t * pstate_ascii_char_handler(FILE *fd, int ch);
pevent_t * pstate_reserve_keyword_handler(FILE *fd, int ch);
pevent_t * pstate_preprocessor_directive_handler(FILE *fd, int ch);
pevent_t * pstate_sub_preprocessor_main_handler(FILE *fd, int ch);

/********** Utility functions **********/

/* function to check if given word is reserved key word */
static int is_reserved_keyword(char *word)
{
	int idx = 0;

	/* search for data type reserved keyword */
	while(*res_kwords_data[idx])
	{
		if(strcmp(res_kwords_data[idx++], word) == 0)
			return RES_KEYWORD_DATA;
	}

	idx = 0; // reset index
	/* search for non data type reserved key word */
	while(*res_kwords_non_data[idx])
	{
		if(strcmp(res_kwords_non_data[idx++], word) == 0)
			return RES_KEYWORD_NON_DATA;
	}

	return 0; // word did not match, return false
}

/* function to check symbols */
static int is_symbol(char c)
{
	int idx;
	for(idx = 0; idx < SIZE_OF_SYMBOLS; idx++)
	{
		if(symbols[idx] == c)
			return 1;
	}

	return 0;
}

/* function to check operator */
static int is_operator(char c)
{
	int idx;
	for(idx = 0; idx < SIZE_OF_OPERATORS; idx++)
	{
		if(operators[idx] == c)
			return 1;
	}

	return 0;
}

/* to set parser event */
static void set_parser_event(pstate_e s, pevent_e e)
{
	pevent_data.data[event_data_idx] = '\0';
	pevent_data.length = event_data_idx;
	event_data_idx = 0;
	state = s;
	pevent_data.type = e;
}


/************ Event functions **********/

/* This function parses the source file and generate 
 * event based on parsed characters and string
 */
pevent_t *get_parser_event(FILE *fd)
{
	int ch, pre_ch;
	pevent_t *evptr = NULL;
	/* Read char by char */
	while((ch = fgetc(fd)) != EOF)
	{
#ifdef DEBUG
	//	putchar(ch);
#endif
		switch(state)
		{
			case PSTATE_IDLE :
				if((evptr = pstate_idle_handler(fd, ch)) != NULL)
					return evptr;
				break;
			case PSTATE_SINGLE_LINE_COMMENT :
				if((evptr = pstate_single_line_comment_handler(fd, ch)) != NULL)
					return evptr;
				break;
			case PSTATE_MULTI_LINE_COMMENT :
				if((evptr = pstate_multi_line_comment_handler(fd, ch)) != NULL)
					return evptr;
				break;
			case PSTATE_PREPROCESSOR_DIRECTIVE :
				if((evptr = pstate_preprocessor_directive_handler(fd, ch)) != NULL)
					return evptr;
				break;
			case PSTATE_RESERVE_KEYWORD :
				if((evptr = pstate_reserve_keyword_handler(fd, ch)) != NULL)
					return evptr;
				break;
			case PSTATE_NUMERIC_CONSTANT :
				if((evptr = pstate_numeric_constant_handler(fd, ch)) != NULL)
					return evptr;
				break;
			case PSTATE_STRING :
				if((evptr = pstate_string_handler(fd, ch)) != NULL)
					return evptr;
				break;
			case PSTATE_HEADER_FILE :
				if((evptr = pstate_header_file_handler(fd, ch)) != NULL)
					return evptr;
				break;
			case PSTATE_ASCII_CHAR :
				if((evptr = pstate_ascii_char_handler(fd, ch)) != NULL)
					return evptr;
				break;
			default : 
				printf("unknown state\n");
				state = PSTATE_IDLE;
				break;
		}
	}

	/* end of file is reached, move back to idle state and set EOF event */
	set_parser_event(PSTATE_IDLE, PEVENT_EOF);

	return &pevent_data; // return final event
}


/********** IDLE state Handler **********
 * Idle state handler identifies
 ****************************************/

pevent_t * pstate_idle_handler(FILE *fd, int ch)
{
	int pre_ch;
	switch(ch)
	{
		case '\'' : // begining of ASCII char 
			break;

		case '/' :
			pre_ch = ch;
			if((ch = fgetc(fd)) == '*') // multi line comment
			{
				if(event_data_idx) // we have regular exp in buffer first process that
				{
					fseek(fd, -2L, SEEK_CUR); // unget chars
					set_parser_event(PSTATE_IDLE, PEVENT_REGULAR_EXP);
					return &pevent_data;
				}
				else //	multi line comment begin 
				{
#ifdef DEBUG	
					printf("Multi line comment Begin : /*\n");
#endif
					state = PSTATE_MULTI_LINE_COMMENT;
					pevent_data.data[event_data_idx++] = pre_ch;
					pevent_data.data[event_data_idx++] = ch;
				}
			}
			else if(ch == '/') // single line comment
			{
				if(event_data_idx) // we have regular exp in buffer first process that
				{
					fseek(fd, -2L, SEEK_CUR); // unget chars
					set_parser_event(PSTATE_IDLE, PEVENT_REGULAR_EXP);
					return &pevent_data;
				}
				else //	single line comment begin
				{
#ifdef DEBUG	
					printf("Single line comment Begin : //\n");
#endif
					state = PSTATE_SINGLE_LINE_COMMENT;
					pevent_data.data[event_data_idx++] = pre_ch;
					pevent_data.data[event_data_idx++] = ch;
				}
			}
			else // it is regular exp
			{
				pevent_data.data[event_data_idx++] = pre_ch;
				pevent_data.data[event_data_idx++] = ch;
			}
			break;
		case '#' :

			break;
		case '\"' :
			break;

		case '0' ... '9' : // detect numeric constant
			break;

		case 'a' ... 'z' : // could be reserved key word

			break;
		default : // Assuming common text starts by default.
			pevent_data.data[event_data_idx++] = ch;
			break;
	}

	return NULL;
}
pevent_t * pstate_preprocessor_directive_handler(FILE *fd, int ch)
{
	int tch;
	switch(state_sub)
	{
		case PSTATE_SUB_PREPROCESSOR_MAIN :
			return pstate_sub_preprocessor_main_handler(fd, ch);
		case PSTATE_SUB_PREPROCESSOR_RESERVE_KEYWORD :
			return pstate_reserve_keyword_handler(fd, ch);
		case PSTATE_SUB_PREPROCESSOR_ASCII_CHAR :
			return pstate_ascii_char_handler(fd, ch);
		default :
				printf("unknown state\n");
				state = PSTATE_IDLE;
	}

	return NULL;
}

//pevent_t * pstate_sub_preprocessor_main_handler(FILE *fd, int ch)
//{
	/* write a switch case here to detect several events here
	 * This state is similar to Idle state with slight difference
	 * in state transition.
	 * return event data at the end of event
	 * else return NULL
	 */
//}

pevent_t *pstate_sub_preprocessor_main_handler(FILE *fd, int ch) 
{
    // Use a buffer to hold the current word being read
    static char word[WORD_BUFF_SIZE];
    static int word_idx = 0;

    // Clear the word buffer if we're starting a new event
    if (word_idx == 0) {
        memset(word, 0, sizeof(word));
    }

    // Store the character in the word buffer
    if (isalpha(ch) || ch == '_') { // Start of a new word
        word[word_idx++] = ch; // Store valid characters (letters and underscores)
        return NULL; // Continue accumulating characters
    } else if (isspace(ch) || ch == '\n') {
        // If we hit a space or new line, check if we've finished a word
        if (word_idx > 0) {
            word[word_idx] = '\0'; // Null-terminate the string

            // Check for specific preprocessor directives
            if (strcmp(word, "define") == 0) {
                set_parser_event(PSTATE_SUB_PREPROCESSOR_MAIN, PEVENT_PREPROCESSOR_DIRECTIVE);
                state = PSTATE_PREPROCESSOR_DIRECTIVE; // Transition to the preprocessor directive state
            } else if (strcmp(word, "include") == 0) {
                set_parser_event(PSTATE_SUB_PREPROCESSOR_MAIN, PEVENT_HEADER_FILE);
                state = PSTATE_HEADER_FILE; // Transition to the header file state
            } else {
                // Handle other preprocessor keywords if necessary
                set_parser_event(PSTATE_SUB_PREPROCESSOR_MAIN, PEVENT_RESERVE_KEYWORD);
                state = PSTATE_RESERVE_KEYWORD; // Transition to reserve keyword state
            }

            word_idx = 0; // Reset the word index for the next word
            return &pevent_data; // Return the current event
        }
    } else {
        // If we encounter any other character, it may be part of a directive
        if (word_idx > 0) {
            ungetc(ch, fd); // Push character back to the stream
            word[word_idx] = '\0'; // Null-terminate the string

            // Check for any other conditions you want to capture here
            // This is also where you might handle unexpected characters.
        }
    }

    return NULL; // If no complete event is formed, return NULL
}






//pevent_t * pstate_header_file_handler(FILE *fd, int ch)
//{
	/* write a switch case here to store header file name
	 * return event data at the end of event
	 * else return NULL
	 */
//}

pevent_t *pstate_header_file_handler(FILE *fd, int ch) 
{
    // Use a buffer to hold the header file name
    static char header_file_name[PEVENT_DATA_SIZE];
    static int index = 0;
    static int is_quoted = 0; // To check if it's enclosed in quotes

    // Clear the buffer if we are starting a new header file
    if (index == 0) {
        memset(header_file_name, 0, sizeof(header_file_name));
    }

    // Check for the opening quote or angle bracket
    if (ch == '"' || ch == '<') {
        is_quoted = (ch == '"'); // Store if it's quoted
        return NULL; // Continue waiting for the filename
    } else if (ch == '\n' || ch == EOF) {
        // End of file or line before closing quote or angle bracket
        // Invalid case: we should return NULL, as we did not find a header file name
        return NULL;
    } else if ((is_quoted && ch == '"') || (!is_quoted && ch == '>')) {
        // Closing quote or angle bracket found, end of filename
        header_file_name[index] = '\0'; // Null-terminate the string
        
        // Set the event data
        pevent_data.type = PEVENT_HEADER_FILE; // Event type for header file
        pevent_data.property = USER_HEADER_FILE; // Assuming it is a user header
        pevent_data.length = index; // Length of the filename
        strncpy(pevent_data.data, header_file_name, PEVENT_DATA_SIZE - 1); // Copy the filename
        
        index = 0; // Reset index for the next file
        return &pevent_data; // Return populated event
    } else {
        // Accumulate characters for the header file name
        if (index < PEVENT_DATA_SIZE - 1) { // Ensure we don't overflow the buffer
            header_file_name[index++] = ch; // Store the character
        }
    }

    return NULL; // If we are not finished, return NULL
}










//pevent_t * pstate_reserve_keyword_handler(FILE *fd, int ch)
//{
	/* write a switch case here to store words
	 * return event data at the end of event
	 * else return NULL
	 */
//}


pevent_t *pstate_reserve_keyword_handler(FILE *fd, int ch) 
{
    static char keyword[PEVENT_DATA_SIZE]; // Buffer to store the keyword
    static int index = 0; // Current index in the keyword buffer

    // Check if the character is a valid part of a keyword (alphanumeric or underscore)
    if (isalnum(ch) || ch == '_') {
        // Ensure we don't overflow the buffer
        if (index < PEVENT_DATA_SIZE - 1) {
            keyword[index++] = ch; // Add character to keyword buffer
            return NULL; // Continue reading characters
        } else {
            // If buffer is full, we can't process this keyword
            return NULL; // or handle an error
        }
    }

    // If we reached here, we encountered a delimiter or a non-keyword character
    // Null-terminate the string
    keyword[index] = '\0'; 

    // Reset index for the next keyword
    index = 0; 

    // Check if the accumulated string is a reserved keyword
    for (int i = 0; i < sizeof(res_kwords_data) / sizeof(res_kwords_data[0]); i++) {
        if (strcmp(keyword, res_kwords_data[i]) == 0) {
            // We found a reserved keyword
            pevent_data.type = PEVENT_RESERVE_KEYWORD; // Event type
            pevent_data.property = RES_KEYWORD_DATA; // Set the property type
            pevent_data.length = strlen(keyword); // Set the length of the keyword
            strncpy(pevent_data.data, keyword, PEVENT_DATA_SIZE - 1); // Copy the keyword into event data

            return &pevent_data; // Return the populated event
        }
    }

    // If it's not a reserved keyword, return NULL
    return NULL; 
}







//pevent_t * pstate_numeric_constant_handler(FILE *fd, int ch)
//{
	/* write a switch case here to store digits
	 * return event data at the end of event
	 * else return NULL
	 */
//}

pevent_t *pstate_numeric_constant_handler(FILE *fd, int ch) 
{
    static char number[PEVENT_DATA_SIZE]; // Buffer to store the numeric constant
    static int index = 0; // Current index in the number buffer

    // Check if the character is a digit or a decimal point
    if (isdigit(ch) || (ch == '.' && index > 0 && index < PEVENT_DATA_SIZE - 1 && strchr(number, '.') == NULL)) {
        if (index < PEVENT_DATA_SIZE - 1) {
            number[index++] = ch; // Add character to number buffer
            return NULL; // Continue reading characters
        } else {
            // Buffer overflow case (not expected in normal operation)
            return NULL; // or handle an error
        }
    }

    // If we reached here, we encountered a non-numeric character
    // Null-terminate the string
    number[index] = '\0'; 

    // Reset index for the next number
    index = 0; 

    // Populate the pevent_t structure with the numeric constant
    pevent_data.type = PEVENT_NUMERIC_CONSTANT; // Event type
    pevent_data.property = RES_KEYWORD_DATA; // You may adjust this depending on your requirements
    pevent_data.length = strlen(number); // Set the length of the number
    strncpy(pevent_data.data, number, PEVENT_DATA_SIZE - 1); // Copy the number into event data

    return &pevent_data; // Return the populated event
}









//pevent_t * pstate_string_handler(FILE *fd, int ch)
//{
	/* write a switch case here to store string
	 * return event data at the end of event
	 * else return NULL
	 */
//}

pevent_t *pstate_string_handler(FILE *fd, int ch) 
{
    static char str_buffer[PEVENT_DATA_SIZE]; // Buffer to store the string
    static int index = 0; // Current index in the string buffer
    int is_escaped = 0; // Flag to check if the previous character was an escape character

    // Check if the character is the end quote of the string
    if (ch == '"' && !is_escaped) {
        // Null-terminate the string
        str_buffer[index] = '\0';

        // Reset index for the next string
        index = 0;

        // Populate the pevent_t structure with the string
        pevent_data.type = PEVENT_STRING; // Event type
        pevent_data.property = RES_KEYWORD_DATA; // Set the property if needed
        pevent_data.length = strlen(str_buffer); // Set the length of the string
        strncpy(pevent_data.data, str_buffer, PEVENT_DATA_SIZE - 1); // Copy the string into event data

        return &pevent_data; // Return the populated event
    }

    // Check for escape character
    if (ch == '\\' && !is_escaped) {
        is_escaped = 1; // Set the escape flag
        return NULL; // Continue reading characters
    }

    // Add the character to the string buffer if there's space
    if (index < PEVENT_DATA_SIZE - 1) {
        str_buffer[index++] = ch; // Add character to the buffer
        is_escaped = 0; // Reset escape flag
    } else {
        // Handle buffer overflow (you can add error handling)
    }

    return NULL; // Continue reading until end of string is detected
}








pevent_t * pstate_single_line_comment_handler(FILE *fd, int ch)
{
	int pre_ch;
	switch(ch)
	{
		case '\n' : /* single line comment ends here */
#ifdef DEBUG	
			printf("\nSingle line comment end\n");
#endif
			pre_ch = ch;
			pevent_data.data[event_data_idx++] = ch;
			set_parser_event(PSTATE_IDLE, PEVENT_SINGLE_LINE_COMMENT);
			return &pevent_data;
		default :  // collect single line comment chars
			pevent_data.data[event_data_idx++] = ch;
			break;
	}

	return NULL;
}
pevent_t * pstate_multi_line_comment_handler(FILE *fd, int ch)
{
	int pre_ch;
	switch(ch)
	{
		case '*' : /* comment might end here */
			pre_ch = ch;
			pevent_data.data[event_data_idx++] = ch;
			if((ch = fgetc(fd)) == '/')
			{
#ifdef DEBUG	
				printf("\nMulti line comment End : */\n");
#endif
				pre_ch = ch;
				pevent_data.data[event_data_idx++] = ch;
				set_parser_event(PSTATE_IDLE, PEVENT_MULTI_LINE_COMMENT);
				return &pevent_data;
			}
			else // multi line comment string still continued
			{
				pevent_data.data[event_data_idx++] = ch;
			}
			break;
		case '/' :
			/* go back by two steps and read previous char */
			fseek(fd, -2L, SEEK_CUR); // move two steps back
			pre_ch = fgetc(fd); // read a char
			fgetc(fd); // to come back to current offset

			pevent_data.data[event_data_idx++] = ch;
			if(pre_ch == '*')
			{
				set_parser_event(PSTATE_IDLE, PEVENT_MULTI_LINE_COMMENT);
				return &pevent_data;
			}
			break;
		default :  // collect multi-line comment chars
			pevent_data.data[event_data_idx++] = ch;
			break;
	}

	return NULL;
}
//pevent_t * pstate_ascii_char_handler(FILE *fd, int ch)
//{
	/* write a switch case here to store ASCII chars
	 * return event data at the end of event
	 * else return NULL
	 */
//}

pevent_t *pstate_ascii_char_handler(FILE *fd, int ch) {
    // Check if the character is a valid ASCII character
    if (ch >= 0 && ch <= 127) {
        // Populate the pevent_t structure
        pevent_data.type = PEVENT_ASCII_CHAR;  // Set the event type
        pevent_data.property = RES_KEYWORD_DATA; // You can set this based on context
        pevent_data.length = 1;  // Length is 1 since we are handling a single character
        pevent_data.data[0] = (char)ch;  // Store the ASCII character in the data array
        pevent_data.data[1] = '\0';  // Null-terminate the string for safety

        return &pevent_data;  // Return the populated event
    }

    return NULL;  // Return NULL if the character is not a valid ASCII character
}



/**** End of file ****/
