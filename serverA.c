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

#define PORT "21387"
#define LOCALHOST "127.0.0.1"
#define BUFFSIZE 65500
#define MAXMAP_SIZE 10

int numNode = 0;
// Learned Dirkstra Algorithm from https://www.geeksforgeeks.org/dijkstras-shortest-path-algorithm-greedy-algo-7/
int minDistance(double dist[], bool visited[]) { 

    double min = DBL_MAX;
    int min_index; 
  
    for (int v = 0; v < numNode; v++){ 
        if (visited[v] == false && dist[v] <= min) 
        {	
        	min = dist[v];
        	min_index = v; 
        }
    }
  
    return min_index; 
} 

double * dijkstra(double map[10][10], int srcVex){
	static double dist[10];
    bool visited[numNode];

    for (int i = 0; i < numNode; i++){
    	dist[i] = DBL_MAX;
    	visited[i] = false; 
    }

    // Find the src node
    dist[srcVex] = 0; 
  	int count = 0;

    // Find shortest path for all vertices 
    while (count < numNode) {  
        int u = minDistance(dist, visited); 
        visited[u] = true; 
        count++;
        for (int v = 0; v < numNode; ++v)
        {
        	if(dist[u] != DBL_MAX && !visited[v] && map[u][v] > 0.0 && dist[v] > dist[u] + map[u][v] ){
        		dist[v] = dist[u] + map[u][v];
        	}
        }
    } 
    return dist;
}

// from https://beej.us/guide/bgnet/html/#client-server-background
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
    	return &(((struct sockaddr_in*)sa)->sin_addr);
    }

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int convertVs(int ** nodes,int v1,bool isInput){
	int converted = -1;

	for(int i = 0; i < numNode; i++){
		if((*nodes)[i] == v1){
			converted = i;
		}
	}

	if(converted == -1 && !isInput){
		*nodes = realloc(*nodes, sizeof(int) * (numNode+1));
		(*nodes)[numNode] = v1;
		converted = numNode;
		numNode++;
	}
	return converted;
}
void addToNodes(int ** nodesInMap, int v, int * num_Node){
	int flag = -1;
	for(int i = 0; i < * num_Node; i++){
		if( (*nodesInMap)[i] == v){
			flag = i;
		}
	}

	if(flag == -1){
		(*nodesInMap) = realloc(*nodesInMap, sizeof(int) * ((*num_Node)+1));
		(*nodesInMap)[(*num_Node)] = v;
		(*num_Node)++;
	}
}

