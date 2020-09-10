# Socket project  
	Name: Huijun Wang    
	ID: 1560035387    

##. What I have done
	In this project, I completed building socket with TCP connection between client and aws, UDP connection between client and the two servers (serverA and serverB). The completness of the project is based on all requirements in the instruction.
##. The unit of Tt Tp Delay
	I used ms as the unit of the number. So the number is 1000 of the number of seconds.
 
## T0 run it
        map file has to be "map.txt"
        make all 
        make serverA
        make serverB
        make aws
        ./client <map ID> <node> <file size>
	
## Code files  
	***aws.c*** 
	The aws boots up at the first step. 
	When it recieve the request from client through TCP connection, it send the map ID and start node to server A through a UDP connection. After server A sends the result back, it send the result with the file size (which is from client) to server B for delay. After it received the delays from server B, AWS send the result back to client and wiating for next query.
	
	***client.c***  
	The client gains the map ID, start node and file size from the user input. It then send a query through TCP connection with those information to aws and waiting for the delay as in the instruction. After AWS send the result back, it prints the result out and ends the program.
	 
	***serverA.c*** 
	The serverA boots up at the first step. It constructs the maps from reading the map.txt file. Waiting for query from aws since then. After it received the query from AWS which is asking for min length, server A extracts the information in the query and do the calculation by using Dirkstra Algorithm and send the result back to AWS.
	
	***serverB.c*** 
	The serverB boots up at the first step. It waits for querys from AWS. When AWS sends query to it, it would do the calculation for delay and send the result back to AWS through UDP connection.
 


## Format of all the messages exchanged
	Step1: Boot up serverA, serverB and aws

	Step2: Client send the query
	-client-
		<map ID> <starting node> <file size>

	Step3: AWS recieve the query
	-aws-
		The AWS has received map ID <A>, start vertex <6> and file size <1888> from the client using TCP over port <24387>

	Step4: AWS send the map ID and node to Server A
	<map ID>
	<starting node>

	Step5: Server A recieve the query and calculate the min length for other nodes. Then Server A send the result to AWS.
	-serverA-

		<Destination1>     <Min Length1>
		<Destination2>     <Min Length2>
		...					...

	Step 6: AWS recieve the result from server A and send out to server B with file size,propagation speed and transmission speed.
		<file size>
		<propagation speed> 
		<transmission speed> 
		<Destination1>     <Min Length1>
		<Destination2>     <Min Length2>
		...					...

	Step 7: After calculation of delay in server B and send the result to AWS.
	-serverB-
		<Destination1>     <Min Length1> <Tt1> <Tp1> <Delay1>
		<Destination2>     <Min Length2> <Tt2> <Tp2> <Delay2>
		...	
						...
	Step 8:AWS recieved result from server B and send to client.
	-aws-
		<Destination1>     <Min Length1> <Tt1> <Tp1> <Delay1>
		<Destination2>     <Min Length2> <Tt2> <Tp2> <Delay2>
		...
							...
	Step 9: Client received the result and print it out. Close the program.
	
  

## Idiosyncrasy of my project.  
	So far so good. If starting node is not in the map, serverA would ends.

## Reused Code
	When I build TCP and UDP connections, I copied code from Beej's website. And for the dijkstra algorithm, I studied it from wiki and geeksforgeeks.  
	https://beej.us/guide/bgnet/html/#sendman
	https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm
	https://www.geeksforgeeks.org/dijkstras-shortest-path-algorithm-greedy-algo-7/


