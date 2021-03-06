//Marc Roman Colom y Laura Cavero Loza
#include "nivel2.h"

char prompt[COMMAND_LINE_SIZE];
char *execHasMade = AMARILLO_T;
int main(){
    char line[COMMAND_LINE_SIZE]; //1024
    while (1){
        if (read_line(line) && strlen(line) > 0){
            execute_line(line);                     //Hacer checking de si la instr. se ejecutó
        }
        execHasMade = VERDEF_T;
    }
    return 1;
}

/**
 * Devuelve el puntero de PWD con el string modificado, indicando en que directorio estas.
 * */
void getDirectory(){
    char *fullPath = getenv("PWD");
    char result[COMMAND_LINE_SIZE];
    getcwd(prompt, COMMAND_LINE_SIZE);
    size_t lenPath = strlen(fullPath);
    size_t cnt = lenPath;
    size_t i = 0;
    --cnt;
    while (*(fullPath+cnt) != '/' && cnt > 0){  //Busca el incio del directorio actual
        --cnt;
    }
    if (*(fullPath+cnt) == '/'){                //Copia el directorio actual
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
}

/**
 * Imprime el prompt.
 * */
void imprimir_prompt(){
    getDirectory();
    printf("%s»"RESET_COLOR VERDE_T "%s " AZUL_T "%c" RESET_COLOR ": ",execHasMade, prompt, PROMPT);
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

/**
 * Ejecuta la línea de comando, siendo un comando interno o externo.
 * */
int execute_line(char *line){
    char *args[ARGS_SIZE];                      //Array de punteros de tokens
    int numOfargss = parse_args(args, line);    //Troceamos la linea de cmd y lo almacenamos en el puntero de tokens
    if (numOfargss > 0){                        //Si hay tokens, tratamos de ejecutarlo
        size_t i = 0;                               //TEMP
        while (*(args + i) != NULL){                //TEMP
            printf("[%ld]%s\t", i, *(args + i));    //TEMP
            i++;                                    //TEMP
        }
        printf("[%ld]%s\t\n", i, *(args + i));      //TEMP
        check_internal(args);
    }
    return 0;
}

/**
 * Procesa la linea de comando escrita, troceandola por tokens y devuelve el numero de tokens.
 * */
int parse_args(char **args, char *line){
    int numOfargss = 0;
    char *tempToken;
    tempToken = strtok(line, DELIMITERS);
    while (tempToken != NULL && *(tempToken) != HASH){
        *(args + numOfargss) = tempToken;
        tempToken = strtok(NULL, DELIMITERS);
        numOfargss++;
    }
    *(args +numOfargss) = NULL;
    return numOfargss;
}
/**
 * Verifica si el cmd es interno o no, y en caso de serlo, lo ejecuta.
 * */
int check_internal(char **args){
    char *intern_cmd[] = {"exit","cd","export","source","jobs","fg","bg"};  //Array de comandos internos
    int n_of_cmds = 7;                                                      //Tamaño del array
    int is_not_internal = 0;
    for (size_t i = 0; i < n_of_cmds; i++){
        is_not_internal = strcmp(args[0],intern_cmd[i]);
        if (!is_not_internal){
            switch (i){
                case 0:
                    exit(0);
                    break;
                case 1:
                    internal_cd(args);
                    break;
                case 2:
                    internal_export(args);
                    break;
                case 3:
                    internal_source(args);
                    break;
                case 4:
                    internal_jobs(args);
                    break;
                case 5:
                    internal_fg(args);
                    break;
                case 6:
                    internal_bg(args);
            }
            return 1;
        }
        
    }
    if (is_not_internal){
        printf("\nNo es un cmd interno\n");
    }
    return 0;
}

/**
 * Cambia el directorio en el cual se haya el usuario.
 * */
int internal_cd(char **args){ //TESTING TIME
    char path[PATHSIZE];
    if (args[1] != NULL){
        char *hasDelimiters;
        if((hasDelimiters = strtok(args[1],ADVCD))){      //Observamos si contiene algun delimitador
            strcpy(path,hasDelimiters);                 //Copiamos la palabra al path
            if ((hasDelimiters = strtok(NULL,ADVCD))){    //En caso de que la palabra contenga algun otro delimitador, concatenamos 
                                                        //su contenido con lo que hay en el path.
                strcat(path, hasDelimiters);
            }
            advanced_cd(path,args);                     //Pasamos a advanced_cd, donde lee el resto
        }
    }else{
        strcpy(path,getenv("HOME"));
    }
    if (chdir(path)){                                   //Intentamos cambiar el path.
        perror("chdir :");
        printf("%s\n",path);
    }
    return 1;
}
/**
 * Funcion complementaria a internal_cd que concatena los tokens 
 * */
void advanced_cd(char *path,char **args){
    size_t cntArgs = 2;
    char *word;
    while (args[cntArgs] != NULL){                  //Mientras no lleguemos al final de los tokens
        word = strtok(args[cntArgs], ADVCD);        //
        strcat(path," ");                           //Concatenamos el nombre del directorio sin los delimitadores al path.
        strcat(path, word);                         //
        word = strtok(NULL,ADVCD);                  //
        while(word){                                //En caso de que un token contenga mas palabras en el token despues
            strcat(path,word);                      //del delimitador, lo concatenamos al path.
            word = strtok(NULL,ADVCD);              //
        }
        cntArgs++;
    }
}

/**
 * Muestra todas las variables de entorno si no tiene parametros.
 * En caso de tener parametros, modifica o crea una variable de entorno y le añade contenido. 
 * */
int internal_export(char **args){
    char *value;
    char *name;
    if (args[1] != NULL){
        name = strtok(args[1], EQUAL);
        if (name == NULL){
            name = "";
        }
        value = strtok(NULL," ");
        if (value == NULL){
            value = "";
        }
    }
    printf("Before: %s=%s",name,getenv(name));
    if (strcmp(value, "") == 0 || strcmp(name, "") == 0 || setenv(name, value, 1)){
        fprintf(stderr, "Error de sintaxis. Uso: Nombre=Valor\n");
        execHasMade = ROJOF_T; //Cambiamos el color del prompt para notificar que ha fallado su ejecución
    }
    printf("After: %s=%s",name, getenv(name));
    return 1;
}

/**
 * */
int internal_source(char **args){
    puts("La funcion source llama a un script para ejecutarlo seguidamente");
    return 1;
}

/**
 * */
int internal_jobs(char **args){
    puts("La funcion jobs muestra los procesos que hay");
    return 1;
}

/**
 * */
int internal_fg(char **args){
    puts("La funcion fg pone el proceso en primer plano");
    return 1;
}

/**
 * */
int internal_bg(char **args){
    puts("La funcion bg pone el proceso en segundo plano, dejando libre la terminal para uso de otra funcion");
    return 1;
}