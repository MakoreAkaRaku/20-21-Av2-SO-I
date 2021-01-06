#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define N_JOBS              64
#define EXPSIZE             1024
#define PATHSIZE            1024
#define ARGS_SIZE           64
#define COMMAND_LINE_SIZE   1024
#define PROMPT              '$'
#define HASH                '#'
#define EQUAL               "="
#define ADVCD               "'\"\\"
#define DELIMITERS          " \t\n\r"
//Constantes para uso de colores
#define RESET_COLOR         "\x1b[0m"
#define NEGRO_T             "\x1b[30m"
#define NEGRO_F             "\x1b[40m"
#define ROJO_T              "\x1b[31m"
#define ROJO_F              "\x1b[41m"
#define VERDE_T             "\x1b[32m"
#define VERDE_F             "\x1b[42m"
#define AMARILLO_T          "\x1b[33m"
#define AMARILLO_F          "\x1b[43m"
#define AZUL_T              "\x1b[34m"
#define AZUL_F              "\x1b[44m"
#define MAGENTA_T           "\x1b[35m"
#define MAGENTA_F           "\x1b[45m"
#define CYAN_T              "\x1b[36m"
#define CYAN_F              "\x1b[46m"
#define BLANCO_T            "\x1b[37m"
#define BLANCO_F            "\x1b[47m"
#define VERDEF_T            "\x1b[32;1m"
#define ROJOF_T             "\x1b[31;1m"

void advanced_cd(char *path,char **args);
char *read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs(char **args);
int internal_fg(char **args);
int internal_bg(char **args);
void reaper(int signum);
void ctrlc(int signum);
void ctrlz(int signum);
int is_background(char **args, int *numOfArgs);
int jobs_list_add(pid_t pid, char status, char *cmd);
int jobs_list_remove(int pos);
int jobs_list_find(pid_t pid);
int is_output_redirection(char **args);
struct info_process{
    pid_t pid;
    char status;                 // ‘N’, ’E’, ‘D’, ‘F’
    char cmd[COMMAND_LINE_SIZE]; // línea de comando
};
