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

#define PORT "24387" // the port client will be connecting to 
#define IPADDRESS "127.0.0.1" // local IP address

#define MAXDATASIZE 60000 // max number of bytes we can get at once

// https://beej.us/guide/bgnet/html/#client-server-background
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if(argc != 4) {
		fprintf(stderr, "ERROR : You should use ./client <MAP ID> <Source Vertex Index> <File Size>.\n");
		exit(1);
	}

    int i, v = 0, size = argc - 1;
    char str[30000];
    for(i = 1; i < 4; i++) {
        strcat(str, argv[i]);
        strcat(str, " ");
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(IPADDRESS, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("The client is up and running\n");

    freeaddrinfo(servinfo); // all done with this structure

    // send mapId, 
    if ((numbytes = send(sockfd, str, strlen(str), 0) ) == -1) {
		perror("send mapId");
		exit(1);
	}

    printf("The client has sent query to AWS using TCP: start vertex <%s>; map <%s>; file size <%s>.\n", argv[2], argv[1], argv[3]);
    printf("The client has received results from AWS:\n----------------------------------------------\nDestination    Min Length          Tt             Tp             Delay\n----------------------------------------------\n");

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
        perror("recv");
        exit(1);
    }
    buf[numbytes] = '\0';

    printf("%s", buf);

    printf("------------------------------------\n");
    


    close(sockfd);

    return 0;
}
