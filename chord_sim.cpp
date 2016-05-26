//auther Xiaobin Zheng, xzheng19, 668374748
//04-25-16
#include "chord_sim.h"


/*Config file parser, to extact parameters. eg, delay range, server ports*/
void configParser(const char *filename)
{
	FILE * file = fopen(filename, "r");
	if (file == NULL)
        exit(EXIT_FAILURE);

	char * line = NULL;
	size_t buf_len = 0;
	int index = -1;
	while (getline(&line, &buf_len, file) != -1)
	{
		line[strlen(line)-1] =0;
		//printf("%s\n", line);
		if (index == -1)
		{
			 char *ret_s = strstr(line, "(");
			 char *ret_e = strstr(line, ")");
			 (*ret_e) = 0;
			 min_delay = atoi(ret_s+1);
			 ret_s = strstr(ret_e+1, "(");
			 ret_e = strstr(ret_e+1, ")");
			 (*ret_e) = 0;
			 max_delay = atoi(ret_s+1);
			 index++;
		}
		else 
		{
			base_port = atoi(line);
			//printf("parser base prot is %d\n", base_port);
		}
	}
}
//update all finger table for all existing nodes
void update_all_ft(int node_num){
	for (int i =0; i < 256; i++){
		if (i!=node_num)
		{
			char * node_port = node_to_port(i);
			figure_node_update(node_port);
		}
		
	}
}
//update a particualr node with given node port nummber
int figure_node_update(char * port){
	//printf("temp is %s\n", port);
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1)
		return 0;
    struct addrinfo hints, *result = NULL;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int get_check = getaddrinfo("127.0.0.1", port, &hints, &result);
    if (get_check !=0)
    {
    	//puts("call here");
    	close(sock_fd);
		sock_fd = -1;
    	return 0;
        // fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(get_check));
    }
    if ( -1 == connect(sock_fd, result->ai_addr, result->ai_addrlen))
   	{
   		// puts("P does not exist");
   		close(sock_fd);
		sock_fd = -1;
   		return 0;
   	}
   	//puts("should not be call");

   	char message[7];
   	strcpy(message, "update");
   	message[6]=0;
   	write(sock_fd, message, strlen(message));
   	close(sock_fd);
	sock_fd = -1;
   	return 1;
}

//return 0 if connection false, else return 1;
int connect_to_node(char * port){
	//printf("temp is %s\n", port);
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1)
		return 0;
    struct addrinfo hints, *result = NULL;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int get_check = getaddrinfo("127.0.0.1", port, &hints, &result);
    if (get_check !=0)
    {
    	//puts("call here");
    	close(sock_fd);
		sock_fd = -1;
    	return 0;
        // fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(get_check));
    }
    if ( -1 == connect(sock_fd, result->ai_addr, result->ai_addrlen))
   	{
   		//puts("P does not exist");
   		close(sock_fd);
		sock_fd = -1;
   		return 0;
   	}
   	close(sock_fd);
	sock_fd = -1;
   	return 1;
}

//create finger table for a given node port number
int * create_ft(int node_id)
{
	int * ft = (int*) calloc(8, sizeof(int));
	for(int i =0; i < m; i++)
	{
		//puts("here now");
		ft[i] = (node_id + (int)pow(2, i)) % (int)pow (2, 8);
		int connected_node = 0;
		for (int x = ft[i]; x < 256; x++)
		{
			int cap_x = base_port+x;
			char temp[5];
			memset(temp, 0, 5);
			snprintf(temp, 5, "%d", cap_x);
			if (1 == connect_to_node(temp))
			{
				connected_node = x;
				break;
			}
		}
		ft[i] = connected_node;
	}
	return ft;
}

//helper function to convert node to node port number  
char * node_to_port(int node_num){
	char * temp = (char*) calloc(1,5);
	int x = base_port+node_num;
	snprintf(temp, 5, "%d", x);
	return temp;
}

