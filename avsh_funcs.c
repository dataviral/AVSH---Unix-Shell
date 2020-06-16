#include "avsh.h"

void load_history(){
        h.n=0;
        if( access( historyFile, F_OK ) != -1 ) {
                // file exists
                FILE *fp = fopen(historyFile, "r");
                char buf;
                char *lineInputByUser = NULL;
                size_t size;
                while( getline(&lineInputByUser, &size, fp) ){
                        lineInputByUser[strcspn(lineInputByUser, "\r\n")] = 0;
                        if(strlen(lineInputByUser) <= 1) break;
                        h.h[h.n++] = strdup(lineInputByUser);
                }
        }
}

void rewrite_history(){
        FILE *fp =  fopen(historyFile, "w+");
        int i;
        for(i=0; i<h.n; i++){
                fprintf(fp, "%s\n", h.h[i]);
        }
        fprintf(fp, "\n");
        fclose(fp);
}

void show_history(int n){
        if( access( historyFile, F_OK ) != -1 ) {
                printf("\n");
                FILE *fp =  fopen(historyFile, "r");
                int i;
                if(n<=0 || n >= h.n) i=0;
                else i=h.n-n;
                for(; i<h.n; i++){
                        dprintf(outfd, "%d: %s",i, h.h[i]);
                        printf("\n");
                }
        } else {
        }
}

void load_alias(){
        a.n=0;
        if( access( aliasFile, F_OK ) != -1 ) {
                // file exists
                FILE *fp = fopen(aliasFile, "r");
                char buf;
                char *lineInputByUser = NULL;
                size_t size;
                while( getline(&lineInputByUser, &size, fp) != EOF){
                        lineInputByUser[strcspn(lineInputByUser, "\r\n")] = 0;
                        if(strlen(lineInputByUser) <= 1) break;
                        a.a[a.n++] = strdup(lineInputByUser);
                        // printf("%s %ld\n", lineInputByUser, strlen(lineInputByUser));
                }
        }
}

void write_alias(){
        FILE *fp =  fopen(aliasFile, "w+");
        int i;
        for(i=0; i<a.n; i++){
                fprintf(fp, "%s\n", a.a[i]);
        }
        fprintf(fp, "\n");
        fclose(fp);
}

void replace_alias(char ***toks){
        char **tokens = *toks;
	int i=0;
	int j;
        char *replace = NULL;
	for(i=0; i<a.n; i+=2){
                if(strcmp(a.a[i], tokens[0])==0){
                        replace = a.a[i+1];
                        break;
                }
        }
        if(replace != NULL){
                // printf("TO replace %s\n", replace);
                char *newString = malloc(1000);
                memset(newString, 0, sizeof(newString));
                strcat(newString, replace);
                i=1;
                while (tokens[i] != NULL) {
                        if(i != 1) strcat(newString, " ");
                        strcat(newString, tokens[i]);
                        i++;
                }
                // printf("newString %s\n", newString );
                *toks = get_tokens(newString);
        }

}

char **get_tokens(char *line){

        char *copyOfLine = strdup(line);
        char **tokens;

        tokens = malloc(sizeof(char *)*10);
        memset(tokens, 0, sizeof(tokens));

        char subbuff[20];

        int llen = strlen(copyOfLine);
        int numTokens = 0;
        int i =0, j=0;
        while(i < llen){
                if(is_special(copyOfLine[j])) {
                        if (copyOfLine[j] == ' ') {
                                memcpy( subbuff, copyOfLine+i, j-i);
                                subbuff[j-i] = '\0';
                                tokens[numTokens] = strdup(subbuff);
                                numTokens++;
                                i=j+1;
                        }
                }
                if(j == llen){
                        memcpy( subbuff, copyOfLine+i, j-i);
                        subbuff[j-i] = '\0';
                        tokens[numTokens] = strdup(subbuff);
                        numTokens++;
                        i=j+1;
                }
                j++;
        }
        tokens[numTokens] = NULL;
        return tokens;
}

int is_special(char ch){
       	if(ch >= 'a' && ch <='z' )
                return 0;
        if(ch >= 'A' && ch <= 'Z')
                return 0;
        if(ch >= '0' && ch <= '9')
                return 0;
        if(ch == '.' || ch == '-' || ch == '<')
                return 0;
        return ch;
}