int main (int argc, char const *argv[]){
	// from https://beej.us/guide/bgnet/html/#client-server-background
	int sockfd,rv,numbytes;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    char ID_buf[BUFFSIZE];
    socklen_t addr_len;
    //char s[INET6_ADDRSTRLEN];
    char outputD_buf[BUFFSIZE];
    char srcNode_buf[BUFFSIZE];

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

	printf("The Server A is up and running using UDP on port <%s>.\n",PORT);

	FILE* file_NumMap = fopen("map.txt", "r");
	int numMaps = 0;
	char line3[256]; 

	while ( fgets(line3, sizeof(line3), file_NumMap) ) {
		if (( line3[0]>='a' && line3[0]<='z' ) || (line3[0]>='A' && line3[0]<='Z'))
		{
			numMaps++;
		}
	}
	fclose(file_NumMap);

	// Construction for all maps
	FILE* file2 = fopen("map.txt", "r");
    char line2[256];     
    int numEdge = 0, num_Node = 0;
    char ID;
	printf("The Server A has constructed a list of <%d> maps:\n",numMaps);
	printf("-------------------------------------------\nMap ID    Num Vertices   Num Edges\n-------------------------------------------\n");

    while ( fgets(line2, sizeof(line2), file2) ) {
    	int* nodesInMap = malloc(0 * sizeof(int));

        if ( ( line2[0]>='a' && line2[0]<='z' ) || (line2[0]>='A' && line2[0]<='Z') ){
        	ID = line2[0];

        	// Jump the two speeds
        	fgets(line2, sizeof(line2), file2);    	// printf("I scaned 2:%s\n",line2 );
        	fgets(line2, sizeof(line2), file2);    	// printf("I scaned 3:%s\n",line2 );

        	//Get nodes
        	while(fgets(line2, sizeof(line2), file2) && 
        		!( (line2[0]>='a' && line2[0]<='z' ) || (line2[0]>='A' && line2[0]<='Z'))){

        		char de[] = " ";
	            char *ptr = strtok(line2, de);
	            char *n1 = ptr;
	            ptr = strtok(NULL, de);
	            char *n2 = ptr;
	            ptr = strtok(NULL, de);
	            

            	addToNodes(&nodesInMap, atoi(n1), &num_Node);
            	addToNodes(&nodesInMap, atoi(n2), &num_Node);
        		numEdge++;
        	}

        	fseek(file2, -strlen(line2), SEEK_CUR); 

        	printf("%c       %d       %d\n",ID,num_Node,numEdge);
        	numEdge = 0;
        	num_Node = 0;
        	free(nodesInMap);


        } 
    }

    printf("-------------------------------------------\n");
    fclose(file2);


	while(1) {
			
		if ((numbytes = recvfrom(sockfd, ID_buf, BUFFSIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
	        perror("recvfrom");
	        exit(1);
	    }
	    ID_buf[numbytes] = '\0';

	    if ((numbytes = recvfrom(sockfd, srcNode_buf, BUFFSIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
	        perror("recvfrom");
	        exit(1);
	    }
	    srcNode_buf[numbytes] = '\0';
	    printf("The Server A has received input for finding shortest paths: starting vertex <%s> of map <%s>. \n", srcNode_buf, ID_buf);

	//Calculate Dirkstra
	    // Read map file
	    FILE* file = fopen("map.txt", "r"); 
	    char line[256];  // printf("%c\n", ID_buf[0]);
	    double proSpeed, tranSpeed; 

	    while ( fgets(line, sizeof(line), file) ) {
	        if (line[0] == ID_buf[0] )
	        {
	        	break;
	        } 
	    }

	    // Get proSpeed and transportation speed from map.txt
	    fgets(line, sizeof(line), file);
	    sscanf(line, "%lf", &proSpeed);
	    fgets(line, sizeof(line), file);
	    sscanf(line, "%lf", &tranSpeed); 


	    int v1,v2, convertedV1,convertedV2;
	    double dis;
	    double map[10][10];
	    int* nodes = malloc(0 * sizeof(int));  		

	    for (int i = 0; i < 10; i++){
	    	for (int j = 0; j < 10; j++){
	  			map[i][j] = 0;
	  		}
	    }

	    while(fgets(line, sizeof(line), file) ){

	    	if ( (line[0]>='a' && line[0]<='z')  || (line[0]>='A' && line[0]<='Z'))
	        {
	        	break;
	        }
	        else{
	        	// This is to separate the line by space for get the edge
	            char de[] = " ";
	            char *ptr = strtok(line, de);
	            char *n1 = ptr;
	            ptr = strtok(NULL, de);
	            char *n2 = ptr;
	            ptr = strtok(NULL, de);
	            char *weight = ptr;
	            ptr = strtok(NULL, de);

	            // Add edge
	            v1 = atoi(n1);
	            v2 = atoi(n2);
				sscanf(weight, "%lf", &dis);

	        	convertedV1 = convertVs(&nodes,v1,false);
	        	convertedV2 = convertVs(&nodes,v2,false);
	        	map[convertedV1][convertedV2] = dis;
	        	map[convertedV2][convertedV1] = dis;        				
	        }

	    }

	    // convert src node 
	    int srcVex = atoi(srcNode_buf);
	    srcVex = convertVs(&nodes,srcVex,true);
		double *dist;

	    if(srcVex == -1){
	    	perror("This node is not in the map");
	    }else{
	    	dist = dijkstra(map, srcVex);
	    }

	    printf("The Server A has identified the following shortest paths:\n-----------------------------\nDestination     Min Length\n-----------------------------\n");
	    
	    //sort nodes[i] dist[i]
	    for(int i = 0; i < numNode-1; i++){
	    	for(int j = i; j < numNode-1; j++){
	    		if(nodes[i] > nodes[i+1]){
	    			int tmp = nodes[i];
	    			nodes[i] = nodes[i+1];
	    			nodes[i+1] = tmp;

	    			double disTmp= dist[i];
	    			dist[i] = dist[i+1];
	    			dist[i+1] = disTmp;
	    		}
	    	}
	    }


	    int numLen = 0;
	    if(numNode == 10){
	    	numLen = 2;
	    }
	    else{
	    	numLen = 1;
	    }

	    char numNode_str[2];
	    char des_str[300];
	    char minLen_str[3000];
	    char proSpeed_str[3000];
	    char tranSpeed_str[3000];

	    sprintf(numNode_str, "%d", numNode-1);//int to string
	    numbytes = sendto(sockfd,numNode_str,numLen,0,(struct sockaddr *)&their_addr, addr_len);

	    sprintf(proSpeed_str, "%lf", proSpeed);// double to string
	    sprintf(tranSpeed_str, "%lf", tranSpeed);// double to string

	    numbytes = sendto(sockfd,proSpeed_str,strlen(proSpeed_str),0,(struct sockaddr *)&their_addr, addr_len);
	    numbytes = sendto(sockfd,tranSpeed_str,strlen(tranSpeed_str),0,(struct sockaddr *)&their_addr, addr_len);


	    for (int i = 0; i < numNode; ++i)
	    {
	        int dis_int = *(dist + i);
	    	if( dis_int == 0){
	    		continue;
	    	}
	    	sprintf(des_str, "%d", nodes[i]);
	        numbytes = sendto(sockfd,des_str,strlen(des_str),0,(struct sockaddr *)&their_addr, addr_len);
	        
	        sprintf(minLen_str, "%d", dis_int);
	        numbytes = sendto(sockfd,minLen_str,strlen(minLen_str),0,(struct sockaddr *)&their_addr, addr_len);
	        printf("%d               %d\n", nodes[i], dis_int); 

	    }

	    printf("-------------------------------------------\n");

	    //numbytes = sendto(sockfd,outputD_buf,strlen(outputD_buf),0,(struct sockaddr *)&their_addr, addr_len);
	    printf("The Server A has sent shortest paths to AWS.\n" );
	    
	    free(nodes);
	    fclose(file);
	    numNode = 0;



	}
    
    return 0;

}