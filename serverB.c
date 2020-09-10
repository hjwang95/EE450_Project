#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <stdbool.h>
#include  <float.h>
#include <string.h>
#include <math.h> 

#define PORT "22387"
#define LOCALHOST "127.0.0.1"
#define BUFFSIZE 3000

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main (int argc, char const *argv[]){
    // from https://beej.us/guide/bgnet/html/#client-server-background
    int sockfd,rv,numbytes;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    //char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);
    addr_len = sizeof their_addr;

    printf("The Server B is up and running using UDP on port <%s>.\n",PORT);

    char fileSize_str[3000], proSpeed_str[3000],tranSpeed_str[3000];
    double proSpeed, tranSpeed, fileSize, Tt,Tp, minLen_dbl;
    char numNode_str[3];
    int numNode;

    char des[300],minLen[3000];

    while(1) {
        if ((numbytes = recvfrom(sockfd, fileSize_str, 3000 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        fileSize_str[numbytes] = '\0';
        sscanf(fileSize_str, "%lf", &fileSize);

        if ((numbytes = recvfrom(sockfd, proSpeed_str, 3000 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        proSpeed_str[numbytes] = '\0';
        sscanf(proSpeed_str, "%lf", &proSpeed);

        if ((numbytes = recvfrom(sockfd, tranSpeed_str, 3000 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        tranSpeed_str[numbytes] = '\0';
        sscanf(tranSpeed_str, "%lf", &tranSpeed);


        if ((numbytes = recvfrom(sockfd, numNode_str, 3 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        numNode_str[numbytes] = '\0';
        numNode = atoi(numNode_str);

        char des_list[numNode][300];
        char minLen_list[numNode][3000];

        printf("The Server B has received data for calculation:\n* Propagation speed: <%s> km/s;\n* Transmission speed <%s> Bytes/s;\n", proSpeed_str,tranSpeed_str);
        for (int i = 0; i < numNode; ++i)
        {
            if ((numbytes = recvfrom(sockfd, des, 300, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                perror("recvfrom");
                exit(1);
            }
            des[numbytes] = '\0';
            strcpy(des_list[i], des);

            if ((numbytes = recvfrom(sockfd, minLen, 3000, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                perror("recvfrom");
                exit(1);
            }
            minLen[numbytes] = '\0';
            strcpy(minLen_list[i], minLen);

            printf("* Path length for destination <%s>: <%s>;\n",des, minLen);
        }

        printf("The Server B has finished the calculation of the delays:\n------------------------\nDestination Delay\n------------------------\n");

        for (int i = 0; i < numNode; ++i)
        {
            //send Tt and Tp 
            sscanf(minLen_list[i], "%lf", &minLen_dbl); // string to double
            Tp = minLen_dbl / proSpeed *1000.00;
            Tt = fileSize / (tranSpeed * 8.00) * 1000.00;
            char Tp_str[3000], Tt_str[3000], Delay_str[3000];

            double delay = Tp + Tt;
            sprintf(Tp_str, "%.2f", Tp);// double to string
            sprintf(Tt_str, "%.2f", Tt);// double to string
            sprintf(Delay_str, "%.2f", delay);// double to string

            if ((numbytes = sendto(sockfd,Tp_str,strlen(Tp_str),0,(struct sockaddr *)&their_addr, addr_len) )== -1) {
                perror("ServerB talker: sendto 1");
                exit(1);
            }

            if ((numbytes = sendto(sockfd,Tt_str,strlen(Tt_str),0,(struct sockaddr *)&their_addr, addr_len) )== -1) {
                perror("ServerB talker: sendto 2");
                exit(1);
            }

            if ((numbytes = sendto(sockfd,Delay_str,strlen(Delay_str),0,(struct sockaddr *)&their_addr, addr_len) )== -1) {
                perror("ServerB talker: sendto 3");
                exit(1);
            }

            printf("%s      %.2f\n", des_list[i], delay);
  
        }

        printf("------------------------\n");

        printf("The Server B has finished sending the output to AWS\n");

    }









    return 0;
}