int run_command(char **tokens, char **env){
        int toklen;
        for(toklen=0;tokens[toklen]!=NULL; toklen++);

        if(strcmp(tokens[0], "!") == 0){
                if( toklen!=2 ){
                        printf("Incorrect Usage :\n! <cmd #>\n");
                        return -1;
                }
               tokens = get_tokens(h.h[atoi(tokens[1])]);
        }

        if(strcmp(tokens[0], "cd") == 0) {
                if( toklen!=2 ){
                        printf("Incorrect Usage :\ncd <path>\n");
                        return -1;
                }
                if( chdir(tokens[1]) == 0 && getcwd(cwd, sizeof(cwd) ) == 0){
                        dprintf(outfd, "chdir / getcwd failed\n");
                }
                return 0;
        } else if (strcmp(tokens[0], "cwd") == 0){
                if( toklen!=1 ){
                        printf("Incorrect Usage :\ncwd\n");
                        return -1;
                }
                dprintf(errfd, "%s\n", cwd);
                return 0;
        }
        if (strcmp(tokens[0], "history") == 0){
                if( toklen != 1 && toklen != 2){
                        printf("Incorrect Usage :\nhistory <#>\n");
                        return -1;
                }
                if(toklen == 1) show_history(0);
                else show_history(atoi(tokens[1]));
                return 0;
        } else if(strcmp(tokens[0], "alias") == 0) {
                if( toklen < 3 ){
                        printf("Incorrect Usage :\nalias <aliased_name> <cmd_to_alias>\n");
                        return -1;
                }
                int i;
                char aliased[100];
                memset(aliased, 0, sizeof(aliased));
                for(i=2; tokens[i] != NULL; i++) {
                        strcat(aliased, tokens[i]);
                        if(tokens[i] != NULL) strcat(aliased, " ");
                }
                a.a[a.n++] = tokens[1];
                a.a[a.n++] = strdup(aliased);
                write_alias();
                return 0;
        } else if(strcmp(tokens[0], "unalias")==0) {
                if( toklen < 2 ){
                        printf("Incorrect Usage :\nunalias <aliased_name> \n");
                        return -1;
                }
                int flg=-1;
                int i,j;
                for(i=0; i<a.n; i+=2){
                        if(strcmp(tokens[1], a.a[i])==0){
                                flg=i;
                                for(j=i; j<a.n; j++) a.a[j] = a.a[j+2];
                                a.n -= 2;
                                break;
                        }
                }
                if(i<0) printf("cannot unalias %s : name not present \n", tokens[1]);
                write_alias();
                return 0;
        } else if(strcmp(tokens[0], "avct")==0){
                if( toklen < 3 ){
                        printf("Incorrect Usage :\navct <ip> <port>\n");
                        return -1;
                }
                numSigint = 0;
                int f = fork();
                if(!f){
                        connect_avsh(tokens[1], atoi(tokens[2]));
                } else {
                        int stat;
                        waitpid(f, &stat, 0);
                        return stat;
                }

        } else if(strcmp(tokens[0], "avctd")==0){
                if( toklen < 3 ){
                        printf("Incorrect Usage :\navctd <ip> <port>\n");
                        return -1;
                }
                int f = fork();
                if(!f){
                        start_server(tokens[1], atoi(tokens[2]));
                } else {
                        int stat;
                        waitpid(f, &stat, 0);
                        return stat;
                }
        } else if(strcmp(tokens[0], "env")==0){

        }
	replace_alias(&tokens);
        int cmdPos[10][2] = {0,0};

        int i = 0, j = 0, k=0;
        int outputRedirect = 0, inputRedirect = 0;
        while(tokens[i] != NULL){
                if( (strlen(tokens[i]) == 1) && is_special(tokens[i][0]) ){
                        if(tokens[i][0] == '>' || tokens[i][0] == '?'){
                                outputRedirect = i+1;
                                break;
                                cmdPos[k][0] = j;
                                cmdPos[k][1] = i-1;
                                j = i+1;
                                k++;
                        } else if (tokens[i][0] == '|'){
                                cmdPos[k][0] = j;
                                cmdPos[k][1] = i-1;
                                j = i+1;
                                k++;
                        }
                }
                i++;
        }


        int totTok = i;
        cmdPos[k][0] = j;
        cmdPos[k][1] = i-1;
        int h = 0;

        int pid = fork();
        if(pid < 0){
                printf("Fork Failed\n");
        } else if( pid == 0){

                int i;
                for( i=0; i<k; i++) {

                        char **command = malloc(sizeof(char *)*10);
                        int n=0, m;
                        for(m=cmdPos[i][0]; m <= cmdPos[i][1]; m++ ){
                                if(strlen(tokens[m]) == 0){
                                        continue;
                                } else if (strlen(tokens[m]) == 1 && tokens[m][0] == '<'){
                                        continue;
                                }
                                command[n] = tokens[m];
                                n++;
                        }
                        command[n] = NULL;

                        int pd[2];
                        pipe(pd);
                        if (!fork()) {
                                if(i==0 && inputRedirect){ // take input from file
                                        printf("Internal\n");
                                        int opfd= open(tokens[inputRedirect], O_RDWR, S_IRWXU);
                                        dup2(opfd, 0);
                                        close(opfd);
                                }
                                dup2(pd[1], 1); // remap output back to parent
                                execvpe(command[0], command , env);
                                perror("exec");
                                abort();
                        }

                        // remap output from previous child to input
                        dup2(pd[0], 0);
                        close(pd[1]);
                }
                char **command = malloc(sizeof(char *)*30);
                int n=0, m;
                for(m=cmdPos[i][0]; m <= cmdPos[i][1]; m++ ){
                        if(strlen(tokens[m]) == 0){
                                continue;
                        } else if (strlen(tokens[m]) == 1 && tokens[m][0] == '<'){
                                continue;
                        }
                        command[n] = tokens[m];
                        n++;
                }
                command[n] = NULL;

                if(i==0 && inputRedirect){ // take input from file
                        int opfd= open(tokens[inputRedirect], O_RDONLY);
                        dup2(opfd, STDIN_FILENO);
                        close(opfd);
                }
                if(outputRedirect){
                        int opfd;
                        if(tokens[outputRedirect-1][0] == '>') opfd = open(tokens[outputRedirect], O_CREAT|O_RDWR|O_TRUNC, S_IRWXU);
                        else if(tokens[outputRedirect-1][0] == '?' ) opfd = open(tokens[outputRedirect], O_CREAT|O_APPEND|O_RDWR, S_IRWXU);
                        dup2(opfd, 1);
                        close(opfd);
                }

                execvpe(command[0], command , env);
                perror("exec");
                abort();
        } else {
                int status;
                waitpid(pid, &status, 0);
                return status;
        }
        return 0;
}

