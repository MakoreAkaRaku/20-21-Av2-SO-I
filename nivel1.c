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

/**
 * Devuelve el puntero de PWD con el string modificado, indicando en que directorio estas.
 * */
char *getDirectory(){
    char *fullPath = getenv("PWD");
    char result[COMMAND_LINE_SIZE];
    size_t lenPath = strlen(fullPath);
    size_t cnt = lenPath;
    size_t i = 0;
    --cnt;
    while (*(fullPath+cnt) != '/' && cnt > 0){
        --cnt;
    }
    if (*(fullPath+cnt) == '/'){
        cnt++;
        while (cnt < strlen(fullPath))
        {
            result[i] = *(fullPath + cnt);
            cnt++;
            i++;
        }
        result[i] = '\0';
        strcpy(fullPath, result);
    }
    return fullPath;
}

/**
 * Imprime el prompt.
 * */
void imprimir_prompt(){
    char *path = getDirectory();
    printf("%s»"RESET_COLOR VERDE_T "%s " AZUL_T "%c" RESET_COLOR ": ",execHasMade, path, PROMPT);
    fflush(stdout);
}
/**
 * Lee la linea de comandos e imprime el prompt.
 * */
char *read_line(char *line){
    imprimir_prompt();
    fgets(line, COMMAND_LINE_SIZE, stdin);
    if (line == NULL){ //Looking for condition
        puts("readline: error al leer el string introducido");
        return NULL;
    }
    return line;
}

int execute_line(char *line){
    char *token[COMMAND_LINE_SIZE];
    int numOfTokens = parse_args(token, line);
    size_t i = 0;
    while (*(token+i) != NULL){
        printf("[%ld]%s\t", i, *(token + i));
        i++;
    }
    printf("[%ld]%s\t", i, *(token + i));
    //int isinternal = check_internal(token);
    return 0;
}

int parse_args(char **args, char *line){
    int numOfTokens = 0;
    char *tempToken;
    tempToken = strtok(line, DELIMITERS);
    while (tempToken != NULL && *(tempToken) != HASH){
        *(args + numOfTokens) = tempToken;
        tempToken = strtok(NULL, DELIMITERS);
        numOfTokens++;
    }
    *(args +numOfTokens) = NULL;
    return numOfTokens;
}

int check_internal(char **args);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs(char **args);
int internal_fg(char **args);
int internal_bg(char **args);