#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORTA "21387"           //server A port 
#define PORTB "22387"           //server B port 
#define PORTUDP "23387"
#define PORT "24387"  // the port users will be connecting to
#define IPADDRESS "127.0.0.1" // local IP address
#define BACKLOG 10   // how many pending connections queue will hold
#define MAXDATASIZE 60000 // max number of bytes we can get at once


// from https://beej.us/guide/bgnet/html/#client-server-background
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

// from https://beej.us/guide/bgnet/html/#client-server-background
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int sendToServerB(int new_fd, char *size,char *numNode, char des_list[][300], char minLen_list[][3000], char *proSpeed_str, char *tranSpeed_str){
    //from https://beej.us/guide/bgnet/html/#client-server-background
    int thesockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    int numbytes;
    char *myServer;
    myServer = PORTB;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(IPADDRESS, PORTB, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((thesockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("AWS talker: socket");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "AWS talker: failed to create socket\n");
        return 2;
    }

    if ((numbytes = sendto(thesockfd, size, strlen(size), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("AWS talker: sendto");
        exit(1);
    }






    if ((numbytes = sendto(thesockfd, proSpeed_str, strlen(proSpeed_str), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("AWS talker: sendto");
        exit(1);
    }
    if ((numbytes = sendto(thesockfd, tranSpeed_str, strlen(tranSpeed_str), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("AWS talker: sendto");
        exit(1);
    }

    // Send numNode
    if ((numbytes = sendto(thesockfd, numNode, strlen(numNode), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("AWS talker: sendto");
        exit(1);
    }

    for (int i = 0; i < atoi(numNode); ++i)
    {
        if ((numbytes = sendto(thesockfd, des_list[i], strlen(des_list[i]), 0, p->ai_addr, p->ai_addrlen)) == -1) {
            perror("AWS talker: sendto");
            exit(1);
        }

        if ((numbytes = sendto(thesockfd, minLen_list[i], strlen(minLen_list[i]), 0, p->ai_addr, p->ai_addrlen)) == -1) {
            perror("AWS talker: sendto");
            exit(1);
        }
    }

    printf("The AWS has sent path length, propagation speed and transmission speed to server B using UDP over port <%s>.\n",PORTUDP );

    char Tt_list[atoi(numNode)][3000], Tp_list[atoi(numNode)][3000], Delay_list[atoi(numNode)][3000], result_str[MAXDATASIZE];


    // receive from server B
    printf("The AWS has received delays from server B:\n--------------------------------------------\nDestination     Tt       Tp       Delay\n--------------------------------------------\n");
    for (int i = 0; i < atoi(numNode); ++i)
    {
        char Tp_str[3000], Tt_str[3000], Delay_str[4000];

        if ((numbytes = recvfrom(thesockfd, Tp_str, 3000, 0, NULL, NULL) )== -1) {
            perror("AWS: recvfrom");
            exit(1);
        }
        Tp_str[numbytes] = '\0';
        strcpy(Tp_list[i], Tp_str);

        if ((numbytes = recvfrom(thesockfd, Tt_str, 3000, 0, NULL, NULL) )== -1) {
            perror("AWS: recvfrom");
            exit(1);
        }
        Tt_str[numbytes] = '\0';
        strcpy(Tt_list[i], Tt_str);

        if ((numbytes = recvfrom(thesockfd, Delay_str, 3000, 0, NULL, NULL) )== -1) {
            perror("AWS: recvfrom");
            exit(1);
        }
        Delay_str[numbytes] = '\0';
        strcpy(Delay_list[i], Delay_str);

        printf("%s         %s       %s      %s\n", des_list[i], Tt_str, Tp_str, Delay_str);
    }
    printf("-------------------------------------\n");

    

    memset(result_str,0,60000);

    // send to cilent
    for (int i = 0; i < atoi(numNode); ++i)
    {
        strcat(result_str, des_list[i]);
        strcat(result_str, "         ");
        strcat(result_str, minLen_list[i]);
        strcat(result_str, "        ");
        strcat(result_str, Tt_list[i]);
        strcat(result_str, "        ");
        strcat(result_str, Tp_list[i]);
        strcat(result_str, "        ");
        strcat(result_str, Delay_list[i]);
        strcat(result_str, "\n");

    }



// printf("des_list[0] is %s des_list[1] is %s des_list[2] is %s des_list[3] is %s",des_list[0],des_list[1],des_list[2],des_list[3]);

    if (send(new_fd, result_str, strlen(result_str), 0) == -1){
        perror("tcp send");
    }

    printf("The AWS has sent calculated delay to client using TCP over port <24387>.\n");
    freeaddrinfo(servinfo);
    close(thesockfd);

    return 0;
}


// it is UDPcommunicateWithServerA
//from https://beej.us/guide/bgnet/html/#client-server-background
int UDPcommunicateWithServerA(int new_fd, char *mapId, char *srcVertex,char *size){
    int thesockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    int numbytes;
    char *myServer;
    myServer = PORTA;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(IPADDRESS, PORTA, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((thesockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("AWS talker: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "AWS talker: failed to create socket\n");
        return 2;
    }

    if ((numbytes = sendto(thesockfd, mapId, strlen(mapId), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("AWS talker: sendto");
        exit(1);
    }

    if ((numbytes = sendto(thesockfd, srcVertex, strlen(srcVertex), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("AWS talker: sendto");
        exit(1);
    }
    freeaddrinfo(servinfo);
    printf("The AWS has sent map ID and starting vertex to server A using UDP over port <%s>”\n", PORTUDP);

    char numNode_str[3];
    if ((numbytes = recvfrom(thesockfd, numNode_str, 3, 0, NULL, NULL) )== -1) {
        perror("AWS: recvfrom");
        exit(1);
    }
    numNode_str[numbytes] = '\0';

    int numNode = atoi(numNode_str);
    char des[300];
    char minLen[3000];
    char des_list[numNode][300];
    char minLen_list[numNode][3000];

    char proSpeed_str[3000];
    char tranSpeed_str[3000];

    if ((numbytes = recvfrom(thesockfd, proSpeed_str, 3000, 0, NULL, NULL) )== -1) {
        perror("AWS: recvfrom");
        exit(1);
    }
    proSpeed_str[numbytes] = '\0';

    if ((numbytes = recvfrom(thesockfd, tranSpeed_str, 3000, 0, NULL, NULL) )== -1) {
        perror("AWS: recvfrom");
        exit(1);
    }
    tranSpeed_str[numbytes] = '\0';

    for (int i = 0; i < numNode; ++i)
    {
        if ((numbytes = recvfrom(thesockfd, des, 300, 0, NULL, NULL) )== -1) {
            perror("AWS: recvfrom");
            exit(1);
        }
        des[numbytes] = '\0';
        strcpy(des_list[i], des);

        if ((numbytes = recvfrom(thesockfd, minLen, 3000, 0, NULL, NULL) )== -1) {
            perror("AWS: recvfrom");
            exit(1);
        }
        minLen[numbytes] = '\0';
        strcpy(minLen_list[i], minLen);
    }

    printf("The AWS has received shortest path from server A:\n-----------------------------\nDestination     Min Length\n-----------------------------\n");
    for (int i = 0; i < numNode; i++) {
        printf("%s               %s\n", des_list[i], minLen_list[i]); 
    }
    printf("-------------------------------------------\n");
    close(thesockfd);

    sendToServerB( new_fd, size, numNode_str, des_list, minLen_list,proSpeed_str,tranSpeed_str);
    
    return 0;
}
// it is UDPcommunicateWithServerA end

// from https://beej.us/guide/bgnet/html/#client-server-background
int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

// set sockfd
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    //Clear dead process
    struct sigaction sa;
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
// set sockfd end

// set Tcp
    printf("The AWS is up and running.\n");

    int msgSize;
    char buf[1000];

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);

        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener

            if ((msgSize = recv(new_fd, buf, sizeof(buf), 0) ) == -1) {
                perror("AWS: recv error map");
                exit(1);
            }
            buf[msgSize] = '\0';

            char delim[] = " ";
            char *ptr = strtok(buf, delim);
            char *mapId = ptr;
            ptr = strtok(NULL, delim);

            char *srcVertex = ptr;
            ptr = strtok(NULL, delim);

            //Filesize is Long
            char *size = ptr;
            ptr = strtok(NULL, delim);

            printf("The AWS has received map ID <%s>, start vertex <%s> and file size <%s> from the client using TCP over port <%s>\n",
                            mapId, srcVertex, size, PORT);
            
// set Tcp end

// set UDP A
            int msgC = UDPcommunicateWithServerA(new_fd, mapId,srcVertex,size);
// set UDP A end

            close(new_fd);
            exit(0);

        }    
        close(new_fd);  // parent doesn't need this
    } 

    return 0;
}
