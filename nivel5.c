#include "nivel5.h"

static struct info_process jobs_list[N_JOBS];
char prompt[COMMAND_LINE_SIZE];
char *execHasMade = AMARILLO_T;
char PROGNAME[COMMAND_LINE_SIZE];//[COMMAND_LINE_SIZE]; //dudar de si peta
int n_pids = 1; //Variable que indica el numero de procesos que hay.

int main(int argc, char **argv){
    if(argc == 1){  //If there's one argument, continue
        signal(SIGCHLD, reaper);
        signal(SIGINT,ctrlc);
        signal(SIGTSTP,ctrlz);
        strcpy(PROGNAME,argv[0]);
        strcpy(jobs_list[0].cmd, "");
        jobs_list[0].pid = 0;
        jobs_list[0].status = 'N';
        char line[COMMAND_LINE_SIZE]; //1024
        while (1){
            if (read_line(line) && strlen(line) > 0){
                if (execute_line(line)){
                    execHasMade = ROJOF_T;
                }else{
                    execHasMade = VERDEF_T;
                }
            }
            strcpy(line,"");
        }
    }else if(argc > 1){
        printf("Demasiados argumentos!");
    }else{
        printf("Se necesita un argumento!");
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
    if (line[strlen(line)-1] == '\n'){          //Temporal fix to
        line[strlen(line)-1] = '\0';
    }
    return line;
}

/**
 * Ejecuta la línea de comando, siendo un comando interno o externo.
 * */
int execute_line(char *line){
    char *args[ARGS_SIZE];                      //Array de punteros de tokens
    char cmdLine[ARGS_SIZE];
    strcpy(cmdLine,line);
    int numOfArgs = parse_args(args, line);    //Troceamos la linea de cmd y lo almacenamos en el puntero de tokens
    int isbkg = is_background(args, &numOfArgs);
    if (numOfArgs > 0){                        //Si hay tokens, tratamos de ejecutarlo
        if(!check_internal(args)){
            pid_t cpid = fork();
            if (cpid < 0){
                perror("fork ");
            }else if(cpid > 0){ //Codigo del proceso padre.
                int status = 0;
                printf("Soy el padre y mi pid es:%d\n", getpid());
                if (!isbkg){
                    jobs_list[0].pid = cpid;
                    jobs_list[0].status = 'E';
                    strcpy(jobs_list[0].cmd, cmdLine);
                    while (jobs_list[0].pid > 0){
                        pause();
                    }
                }else{
                    status = jobs_list_add(cpid, 'E', cmdLine);
                }
                return status;
            }else{  //Codigo del proceso hijo.
                signal(SIGCHLD,SIG_DFL);
                signal(SIGINT,SIG_IGN);
                signal(SIGTSTP,SIG_IGN);
                printf("Soy el hijo y mi pid es: %d\n", getpid());
                if (execvp(args[0], args)){
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
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
    if (hasDelimiters = strtok(args[cntArgs], ADVCD)){    //Observamos si contiene algun delimitador
        strcpy(path, hasDelimiters);                //Copiamos la palabra al path
        if (hasDelimiters = strtok(NULL, ADVCD)){   //En caso de que la palabra contenga algun otro delimitador, concatenamos
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
    if (value=="" || name=="" || setenv(name, value, 1)){
        fprintf(stderr, "Error de sintaxis. Uso: Nombre=Valor");
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
    printf("\tNº\tPID\tSTATUS\tCMDLINE\n");
    for (size_t i = 0; i < n_pids; i++){
        printf("\t%ld\t%d\t%c\t%s\n",i,jobs_list[i].pid,jobs_list[i].status,jobs_list[i].cmd);
    }
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

void reaper(int signum){
    signal(SIGCHLD,reaper);
    pid_t p_ended;
    int status;     //Usado para almacenar el estatus del hijo.
    while ((p_ended = waitpid(-1,&status,WNOHANG)) > 0);
    if(p_ended == -1){                                      //En caso de que la espera del proceso hijo falle
        perror("La espera del proceso falló...");
    }else if (jobs_list[0].pid == p_ended){                 //En caso de que el proceso hijo esté en foreground.
        jobs_list[0].pid = 0;
        strcpy(jobs_list[0].cmd, "");
        jobs_list[0].status = 'F';
    }else{                                                  //En caso de que el proceso hijo esté en background.
        int site = jobs_list_find(p_ended);
        if (site != -1){
            if(jobs_list_remove(site)){
                perror("El proceso no se ha removido");
            }else{
                if (WIFEXITED(status)){
                    printf("Child finished his work by exit: result of exit: %d and pid is:%d\n", WEXITSTATUS(status), jobs_list[0].pid);
                }
                else if (WIFSIGNALED(status)){
                    printf("Child finished his process by a signal: result of signal: %d and pid is: %d\n", WTERMSIG(status), jobs_list[0].pid);
                }
            }
        }else{
            perror("Proceso no encontrado");
        }
    }
}

void ctrlc(int signum){
    signal(SIGINT,ctrlc);
    printf("Soy el proceso con pid %d, el proceso en foreground es %d\n",getpid(),jobs_list[0].pid);
    if (jobs_list[0].pid > 0){
        printf("\nss%svs%sss\n",PROGNAME, jobs_list[0].cmd);
        printf("%d\n",strcmp(PROGNAME,jobs_list[0].cmd));
        if (strcmp(PROGNAME,jobs_list[0].cmd) != 0){
            kill(jobs_list[0].pid,SIGTERM);
        }else{
            perror("Señal SIGTERM no enviada debido a que el proceso en foreground es el shell");
        }
    }else{
        perror("Señal SIGTERM no enviada debido a que no hay proceso en foreground");
    }
    fflush(stdout);
}

void ctrlz(int signum){
    signal(SIGTSTP,ctrlz);
    if (jobs_list[0].pid > 0 && strcmp(jobs_list[0].cmd,PROGNAME) != 0){                 //Si el proceso esta en foreground
        kill(jobs_list[0].pid, SIGSTOP);
        jobs_list_add(jobs_list[0].pid,'D',jobs_list[0].cmd);
        strcpy(jobs_list[0].cmd,"");
        jobs_list[0].pid = 0;
        jobs_list[0].status = 'F';
    }else if(jobs_list[0].pid  > 0){
        perror("Señal SIGSTOP no enviada debido a que el proceso en foreground es el shell");
    }else{
        perror("Señal SIGSTOP no enviada debido a que no hay proceso en foreground");
    }
}

int is_background(char **args, int *numOfElements){
    int isbkg = 0;
    if (strcmp(args[*numOfElements -1],"&") == 0){
        isbkg = 1;
        args[*numOfElements -1] = NULL;
        *numOfElements = *numOfElements-1;
    }
    return isbkg;
}

int jobs_list_add(pid_t pid, char status, char *cmd){
    struct info_process job;
    job.pid= pid;
    job.status = status;
    strcpy(job.cmd,cmd);
    if (n_pids < N_JOBS){
        jobs_list[n_pids] = job;
        n_pids++;
    }else{
        perror("Se ha llegado a la capacidad máxima de procesos permitidos");
    }
}

int jobs_list_find(pid_t pid){
    int position = 0;
    for(int i = 0; i < n_pids && jobs_list[position].pid != pid; i++){
        position = i;
    }
    if (jobs_list[position].pid == pid){                        //Si el bucle ha parado porque son iguales,
        return position;                                        //entonces devolvemos la posición
    }else{                                                      //Sinó  es que el proceso no se ha encontrado.
        return -1;
    }
}

int jobs_list_remove(int pos){
    if (pos > n_pids || pos < 1){
        perror("No existe el proceso en la lista de jobs");
        return EXIT_FAILURE;
    }else{
        jobs_list[pos] = jobs_list[n_pids];
        n_pids--;
        return EXIT_SUCCESS;
    }
    
    
}