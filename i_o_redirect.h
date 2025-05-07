#ifndef I_O_REDIRECT_H
#define I_O_REDIRECT_H

int redirection(char* instruct);
void output(char filename[]);
void input(char filename[]);
char* parse_redirect(char* instruct);

#endif