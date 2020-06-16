#include<readline/readline.h>
#include<readline/history.h>
#include "avsh.h"

int main(int argc, char **argv, char **env){

        char *lineInputByUser = NULL;
        size_t size;
        int bytesRead;

        char **tokens;
        char shell_prompt[100];
        rl_bind_key('\t', rl_complete);

        if(getcwd(cwd, sizeof(cwd) ) == 0){
                dprintf(outfd, "getcwd failed\n");
        }

        load_history();
        load_alias();

        if(signal(SIGINT, sighandler_main) < 0 ){
                printf("Could not set SIGINT handler : main()\n");
        }
        printf("\n-------------------------------Starting Shell-------------------------------\n\n");
        while(1){
                while ( sigsetjmp( ctrlc_jp, 1 ) != 0 ){
                        // signal(SIGINT, SIG_DFL);
                }

                snprintf(shell_prompt, sizeof(shell_prompt), "\n>> ");
                lineInputByUser = readline(shell_prompt);
                if (!lineInputByUser)
                    break;

                if(strlen(lineInputByUser)==0) continue;
                add_history(lineInputByUser);
                h.h[h.n++] = strdup(lineInputByUser);
                rewrite_history();
                int eval = strcmp(lineInputByUser, "quit");
                if (eval == 0){
                        printf("Good Bye!\n");
                        exit(0);
                }
                /* Seperate Into Arguments */
                tokens = get_tokens(lineInputByUser);
                if(run_command(tokens, env) < 0){
                        printf("Invalid Command\n");
                } else {
                }


        }


        return 0;
}
