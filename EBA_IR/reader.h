#ifndef _INC_READER_
#define _INC_READER_


char *** full_read(char *fname);
char ** line_to_words(char *line);
void full_free(char ***IRcode);
void print_code(char ***IRcode);



#endif
