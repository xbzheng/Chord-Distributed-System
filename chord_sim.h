#include <iostream>
#include <map>
#include <list>
#include <math.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/timeb.h>
#define MAXBUFLEN 10000
#define MESG_SIZE 10000
int m = 8;
int min_delay;
int max_delay;
int base_port;
void* node_init(void * message);
void* node_create(void * arg);
int * create_ft(int node_id);
void configParser(const char *filename);
int unisend(char* message, char* serverport, int flag);
void * processClient(void * arg);
int connect_to_node(char * port);
char * node_to_port(int node_num);
void update_all_ft(int node_num);
int figure_node_update(char * port);
int find_key(int key, std::list<int> key_list);
char * find_unisend(char * buffer, char * serverport);
void back_up_request(std::list<int>key, int node_num, int next_node);
void crash_unisend(char* message, char *serverport);
int backup_key_send(char*message, char*serverport);
int show_unisend(char *serverport);
void find_predecessor(int node_num, int * predecessor);
void send_back_client(char *message);
void print_ft(int node_num, int * node_ft);