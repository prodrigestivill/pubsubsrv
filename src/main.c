/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
 * Copyright (C) Pau Rodriguez 2009 <prodrigestivill@gmail.com>
 * 
 * main.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * main.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void usage (char *_name){
  fprintf(stderr, "\nUsage: %s [options]\n", _name);
  fprintf(stderr, "  -p<port>        : TCP port number\n");
  fprintf(stderr, "  -t<protocol>    : Protocol (text, ...)\n");
  fprintf(stderr, "  -h              : Show this help message\n");
}

int main(int argc, char *argv[])
{
	struct sockaddr_in serv_addr;
	char   *programName;
	int sockfd, opt, port=0, protocol=0;
	programName = argv[0];
	topology();
	while ((opt = getopt(argc, argv, "p:t")) > 0) {
		switch (opt) {
		   case 'p':
				port = atoi(optarg);
			break;
		   case 't':
				if (strcmp (optarg, "text")){
					protocol=1;
					protocol_text();
				}else{
					usage(programName);
					return 0;
				}
			break;
			default:
				usage(programName);
				return 0;
		}
	}
	if (port==0){
		usage(programName);
		return 0;
	}	
	if (protocol==0)
		protocol_text();
	//Start
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){
		fprintf(stderr, "ERROR opening socket");
		return 1;
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		fprintf(stderr, "ERROR on binding");
		return 1;
	}
	server(sockfd);
	return 0;
}