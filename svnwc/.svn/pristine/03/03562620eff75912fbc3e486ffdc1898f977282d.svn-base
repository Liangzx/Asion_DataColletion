#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>


int main(int argc, char ** argv)
{	
	if (argc != 5) {
		fprintf(stderr, "parameters not right!!!\n");
		fprintf(stderr, "useage: program IP port  interval(seconds) vender[LW/PAD/JIT808]\n");
		return -1;
	}
	int interal = atoi(argv[3]);
	std::string vender = argv[4];
	// get socket fd
	int clifd =  socket(PF_INET, SOCK_STREAM, 0);
	int opt = 1;
	// reuse socket fd
	setsockopt(clifd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in cliaddr;
	cliaddr.sin_family = PF_INET;
	char *IP = argv[1];// ip
	short PORT = atoi(argv[2]); // port pad-5000 lw-8996 808-3333

	cliaddr.sin_port = htons(PORT);
	cliaddr.sin_addr.s_addr = inet_addr(IP);
	// connect server
	if (connect(clifd, (struct sockaddr *)&cliaddr, sizeof(cliaddr)) == -1) {
		perror("can't connect server.");
		exit(-1);
	}
	fprintf(stdout, "ip[%s] port[%d],interal[%d]\n", IP, PORT, interal);
	while (true)
	{
		
		//char buf_pad[] = "AT66867106022056110,33251,125469984,0,102.6133426732902,26.343084828707404,0.0,0.0,,,-1,460018230389346,15608179872,12.5,0X0D 0X0AAT66867106022056110,33251,125469984,0,102.6133426732902,26.343084828707404,0.0,0.0,,,-1,460018230389346,15608179872,12.5,0X0D 0X0AAT66867106022056110,33251,125469984,0,102.6133426732902,26.343084828707404,0.0,0.0,,,-1,460018230389346,15608179872,12.5,0X0D 0X0A";
		char buf_pad[] = "AT66867106022056110, 33251, 125469984, 0, 102.6133426732902, 26.343084828707404, 0.0, 0.0, , , -1, 460018230389346, 15608179872, 12.5, 0X0D 0X0A";
		char buf_808[] = "\x7e\x02\x00\x00\x33\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\xf1\xf1\xf1\xf1\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\xf1\xf1\x01\x01\x01\x01\x01\x01\x7e";
		char buf_lw[] = "\x4f\x4e\x53\x33\x00\x01\x73\x23\x14\x0e\x0c\x16\x07\x28\x21\x16\x21\xa8\x38\x14\x4e\x0b\x23\x8a\x1a\x33\x00\x45\x00\x46\x3c\x00\x00\x2f\x01\x01\x01\x5F\x89\x34\x00\xA5\x2E\x48\x47\x0d\x01\x02\x04\x01\x01\x09\x01\x02\x03\x04\x05\x06\x07\x08\x09\x04\x06\x00\x00\x02\x09\x09\x06\x01\x01\x02\x08\x00\x08\x05\x01\x03\x05\x01\x02\x03\x04\x05\x06\x07\x08\x00\x00";
		int len = 0;
		char *buf = NULL;
		if (vender == "LW") {
			buf = buf_lw;
			len = sizeof(buf_lw);
		}	else if(vender == "PAD") {
			buf = buf_pad;
			len = sizeof(buf_pad);
		}	else if(vender == "JIT808") {
			buf = buf_808;
			len = sizeof(buf_808);
		}
		int sd = send(clifd, buf, len, MSG_DONTWAIT);
		if (sd == EAGAIN) {
			printf("EAGAIN\n");	
		}
		else if (sd == -1) {
			printf("send error\n");
		}	else {
			;
		}
		sleep(interal);
	}

	return 0;
}
