#include "nivel1.h"

int main(){
    char line[COMMAND_LINE_SIZE]; //1024
    while (1){
        if (read_line(line) && strlen(line) > 0){
            execute_line(line);
        }
        
    }
    return 1;
}

char *read_line(char *line){
    fprintf("%c ",PROMPT);
    fflush(stdout);
}
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs(char **args);
int internal_fg(char **args);
int internal_bg(char **args);