//unisend helper function for sending local key to sucessor node for backup
int backup_key_send(char*message, char*serverport)
{
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1)
		return 0;
    struct addrinfo hints, *result = NULL;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int get_check = getaddrinfo("127.0.0.1", serverport, &hints, &result);
    if (get_check !=0)
    {
    	//puts("call here");
    	close(sock_fd);
		sock_fd = -1;
    	return 0;
        // fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(get_check));
    }
    if ( -1 == connect(sock_fd, result->ai_addr, result->ai_addrlen))
   	{
   		// puts("P does not exist");
   		close(sock_fd);
		sock_fd = -1;
   		return 0;
   	}
   	//puts("should not be call");
   	//printf("send backup message is %s\n", message);
   	write(sock_fd, message, strlen(message));
   	close(sock_fd);
	sock_fd = -1;
   	return 1;
}

//iterate all existing node to update all the backup key;
void back_up_request(std::list<int>key, int node_num, int next_node){
	char *serverport = node_to_port(next_node);
	char *message = (char*) calloc(1000, 1);
	strcpy(message, "backup");
	char temp[5];
	memset(temp, 0, 5);
	snprintf(temp, 5, "%d", node_num);
	strcat(message, temp);

	for (std::list<int>::iterator it = key.begin(); it != key.end(); it++)
	{
		strcat(message, " ");
		memset(temp, 0, 5);
		snprintf(temp, 5, "%d", *it);
		strcat(message,temp);
	}
	//printf("sending back up key from node %d to node %d key is %s\n", node_num, next_node, message);
	backup_key_send(message, serverport);
}

//find predecessor of given node
void find_predecessor(int node_num, int * predecessor){
	if (node_num == 0)
	{
		for (int i = 255; i >0; i--)
		{
			char *serverport = node_to_port(i);
			if (1==connect_to_node(serverport))
			{
				//printf("find node %d predecessor is %d\n", node_num, *predecessor);
				*predecessor=i;
				return;
			}
		}
		
		*predecessor = 0;
		//printf("find node %d predecessor is %d\n", node_num, *predecessor);
		return;
	}
	else
	{
		for (int i = node_num-1; i > -1; i--)
		{
			char *serverport = node_to_port(i);
			if (1==connect_to_node(serverport))
			{
				//printf("find node %d predecessor is %d\n", node_num, *predecessor);
				*predecessor=i;
				return;
			}
		}
		// *predecessor = 0;
		// printf("find node %d predecessor is %d\n", node_num, *predecessor);
		return;
	}	
}

//send helper function for find command
char * find_unisend(char * message, char * serverport){
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct addrinfo hints, *result = NULL;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int get_check = getaddrinfo("127.0.0.1", serverport, &hints, &result);
    if (get_check !=0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(get_check));
        close(sock_fd);
		sock_fd = -1;
   		return 0;
    }
    if ( -1 == connect(sock_fd, result->ai_addr, result->ai_addrlen))
   	{
   		//if (flag==1)
   		puts("P does not exist");
   		puts("Ready for next command:");
   		close(sock_fd);
		sock_fd = -1;
   		return 0;
   	}
   	usleep(((rand() % (max_delay + 1 - min_delay)) + min_delay) * 1000);
   	write(sock_fd, message, strlen(message));

   	char * buffer = (char*) calloc(1, MESG_SIZE);
   	//memset(buffer, 0, MESG_SIZE);
   	int len = read(sock_fd, buffer, MESG_SIZE-1);
   	if (len !=-1)
   	{
   		return buffer;
   	}
   	perror("find_unisend error");
   	return NULL;
}

//send helper function for crash command
void crash_unisend(char* message, char *serverport){
	//printf("crash node serverport is %s and message is %s\n", serverport, message);
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct addrinfo hints, *result = NULL;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int get_check = getaddrinfo("127.0.0.1", serverport, &hints, &result);
    if (get_check !=0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(get_check));
        close(sock_fd);
		sock_fd = -1;
   		return;
    }
    if ( -1 == connect(sock_fd, result->ai_addr, result->ai_addrlen))
   	{
   		//if (flag==1)
   		puts("P does not exist");
   		close(sock_fd);
		sock_fd = -1;
   		return;
   	}
   	usleep(((rand() % (max_delay + 1 - min_delay)) + min_delay) * 1000);
   	write(sock_fd, message, strlen(message));
   	close(sock_fd);
	sock_fd = -1;
   	return;
}
//find given key in local key list. return 0 if found, otherwise 1.
int find_key(int key, std::list<int> key_list){
	for (std::list<int>::iterator it = key_list.begin(); it != key_list.end(); it++)
	{
		if (key == *it)
			return 0;
	}
	return 1;

}
//debuging function call to print out. 
void print_ft(int node_num, int * node_ft){
	for (int i = 0; i<8;i++)
		printf("node_num is %d ft is %d\n", node_num, node_ft[i]);
}