void start_server(char *ip, int port){
        int socket_desc , client_sock , c , *new_sock;
        struct sockaddr_in server , client;

        socket_desc = socket(AF_INET , SOCK_STREAM , 0);
        if (socket_desc == -1){
                printf("Could not create socket");
        }

        //Prepare the sockaddr_in structure
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons( port );
        bzero (&server.sin_zero, 8);


        //Bind
        if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
        {
            //print the error message
            perror("bind failed. Error");

        }
        //Listen
        listen(socket_desc , MAX_CLIENTS);
        //Accept and incoming connection
        printf("Waiting for incoming connections\n");
        c = sizeof(struct sockaddr_in);
        while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) ){
                printf("Connection accepted\n");
                pthread_t thread_id;

                if( pthread_create( &thread_id , NULL ,  new_connection_handler , (void*) &client_sock) < 0){
                    perror("could not create thread");

                }
                printf("Handler assigned\n");
        }
}

void *new_connection_handler(void *socket_desc){
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];
    char *data = malloc(100000);
    char *msg = "----------------You are connected to a Remote Machine----------------\n";
    write(sock , msg , strlen(msg));
    //Receive a message from client
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
        //Send the message back to client
        printf("CMD>> %s (%d)\n", client_message, read_size);
        int paresedMessage = parseInputServer(client_message, sock);
        if(paresedMessage == 1){
                char buf[100];
                memset(data, 0, sizeof(data));
                memset(buf, 0, sizeof(buf));
                FILE *stream = popen(client_message, "r");
                while (fgets(buf, sizeof(buf), stream) != 0) {
                        strcat(data, buf);
                        memset(buf, 0, sizeof(buf));
                }
                pclose(stream);
                strcat(data, "\n");
                write(sock , data , strlen(data));
                memset(client_message, 0, sizeof(client_message));
        } else {
                write(sock, "\n", strlen("\n"));
        }
    }

    if(read_size == 0){
        printf("Client disconnected\n");
        fflush(stdout);
    }
    else if(read_size == -1){
        perror("recv failed");
    }

    //Free the socket pointer
    // if(socket_desc){
    //         free(socket_desc);
    // }
    return 0;
}

int parseInputServer(char *ip, int sock){
        ip[strcspn(ip, "\r\n")] = 0;
        printf("Parse Input : %s SIZE : %ld \n", ip, strlen(ip));
        if(strlen(ip) < 2){
                return 0;
        }
        char **tokens = get_tokens(ip);
        if (strcmp(tokens[0], "cd")==0){
                chdir(tokens[1]);
                return 0;
        } else if ( strcmp(tokens[0], "get")==0){
                get_fileServer(tokens[1], sock);
                return 0;
        } else if ( strcmp(tokens[0], "send")==0){
                get_fileClient(tokens[2], sock);
                write(sock, "\n", strlen("\n"));
                return 0;
        } else if( strcmp(tokens[0], "ctrl+c")==0){
                printf("-------------------Connection Terminated By Client-------------------\n");
                exit(0);
        }
        return 1;
}

