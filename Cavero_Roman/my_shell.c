//Marc Roman Colom y Laura Cavero Loza
#include "my_shell.h"

static struct info_process jobs_list[N_JOBS];
char prompt[COMMAND_LINE_SIZE];
char *execHasMade = AMARILLO_T;
char PROGNAME[COMMAND_LINE_SIZE];
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
                execHasMade = VERDEF_T;
                execute_line(line);
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
                //printf("Soy el padre y mi pid es:%d\n", getpid());
                if (isbkg != 1){
                    jobs_list[0].pid = cpid;
                    jobs_list[0].status = 'E';
                    strcpy(jobs_list[0].cmd, cmdLine);
                    while (jobs_list[0].pid > 0){
                        pause();
                    }
                }else{
                    jobs_list_add(cpid, 'E', cmdLine);
                }
            }else{  //Codigo del proceso hijo.
                signal(SIGCHLD,SIG_DFL);
                signal(SIGINT,SIG_IGN);
                signal(SIGTSTP,SIG_IGN);
                //printf("\nSoy el hijo y mi pid es: %d\n", getpid());
                is_output_redirection(args);
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
        execHasMade = ROJOF_T; //Cambiamos el color del prompt para notificar que ha fallado su ejecución
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
        strcpy(path, hasDelimiters);                    //Copiamos la palabra al path
        if ((hasDelimiters = strtok(NULL, ADVCD))){     //En caso de que la palabra contenga algun otro delimitador, concatenamos
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
    if (strcmp(value,"")==0 || strcmp(name,"")==0 || setenv(name, value, 1)){
        fprintf(stderr, "Error de sintaxis. Uso: Nombre=Valor\n");
        execHasMade = ROJOF_T; //Cambiamos el color del prompt para notificar que ha fallado su ejecución
    }
    return 1;
}

/**
 * Lee y ejecuta los comandos de shell escritos en el archivo indicado por parametro.
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
            execHasMade = ROJOF_T; //Cambiamos el color del prompt para notificar que ha fallado su ejecución
        }
    }else{
        perror("fopen");
        puts("Sintaxis del source: source <filename that exists>");
        execHasMade = ROJOF_T; //Cambiamos el color del prompt para notificar que ha fallado su ejecución
    }
    
    
    return 1;
}

/**
 * Muestra la lista de trabajos que hay. Nos muestra su respectivo índice, pid, status y el comando en cuestión.
 * */
int internal_jobs(char **args){
    printf("\t Nº\tPID\tSTATUS\tCMDLINE\n");
    for (size_t i = 0; i < n_pids; i++){
        printf("\t[%ld]\t%d\t   %c\t%s",i, jobs_list[i].pid,jobs_list[i].status,jobs_list[i].cmd);
        if (i < n_pids){
            printf("\n");
        }
    }
    return 1;
}

/**
 * Mueve un trabajo que estaba en background a foreground mediante. En caso de que el trabajo estuviese detenido, lo reanuda.
 * */
int internal_fg(char **args){
    if (args[1] != NULL){               //Si el token no está vacio, continuamos
        int pos;
        char *c = strchr(args[1], '\%');
        if (c != NULL){                 //Si contiene % entonces lo tratamos.
            pos = atoi(c+1);
        }else{                          //En caso contrario, seguimos como si fuese un numero.
            pos = atoi(args[1]);   
        }
        if (pos >= n_pids || pos <= 0){ //Si el indice es mayor o igual que la cantidad de trabajos que hay o menor o igual que 0
                                        //entonces no existe el trabajo.
            perror("No existe el trabajo");
            execHasMade = ROJOF_T;      //Cambiamos el color del prompt para notificar que ha fallado su ejecución
            return 1;
        }
        if (jobs_list[pos].status =='D'){   //Si el proceso estaba detenido, lo reanuda.
            kill(jobs_list[pos].pid, SIGCONT);
        }
        c = strtok(jobs_list[pos].cmd,"&"); //Le quitamos & del cmd, lo movemos al job en foreground, eliminamos el de la lista y
                                            //esperamos a que el proceso termine en un bucle con pause.
        strcpy(jobs_list[pos].cmd,c);
        jobs_list[0] = jobs_list[pos];
        jobs_list_remove(pos);
        while (jobs_list[0].pid > 0){
            pause();
        }
        
    }else{
        execHasMade = ROJOF_T; //Cambiamos el color del prompt para notificar que ha fallado su ejecución
        perror("Faltan argumentos. Syntaxis: fg %<num del trabajo> o <num del trabajo>");
    }
    return 1;
}

/**
 * Pone un proceso en background detenido en ejecución. Si no lo está, da error.
 * */
int internal_bg(char **args){
    if (args[1] != NULL){
        char *c=strchr(args[1],'\%');
        int pos;
        if (c != NULL){                 //Si encontramos el caracter %, obtenemos solo el numero.
            pos = atoi(c+1);
        }else{                          //Sinó, obtenemos el numero.
            pos = atoi(args[1]);
        }
        if(pos < n_pids && pos >= 0){   //Si la posición del proceso existe, seguimos.
            if (jobs_list[pos].status == 'D'){  //Si el estado del proceso es 'Detenido', lo cambiamos a 
                                                //'Ejecutandose' y le enviamos la señal SIGCONT para que continue.
                printf("[%d] con pid %d status %c y cmd %s puesto en ejecucion\n", pos, jobs_list[pos].pid, jobs_list[pos].status, jobs_list[pos].cmd);
                jobs_list[pos].status = 'E';
                kill(jobs_list[pos].pid,SIGCONT);
            }else{                              //Sinó, entonces el proceso ya está en ejecución.
                printf("El proceso ya esta en ejecucion\n");
            }
        }else{                          //Sino existe la posición del proceso, entonces error de sintaxis.
            perror("Error, sintaxis correcta; bg \%<num del trabajo> o <num del trabajo>");
        }
    }
    return 1;
}

/**
 * Maneja la señal producida cuando un proceso hijo muere.
 * El padre espera a cualquier proceso hijo para recogerlo. En caso de que el hijo finalizado fuese el proceso en foreground,
 * modifica el trabajo que representa el proceso foreground para indicar que ha finalizado.
 * En caso de ser el proceso en background, lo busca y lo elimina de la lista de trabajos.
 * */
void reaper(int signum){
    signal(SIGCHLD,reaper);
    pid_t p_ended;
    int status;     //Usado para almacenar el estatus de finalización del hijo.
    while ((p_ended = waitpid(-1,&status,WNOHANG)) > 0){
        if (p_ended == jobs_list[0].pid){   //En caso de que el proceso terminado sea el de foreground.
            strcpy(jobs_list[0].cmd,"");
            jobs_list[0].pid = 0;
            jobs_list[0].status= 'F';
        }else if( p_ended == -1){           //En caso de que el proceso no se haya podido enterrar.
            perror("Fallo al intentar enterrar el proceso");
            execHasMade = ROJOF_T;          //Cambiamos el color del prompt para notificar que ha fallado su ejecución
        }else{                              //En caso de que el proceso terminado sea en background.
            int pos = jobs_list_find(p_ended);
            printf("\nEl proceso en background [%d] con pid |%d| y cmd |%s| terminó\n",pos, jobs_list[pos].pid,jobs_list[pos].cmd);
            if(jobs_list_remove(pos)){
                perror("No se pudo eliminar el trabajo");
                execHasMade = ROJOF_T; //Cambiamos el color del prompt para notificar que ha fallado su ejecución
            }
        }
        if (WIFEXITED(status)){
            printf("Child finished his work by exit: result of exit: %d and pid is:%d\n", WEXITSTATUS(status), p_ended);
        }
        else if (WIFSIGNALED(status)){
            printf("Child finished his process by a signal: result of signal: %d and pid is: %d\n", WTERMSIG(status), p_ended);
        }
    }
    if(status){ //En caso de que el hijo haga EXIT_FAILURE, cambiamos el prompt a que la ejecución fué mal. 
        execHasMade = ROJOF_T;
    }
}
/**
 * Maneja la señal que produce el uso de (Ctrl + C).
 * Si existe un proceso en foreground y no es el shell, entonces termina el proceso.
 * En caso de que no exista o el proceso en foreground sea el shell, da error.
 * */
void ctrlc(int signum){
    signal(SIGINT,ctrlc);
    printf("Soy el proceso con pid %d, el proceso en foreground es %d\n",getpid(),jobs_list[0].pid);
    if (jobs_list[0].pid > 0){  //Si hay un proceso en foreground
        if (strcmp(PROGNAME,jobs_list[0].cmd) != 0){    //Si el proceso en foreground es el shell.
            printf("El proceso %d ha muerto, su cmd era: %s\n",jobs_list[0].pid, jobs_list[0].cmd);
            kill(jobs_list[0].pid,SIGTERM);
        }else{
            perror("Señal SIGTERM no enviada debido a que el proceso en foreground es el shell");
            execHasMade = ROJOF_T; //Cambiamos el color del prompt para notificar que ha fallado su ejecución
        }
    }else{
        perror("Señal SIGTERM no enviada debido a que no hay proceso en foreground");
        execHasMade = ROJOF_T; //Cambiamos el color del prompt para notificar que ha fallado su ejecución
    }
    fflush(stdout);
}
/**
 * Maneja la señal que produce el uso de (Ctrl + Z).
 * Si hay un proceso en foreground y no es el shell, entonces lo detiene y lo deja en background.
 * En caso de que no haya un proceso en foreground o el proceso en foreground es el shell, da error.
 * */
void ctrlz(int signum){
    signal(SIGTSTP,ctrlz);
    if (jobs_list[0].pid > 0 && strcmp(jobs_list[0].cmd,PROGNAME) != 0){                 //Si el proceso esta en foreground
        kill(jobs_list[0].pid, SIGSTOP);
        strcat(jobs_list[0].cmd," &");
        jobs_list_add(jobs_list[0].pid,'D',jobs_list[0].cmd);
        strcpy(jobs_list[0].cmd,"");
        jobs_list[0].pid = 0;
        jobs_list[0].status = 'N';
    }else if(jobs_list[0].pid  > 0){
        perror("Señal SIGSTOP no enviada debido a que el proceso en foreground es el shell");
        execHasMade = ROJOF_T; //Cambiamos el color del prompt para notificar que ha fallado su ejecución
    }else{
        perror("Señal SIGSTOP no enviada debido a que no hay proceso en foreground");
        execHasMade = ROJOF_T; //Cambiamos el color del prompt para notificar que ha fallado su ejecución
    }
}
/**
 * Devuelve 1 si el comando del shell debe ejecutarse en background (encuentra &) y elimina &.
 * En caso contrario, devuelve 0.
 * */
int is_background(char **args, int *numOfElements){
    int isbkg = 0;
    if (strcmp(args[*(numOfElements)-1],"&") == 0){ //Si en la posición final del numero de elementos hay un &, lo elimina
        isbkg = 1;                                  //y decrementa el numero de elementos.
        *(numOfElements) = *(numOfElements) -1;
        args[*numOfElements] = NULL;
    }
    return isbkg;
}

/**
 * Añade un trabajo a la lista de trabajos siempre que no se pase del límite establecido.
 * */
int jobs_list_add(pid_t pid, char status, char *cmd){
    if (n_pids < N_JOBS){                           //Mientras no se llegue al maximo de jobs permitidos
        struct info_process job;
        strcpy(job.cmd,cmd);
        job.pid = pid;
        job.status = status;
        jobs_list[n_pids] = job;
        n_pids++;
        return 0;
    }else{
        perror("Se ha llegado al límite de jobs");
        execHasMade = ROJOF_T; //Cambiamos el color del prompt para notificar que ha fallado su ejecución
        return 1;
    }
    
}

/**
 * Busca un trabajo en la lista de trabajos: si lo encuentra, devuelve el índice de éste.
 * En caso de que la busqueda falle, devuelve -1.
 * */
int jobs_list_find(pid_t pid){
    for (size_t i = 1; i <= n_pids; i++){
        if (jobs_list[i].pid == pid){
            return i;
        }
    }
    return -1;
}

/**
 * Elimina el trabajo almacenado en el índice pasado por parámetro de la lista de trabajos y
 * devuelve 0. En caso contrario, devuelve -1 y error.
 * */
int jobs_list_remove(int pos){
    if (pos < n_pids && pos > 0){   //Si el índice está entre 1 y el numero de trabajos que hay, procede a borrarlo.
        jobs_list[pos].pid = jobs_list[n_pids-1].pid;
        jobs_list[pos].status = jobs_list[n_pids-1].status;
        strcpy(jobs_list[pos].cmd,jobs_list[n_pids-1].cmd);
        n_pids--;
        return 0;
    }else{
        perror("No se ha podido eliminar el proceso!");
        execHasMade = ROJOF_T; //Cambiamos el color del prompt para notificar que ha fallado su ejecución
        return -1;
    }
}
/**
 * Observa si el comando del shell debe ser redireccionado a un archivo. En caso de ser así, elimina > de los argumentos
 * y redirecciona el resultado del comando al archivo. En caso de que el archivo no exista, lo crea.
 * Devuelve 0 si no hay redirección. En caso contrario, devuelve 1. También devuelve un error si se llega a producir.
 * */
int is_output_redirection(char **args){
    int has_output = 0;                                             //Esta variable nos sirve como índice y para indicar si args contiene >.
    for (size_t i = 0; args[i] != NULL && has_output == 0; i++){    //Buscamos si algun token contiene >
        if (strchr(args[i], '>') != NULL){                          //Si lo contiene, lo elimina, incrementa el índice y se lo asigna a has_output.
            args[i] = NULL;
            i++;
            has_output = i;
        }        
    }
    if( has_output  == 0){                                          //Si no contiene >, devolvemos 0.
        return 0;
    }else if(args[has_output] != NULL ){                            //En caso contrario, miramos si el siguiente token es distinto de NULL.
        int fd = open(args[has_output], O_WRONLY | O_CREAT | O_APPEND); //Si lo es, abrimos el archivo y obtenemos su file descriptor.
        if(fd > 0){                                                 //Intentamos abrir el file
            if(dup2(fd,1) == -1){                                   //Intentamos duplicar el descriptor de fichero para redireccionar 
                                                                    //la salida del stdout. Si da error, entonces saltamos el error.
                perror("No se ha podido duplicar el descriptor de ficheros");
                execHasMade = ROJOF_T; //Cambiamos el color del prompt para notificar que ha fallado su ejecución
            }else{                    
                if(close(fd) == -1){                                //Intentamos cerrar el descriptor de fichero, si falla, tratamos error.
                    perror("El archivo no se pudo cerrar");
                    execHasMade = ROJOF_T; //Cambiamos el color del prompt para notificar que ha fallado su ejecución
                }
            }
        }else{                                                      //En caso de no abrir bien el file,si falla, tratamos error.
            perror("No se ha podido abrir el archivo");
            execHasMade = ROJOF_T; //Cambiamos el color del prompt para notificar que ha fallado su ejecución
        }
    }else{                                                          //Si no viene seguido de filename, entonces es que la sintaxis se escribió mal.
        perror("Error de sintaxis: cmd opcionCMD > filename");
        execHasMade = ROJOF_T; //Cambiamos el color del prompt para notificar que ha fallado su ejecución
    }
    return 1;
}