//send helpfer function show show and find command 
int unisend(char* message, char* serverport, int flag)
{
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct addrinfo hints, *result = NULL;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int get_check = getaddrinfo("127.0.0.1", serverport, &hints, &result);
    if (get_check !=0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(get_check));
        close(sock_fd);
		sock_fd = -1;
   		return 0;
    }
    if ( -1 == connect(sock_fd, result->ai_addr, result->ai_addrlen))
   	{
   		if (flag==1)
   			puts("P does not exist");
   		close(sock_fd);
		sock_fd = -1;
   		return 0;
   	}
   	//puts("still send ");
   	usleep(((rand() % (max_delay + 1 - min_delay)) + min_delay) * 1000);
   	write(sock_fd, message, strlen(message));

   	char buffer[MESG_SIZE];
   	memset(buffer, 0, MESG_SIZE);
   	int len = read(sock_fd, buffer, MESG_SIZE-1);
   	if (len !=-1)
   	{
   		buffer[strlen(buffer)]=0;
   		printf("%s\n", buffer);
   	}
   	close(sock_fd);
	sock_fd = -1;
   	return 1;
}


//Master node, each node is a pthread handles request from other nodes or client
void* node_init(void * arg){
	/*init all finger table, key for Node 0*/
	int node_num = atoi((char*) arg);
	//printf("node %d is joining the chord\n", node_num);
	pthread_detach(pthread_self()); // no join() required
	std::list<int> local_key;

	//Node 0 only 
	if (node_num == 0)
	{
		for (int i = 0; i < 256; i++)
		local_key.push_back(i);
	}
	int * node_ft = create_ft(node_num);
	std::list<int> bt_key;
	int successor = 0;
	int predecessor = 0;
	//server setup
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    struct addrinfo hints, *result;
    result = NULL;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    char *node_port = node_to_port(node_num);
    //printf("new node port %s\n", node_port);
    int get_check = getaddrinfo(NULL, node_port, &hints, &result);
    if (get_check !=0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(get_check));
        perror("get address info error");
        exit(1);
    }
    if(bind(sock_fd, result->ai_addr, result->ai_addrlen)!=0)
    {
        perror("bind() error\n");
        exit(1);
    }
    if(listen(sock_fd, 10) !=0 )
    {
        perror("listen() error\n");
        exit(1);
    }

    //Joining node that is not Node 0
    if (node_num != node_ft[0])
    {
    	char * send_port = node_to_port(node_ft[0]);
    	char *message=(char*) calloc(1, 20);
		strcpy(message, "key");
		strcat(message, (char*)arg);
		//message[strlen(message)+1] = 0;

		int sock_fd_key = socket(AF_INET, SOCK_STREAM, 0);
	    struct addrinfo hints, *result = NULL;
	    memset(&hints, 0, sizeof(struct addrinfo));
	    hints.ai_family = AF_INET;
	    hints.ai_socktype = SOCK_STREAM;
	    int get_check = getaddrinfo("127.0.0.1", send_port, &hints, &result);
	    if (get_check !=0)
	    {
	        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(get_check));
	    }
	    if ( -1 == connect(sock_fd_key, result->ai_addr, result->ai_addrlen))
	   	{
	   		puts("P does not exist");
	   		return 0;
	   	}
	   	write(sock_fd_key, message, strlen(message));
		//printf("key message is %s\n", message);
		char buffer[MAXBUFLEN];
		memset(buffer, 0, MAXBUFLEN);

		//presuccsor node following by keys such 
		//123 1 2 3 4 keys: 1,2,3,4,
    	int len = read(sock_fd_key, buffer, MESG_SIZE-1);
    	if (len !=0)
    	{
    		buffer[strlen(buffer)] = '\0';
    		//printf("key received from successor is %s\n", buffer);
    		char *temp = strstr(buffer, " ");
    		*temp = 0;
    		successor = node_ft[0];
    		predecessor = atoi(buffer);
    		char * pointer = temp+1;
    		temp = strstr(pointer, " ");
	    	while(temp !=NULL)
	    	{
	    		*temp = 0;
	    		local_key.push_back(atoi(pointer));
	    		pointer = temp+1;
	    		temp = strstr(pointer, " ");
	    	}
	    	local_key.push_back(atoi(pointer));
	    	close(sock_fd_key);
			sock_fd_key = -1;
			update_all_ft(node_num);
			back_up_request(local_key, node_num, node_ft[0]);
    	}

    }
    //puts("ready to accept");
    while(1)
    {
    	int fd= accept(sock_fd, NULL, NULL);
    	//printf("node %d is accepting the connection, fd is %d\n", node_num, fd );
    	if (fd != -1)
    	{
    		char buffer[MAXBUFLEN];
    		memset(buffer, 0, MAXBUFLEN);

    		int len = read(fd, buffer, MESG_SIZE-1);
	        if (len != 0)
	    	{
	    		buffer[strlen(buffer)] =0;
	    		//printf("node %d, imcoming message is %s\n", node_num, buffer);
	    		//update the finger table
	    		if (!strncmp(buffer, "update", 4))
	    		{
	    			//printf("node %d updating the ft\n", node_num);
	    			node_ft = create_ft(node_num);
	    			find_predecessor(node_num, &predecessor);
	    		}
	    		//new node joined, need to split my key set
	    		//key newnode; key3
	    		//key denotes for request key, and newnode denotes newly join node and become my precessor
	    		else if (!strncmp(buffer, "key", 3))
	    		{

	    			int new_predecessor = atoi(buffer+3);
	    			char *message = (char*) calloc(1,10000);
	    			char presucc[5];
	    			snprintf(presucc, 5, "%d", predecessor);
	    			strcpy(message, presucc);
	    			predecessor = new_predecessor;
	    			int zero_flag =0;
	    			if (local_key.front() ==node_num)
	    			{	
	    				local_key.pop_front();
	    				zero_flag =1;
	    			}
	    			while(local_key.front() <= predecessor)
	    			{
	    				strcat(message, " ");
	    				char temp[5];
	    				memset(temp, 0, 5);
	    				snprintf(temp, 5, "%d", local_key.front());
	    				strcat(message, temp);
	    				// bt_key.push_back(local_key.front());
	    				local_key.pop_front();
	    			}
	    			if (zero_flag==1)
	    				local_key.push_front(0);
	    			//printf("sent to precessor is %d key is %s\n", predecessor, message);
	    			write(fd, message, strlen(message));
	    			close(fd);
					fd = -1;
					//puts("key)");
	    		}
	    		else if(!strncmp(buffer, "init",4))
	    		{
	    			//puts("init call");
	    			char message[100];
	    			memset(message, 0, 100);
	    			strcpy(message, "Ready for next command");
	    			write(fd, message, strlen(message));
	    			close(fd);
					fd = -1;
	    		}
	    		//save the presuccor's key to my node, and send my keys to sussocor node
	    		else if(!strncmp(buffer, "backup",6))
	    		{
	    			bt_key.clear();
	    			char * start = buffer;
	    			char * end = strstr(start, " ");
	    			*end = 0;
	    			int enter_node = atoi(start+6);
	    			start = end +1;
	    			end = strstr(start, " ");
	    			while(end !=NULL)
	    			{
	    				*end = 0;
	    				bt_key.push_back(atoi(start));
	    				start = end+1;
	    				end = strstr(start, " ");
	    			} 
	    			bt_key.push_back(atoi(start));
	    			if (enter_node != node_num)
	    			{
	    				back_up_request(local_key, enter_node, node_ft[0]);    				
	    			}
	    			else 
	    			{
	    				char * message = (char*) calloc(1, 100);
	    				strcpy(message, "Ready for next command");
	    				send_back_client(message);
	    			}
	    				
	    			close(fd);
					fd = -1;
	    		}

	    		else if (!strncmp(buffer, "merge", 5))
	    		{
	    			char * temp = buffer;
	    			predecessor = atoi(temp+5);
	    			local_key.merge(bt_key);
	    			back_up_request(local_key, node_num, node_ft[0]);

	    		}
	    		//show command
	    		else if(!strncmp(buffer, "show", 4))
	    		{

	    			//my sucessor node is crashed
	    			int old_next = node_ft[0];
	    			node_ft = create_ft(node_num);
	    			if (old_next != node_ft[0]){
	    				char * serverport = node_to_port(node_ft[0]);
	    				char * message = (char *) calloc(1, 50);
	    				strcpy(message, "merge");
	    				char temp[5];
	    				snprintf(temp, 5, "%d", node_num);
	    				strcat(message, temp);
	    				crash_unisend(message, serverport);
	    			}
	    			//my predecessor node is crashed
	    			char * try_port = node_to_port(predecessor);
	    			if (0 == connect_to_node(try_port))
	    			{
	    				local_key.merge(bt_key);
	    				back_up_request(local_key, node_num, node_ft[0]);
	    			}

	    			char *message = (char*) calloc(1, 10000);
	    			strcpy(message, "node-indentifier ");
	    			strcat(message, (char*) arg);
	    			strcat(message, "\nFingerTable:");
	    			for (int i =0; i<8; i++)
	    			{
	    				char temp[5];
	    				snprintf(temp, 5, "%d", node_ft[i]);
	    				strcat(message, temp);
	    				if (i !=7)
	    					strncat(message,",",1);
	    			}
	    			strcat(message,"\nKeys:");
	    			std::list<int> local_key_cpy = local_key;
	    			while(local_key_cpy.size()!=0)
	    			{
	    				char temp[5];
	    				snprintf(temp, 5, "%d", local_key_cpy.front());
	    				strcat(message, temp);
	    				local_key_cpy.pop_front();
	    				strncat(message, " ", 1);
	    			}
	    			//print out back up key for debug purpose
	    			// strcat(message,"\nbt_Keys:");
	    			// std::list<int> local_key_cpy1 = bt_key;
	    			// while(local_key_cpy1.size()!=0)
	    			// {
	    			// 	char temp[5];
	    			// 	snprintf(temp, 5, "%d", local_key_cpy1.front());
	    			// 	strcat(message, temp);
	    			// 	local_key_cpy1.pop_front();
	    			// 	strncat(message, " ", 1);
	    			// }
	    			//strcat(message, "\nReady for next command");
	    			write(fd, message, strlen(message));
	    			close(fd);
					fd = -1;
	    		}

	    		//find 8 9
	    		else if (!strncmp(buffer, "find", 4))
	    		{
	    			//check if crashed
	    			//my successor node is crashed
	    			int old_next = node_ft[0];
	    			node_ft = create_ft(node_num);
	    			if (old_next != node_ft[0]){
	    				char * serverport = node_to_port(node_ft[0]);
	    				char * message = (char *) calloc(1, 50);
	    				strcpy(message, "merge");
	    				char temp[5];
	    				snprintf(temp, 5, "%d", node_num);
	    				strcat(message, temp);
	    				crash_unisend(message, serverport);
	    			}
	    			//my predecessor node is crashed
	    			char * try_port = node_to_port(predecessor);
	    			if (0 == connect_to_node(try_port))
	    			{
	    				local_key.merge(bt_key);
	    				back_up_request(local_key, node_num, node_ft[0]);
	    			}
	    			//now handle the request
	    			char *temp = strstr(buffer, " ");
	    			temp++;
					char *temp1 = strstr(temp, " ");
					temp1++;
					//printf("temp is %s\n", temp1);
					int key = atoi(temp1);
					//printf("key is %d\n", key);
					if (0 == find_key(key, local_key))
					{
						char message[100];
						memset(message, 0, 100);
						sprintf(message, "Node %d contains %d", node_num, key);
						strcat(message, "\nReady for next command");
						write(fd, message,strlen(message));
					}
					else{
						char *serverport = node_to_port(node_ft[0]);
						char * message = find_unisend(buffer, serverport);
						write(fd, message, strlen(message));
					}
	    		}
	    		else if (!strncmp(buffer, "crash", 5))
	    		{
	    			//printf("node %d is crashed\n", node_num);
	    			char message[100];
	    			memset(message, 0, 100);
	    			strcpy(message, "Ready for next command");
	    			write(fd, message, strlen(message));
	    			close(fd);
					fd = -1;
	    			close(sock_fd);
					sock_fd = -1;
					pthread_exit(NULL);
	    			return NULL;
	    		}
	    	}
    	}
    }
    //printf("node %d is out of chord\n", node_num);
    return NULL;
}


