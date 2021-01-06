//Marc Roman Colom y Laura Cavero Loza
#include "nivel3.h"

char prompt[COMMAND_LINE_SIZE];
char *execHasMade = AMARILLO_T;
int main(){
    char line[COMMAND_LINE_SIZE]; //1024
    while (1){
        if (read_line(line) && strlen(line) > 0){
            if(execute_line(line)){
                execHasMade = ROJOF_T;
            }else{
                execHasMade = VERDEF_T;
            }
        }
    }
    return 1;
}

/**
 * Devuelve el puntero de PWD con el string modificado, indicando en que directorio estas.
 * */
void getDirectory(){
    getcwd(prompt, COMMAND_LINE_SIZE);
    int plength = strlen(prompt);
    int actdirect = plength-1;
    while (*(prompt + actdirect) != '/' && actdirect > 0){
        actdirect--;
    }
    if (*(prompt + actdirect) == '/'){
        actdirect++;
        strcpy(prompt,(prompt+actdirect));
    }
}

/**
 * Imprime el prompt.
 * */
void imprimir_prompt(){
    getDirectory();
    printf("%s➤ " RESET_COLOR VERDE_T "%s " AZUL_T "%c" RESET_COLOR ": ", execHasMade, prompt, PROMPT);
    fflush(stdout);
}

/**
 * Lee la linea de comandos e imprime el prompt.
 * */
char *read_line(char *line){
    imprimir_prompt();
    if(!fgets(line, COMMAND_LINE_SIZE, stdin)){
        if (feof(stdin)){
            puts("\nCTRL+D pressed");
            exit(0);
        }
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
        if(!check_internal(args)){
            int status;
            pid_t cpid = fork();
            if (cpid < 0){
                perror("fork ");
            }else if(cpid > 0){
                printf("Soy el padre y mi pid es:%d\n", getpid());
                while (wait(&status) != cpid);
                if(WIFEXITED(status)){
                    printf("\nchild exit %d\n",WEXITSTATUS(status));
                    return(WEXITSTATUS(status));
                }
                else if (WIFSIGNALED(status)){
                    printf("\nchild process finalized with a signal: %d", WTERMSIG(status));
                }
            }else{
                printf("Soy el hijo y mi pid es: %d\n", getpid());
                if (execvp(args[0], args)){
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
                exit(EXIT_SUCCESS);
            }
            
        }
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
    return 0;
}

/**
 * Cambia el directorio en el cual se haya el usuario.
 * */
int internal_cd(char **args){ //TESTING TIME
    char path[PATHSIZE];
    if (args[1] != NULL){
        advanced_cd(path,args);
    }else{
        strcpy(path,getenv("HOME"));
    }
    if (chdir(path)){                                   //Intentamos cambiar el path.
        perror("chdir");
        printf("%s\n",path);
    }
    return 1;
}
/**
 * Funcion complementaria a internal_cd que concatena los tokens.
 * También es utilizada en el source para concatenar los path.
 * */
void advanced_cd(char *path,char **args){
    char *hasDelimiters;
    size_t cntArgs = 1;
    if ((hasDelimiters = strtok(args[cntArgs], ADVCD))){    //Observamos si contiene algun delimitador
        strcpy(path, hasDelimiters);                //Copiamos la palabra al path
        if ((hasDelimiters = strtok(NULL, ADVCD))){   //En caso de que la palabra contenga algun otro delimitador, concatenamos
                                                    //su contenido con lo que hay en el path.
            strcat(path, hasDelimiters);
        }
    }
    cntArgs++;
    while (args[cntArgs] != NULL){                  //Mientras no lleguemos al final de los tokens
        hasDelimiters = strtok(args[cntArgs], ADVCD);        //
        strcat(path," ");                           //Concatenamos el nombre del directorio sin los delimitadores al path.
        strcat(path, hasDelimiters);                         //
        hasDelimiters = strtok(NULL,ADVCD);                  //
        while(hasDelimiters){                                //En caso de que un token contenga mas palabras en el token despues
            strcat(path,hasDelimiters);                      //del delimitador, lo concatenamos al path.
            hasDelimiters = strtok(NULL,ADVCD);              //
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
    char path[COMMAND_LINE_SIZE];
    strcpy(path,args[1]);
    advanced_cd(path,args);
    FILE *fd = fopen(path,"r");
    char buff[COMMAND_LINE_SIZE];
    if (fd > 0){
        int cnt = 0;
        while(fgets(buff,COMMAND_LINE_SIZE,fd) > 0){
            fflush(fd);
            printf("%s:[line %d] %s\n",path,cnt, buff);
            fflush(stdout);
            execute_line(buff);
            cnt++;
        }
        if(fclose(fd) != 0){
            perror("fclose");
        }
    }else{
        perror("fopen");
        puts("Sintaxis del source: source <filename that exists>");
    }
    
    
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