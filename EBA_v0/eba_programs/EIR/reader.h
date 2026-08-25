#ifndef _INC_READER_
#define _INC_READER_

int samestr(char *a, char *b);
int is_label(char *a);

char *** full_read(char *fname);
char ** line_to_words(char *line);
void full_free(char ***IRcode);
void print_code(char ***IRcode);

void labels_to_lines(char ***IRcode);



#endif