int main(int argc, char *argv[]){
	//pthread_mutex_init(&join_mutex, NULL);
	//pthread_cond_init(&join_cv, NULL);
	configParser(argv[1]);
	char node_id[2];
	strcpy(node_id, "0");
	node_id[1] = 0;
	pthread_t node_0;
	pthread_create(&node_0, NULL, node_init, (void*) node_id);
  	sleep(3);
  	char *init_port = node_to_port(0);
	char * check = (char *) calloc(1, 5);
	strcpy(check, "init");

	while (0==unisend(check, init_port,0)){
		sleep(2);
	}

	while(1)
	{
		//puts("ready for next command:");
		char *line = NULL;
  		size_t buf_len = 0;
  		if (getline(&line, &buf_len, stdin) != -1)
  		{
  			line[strlen(line)-1] = '\0';
			if (strncmp("join", line, 4) == 0)
			{
				//line[strlen(line)] = '\0';
				//printf("line is %s\n", line);
				char * new_nodeNum = (char*) calloc(1, strlen(line+5)+1);
				strcpy(new_nodeNum, line+5);
				//join_flag =0;
				pthread_t node_pid;
				pthread_create(&node_pid, NULL, node_init, (void*) new_nodeNum);
				
				//char *new_port = node_to_port(atoi(new_nodeNum));
				//printf("new port is %s\n", new_port);
			 //  	// puts("here1");
			 //  	// puts("here2");
				// char command[5];
				// strcpy(command, "init");
				// command[4]=0;
				// while (0==unisend(command, init_port,0)){
				// 	puts("here");
				// 	sleep(2);
				// }
				// puts("done");
			}

			//find p k command: ask node number p to look for key k;
			//find 0 200
			else if (strncmp("find", line, 4) == 0)
			{
				char *command = (char *) calloc(1, strlen(line)+1);
				//char command[strlen(line)+1];
				strcpy(command,line);
				//command[strlen(line)]=0;
				char *temp = strstr(line, " ");
				temp ++;
				char * temp1= strstr(line, " ");
				*temp1=0;
				int num = atoi(temp);
				char *serverport = node_to_port(num);
				//printf("find command is %s, serverport is %s\n", command, serverport);
				unisend(command, serverport,1);
			}
			else if (strncmp("crash", line, 5) == 0)
			{
				int node_num = atoi(line+6);
				char *serverport = node_to_port(node_num);
				char *command = (char *) calloc(1, 6);
				strcpy(command, "crash");
				crash_unisend(command, serverport);
			}

			else if (strncmp("show all", line, 8) == 0)
			{
				for(int i =0; i<256; i++)
				{
					char *serverport = node_to_port(i);
					char *command = (char *) calloc(1, 5);
					strcpy(command, "show");
					unisend(command, serverport, 0);
				}

			}
			//command example: show 8
			else if (strncmp("show", line ,4) == 0)
			{
				int node_num = atoi(line+5);
				char *serverport = node_to_port(node_num);
				char *command = (char *) calloc(1, 5);
				strcpy(command, "show");
				unisend(command, serverport,1);
			}

			else if(strncmp("exit", line, 4) == 0)
				return 0;
			else 
				perror ("invalid command input\n");	
  		}
	}
	return 0;
}

void send_back_client(char *message)
{
	puts(message);
}