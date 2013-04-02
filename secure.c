#include <signal.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string>

void usage();

int main(int argc, char **argv)
{
        if(argc != 2)
            usage();
        std::string file1 = "/u/vgupta/github/sample/data/";
        file1.append(argv[1]);
        file1.append("/function");
        printf("%s\n", file1.c_str());
        const char* fileloc = file1.c_str();

        int i;
        pid_t child;
        int status;
        long orig_eax;
        int kill_ret = 0;




        child = fork();


        if(child == 0)
        {
                ptrace(PTRACE_TRACEME, 0, NULL, NULL);
                execl(fileloc, fileloc,  NULL);
        }
        else
        {
            bool filevar = false, kill_check = false;
                i = 0;
                while(1)
                {
                        wait(&status);
                        if (WIFEXITED(status) || WIFSIGNALED(status) )
                                break;

                        orig_eax = ptrace(PTRACE_PEEKUSER, child, 8 * ORIG_RAX, NULL);
                        if(orig_eax != 12 && orig_eax != 2)
                            filevar = false;
                        switch (orig_eax)
                        {
                            case 1: //for file open write
                                kill_check = true;
                            case 0: //for file open read
                            case 2: //for file open both
                                if(filevar)
                                    kill_check = true;
                                if(kill_check) fprintf(stderr, "Invalid System Call: FILE_ACCESS\n");
                                break;
                            case 12: //is needed for file.open()
                                filevar = true;
                                break;
                            case 87: //for stdlib remove()
                            case 56: //for system call
                                fprintf(stderr, "Invalid System Call: SYST_CALL\n");
                                kill_check = true;
                                break;
                        }
                        if(kill_check) {
                            kill_ret = kill(child, SIGKILL);
                            if (kill_ret == -1)
                                fprintf(stderr, "Failed to kill ---> %s\n", strerror(errno));
                        }
                        //printf("%d time, system call %ld\n", i++, orig_eax);
                        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
                }
        }
        return 0;
}

void usage() {
    printf("./secure <randomchars> <type>\n");
    exit(0);
}