void get_fileServer(char *fileName, int sock){
        FILE *fp = fopen(fileName, "r");
        if(!fp){
                char *msg = "Could not find File\n";
                printf("%s\n", msg);
                write(sock , msg , strlen(msg));
        } else {
                char buf[100];
                int tlen=0,len;
                memset(buf, 0, sizeof(buf));
                while((len=fread(buf, 1, sizeof buf, fp))>0){
                        tlen+=len;
                        write(sock , buf , len);
                        memset(buf, 0, sizeof(buf));
                        if (len<sizeof(buf)) break;
                }
                printf("SENT : %d bytes\n",tlen);
                fclose(fp);
        }
}

void connect_avsh(char *ip, int port){
        struct sockaddr_in server;
        char message[1000] , server_reply[100];
        int len;


        //Create socket
        sock = socket(AF_INET , SOCK_STREAM , 0);
        if (sock == -1) {
                printf("Could not create socket");
        }
        // puts("Socket created");

        server.sin_addr.s_addr = inet_addr(ip);
        server.sin_family = AF_INET;
        server.sin_port = htons( port );

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
            perror("Failed To Connect");
    } else {
            if( signal(SIGINT, sighandler) < 0 ){
                printf("Could Not Set Signal Handler\n");
            }
    }

    while((len = recv(sock, server_reply, sizeof(server_reply), 0)) > 0) {
            printf("\n%.*s", len, server_reply);
            if(len == sizeof(server_reply)){
                    while((len = recv(sock, server_reply, sizeof(server_reply), 0)) > 0){
                            printf("%.*s", len, server_reply);
                            if(len < sizeof(server_reply)) break;
                        }
                }
                printf("\n");

            printf("%s:%d >> ", ip, port);
            if (fgets(message, sizeof(message), stdin) == NULL)
            break;

            if( send(sock , message , strlen(message) , 0) < 0){
                    puts("Send failed");
            } else{
                    parseInputClient(message, sock);
            }
    }
    close(sock);
}

void get_fileClient(char *fileName, int sock){
        int fp = open(fileName, O_CREAT|O_WRONLY|O_TRUNC, 0644);
        sleep(1);
        if(!fp){
                char *msg = "Could not open find File\n";
                printf("%s\n", msg);
        } else {
                int len;
                char *buf = malloc(1000);
                memset(buf, 0 , sizeof(buf));
                while((len = recv(sock, buf, sizeof(buf), 0)) > 0){
                        if (len < sizeof(buf)){
                                write(fp, buf, len);
                                break;
                        } else {
                                write(fp, buf, len);
                        }
                        memset(buf, 0 , sizeof(buf));
                }
                close(fp);
                write(sock, "\n", strlen("\n"));
        }
}

int parseInputClient(char *ip, int sock){
        ip[strcspn(ip, "\r\n")] = 0;
        printf("Parse Input : %s SIZE : %ld \n", ip, strlen(ip));
        if(strlen(ip) < 2){
                return 0;
        }
        int toklen;
        char **tokens = get_tokens(ip);
        for(toklen=0;tokens[toklen]!=NULL; toklen++);
        if ( strcmp(tokens[0], "get")==0){
                if(toklen<3){
                        printf("Invaid Usage\nget <server_file_path_to_get> <client_name_to_save>\n");
                }
                get_fileClient(tokens[2], sock);
                return 0;
        } else if ( strcmp(tokens[0], "send")==0){
                if(toklen<3){
                        printf("Invaid Usage\nsend <client_file_path_to_send> <server_name_to_save>\n");
                }
                get_fileServer(tokens[1], sock);
                return 0;
        }
        return 1;
}

void sighandler(int signo) {
        if(signo == SIGINT) {
                char *message = "ctrl+c";
                if (send(sock , message , strlen(message) , 0));
                close(sock);
                printf("\n-------------------Terminated Connection-------------------\n");
                // numSigint = 1;
                signal(SIGINT, SIG_DFL);
                // signal(SIGINT, sighandler_main);
                exit(0);
        }
}

void sighandler_main(int signo) {
        if(signo == SIGINT) {
                if(numSigint < 1){
                        numSigint++;
                } else {
                        printf("\n\n");
                        signal(SIGINT, SIG_DFL);
                        exit(0);
                }
        }
}
