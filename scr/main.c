#include<stdio.h>    
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<math.h>

#define SIZE 2

void WritePipe(int *lhs, int *rhs, char *op, int fd) {
    write(fd, lhs, sizeof(int));
    write(fd, rhs, sizeof(int));
    write(fd, op, sizeof(char));
}

void ReadPipe(int *res, int fd) {
    read(fd, res, sizeof(int));
}

void write_str(int res){
    char c[100];
    sprintf(c, "%d", res);
    int i = 0;
    while(c[i] != '\0'){
        i++;
    }
    write(1, c, i);
}

typedef enum{
    T_VAL,
    T_NULL,
    T_OPER,
    T_MINUS,
}T_TYPE;

typedef struct{
    int arg1;
    int arg2;
    char oper;
    bool out;
}ARG;

ARG token_read(){
    ARG res;
    T_TYPE prev = T_NULL;
    T_TYPE prev_tmp = T_NULL;
    while(1){
        char c = getchar();
        if(c == EOF || c == '\n'){
            res.out = false;
            return res;
        }
        else if(c == ' '){
            continue;
        }
        else if(c >= '0' && c <= '9'){
            if(prev == T_VAL){
                res.out = false;
                return res;
            }
	    int tmp = c - '0';
            while(1){
                c = getchar();
                if(c >= '0' && c <= '9'){
                    tmp = tmp * 10 + (c - '0');
                }
                else{
                   ungetc(c, stdin);
                   break;
                }
            }
            if(prev == T_NULL){
                if(prev_tmp == T_MINUS){
                    res.arg1 = -tmp;
                    prev_tmp = T_NULL;
                }
                else{
                    res.arg1 = tmp;
                }
                prev = T_VAL;
            }
            else{
                if(prev_tmp == T_MINUS){
                    res.arg2 = -tmp;
                    prev_tmp = T_NULL;
                }
                else{
                    res.arg2 = tmp;
                }
                res.out = true;
                return res;
            }
        }
        else if(c == '*' || c == '/'){
            if(prev != T_VAL){
                res.out = false;
                return res;
            }
            res.oper = c;
            prev = T_OPER;
        }
        else if(c == '-'){
            if(prev == T_NULL || prev == T_OPER){
                prev_tmp = T_MINUS;
            }
            else{
                res.out = false;
                return res;
            }
        }
        else{
            res.out = false;
            return res;
        }
    }
}


int main(){
    int fd1[SIZE];
    int fd2[SIZE];
    char buf[100];
    pid_t pid;
    ARG tmp;
    if (pipe(fd1) == -1){
        write(1, "Pipe не создался\n", 17);
	exit(0);
    }
    if(pipe(fd2) == -1){
        write(1, "Pipe не создался\n", 17);
	exit(0);
    }
    pid = fork();
    if (pid < 0) {
	write(1, "Error. Проблемы с fork\n", 23);
	exit(0);
    } else if (pid > 0) {
	close(fd1[0]);
	close(fd2[1]);

        int res;
	tmp = token_read();
        if(!tmp.out){
            write(1, "ERROR\n", 6);
            return 1;
        }
        WritePipe(&tmp.arg1, &tmp.arg2, &tmp.oper, fd1[1]);
        ReadPipe(&res, fd2[0]);
        write_str(res);
        write(1, "\n", 1);

	close(fd1[1]);
        close(fd2[0]);
     } else {
	close(fd1[1]);
	close(fd2[0]);

	int res, lhs, rhs;
	char oper;
	read(fd1[0], &lhs, sizeof(int));
        read(fd1[0], &rhs, sizeof(int));
        read(fd1[0], &oper, sizeof(char));
        if(oper == '*'){
            res = lhs * rhs;
        }
        else{
            res = lhs / rhs;
        }
	write(fd2[1], &res, sizeof(res));

        close(fd1[0]);
        close(fd2[1]);
    }
    return 0;
}

