/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
 * Copyright (C) Pau Rodriguez 2009 <prodrigestivill@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "server.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>

static char *usocket = 0;

static int usage(char *_name)
{
  fprintf(stderr, "Usage: %s [options] -- [protocol options]\n"
    "  -l<port>        : Listen TCP port number\n"
    "  -s<unix-socket> : Listen Unix socket file\n"
    "  -P<protocol>    : Protocols (basic, textline, http, smtp, irc)\n"
    "  -h              : Show this help message\n", _name);
  return 0;
}

static void sig_int_handler(int sig)
{
  if (usocket!=0)
    unlink(usocket);
  exit(143);
}

int main(int argc, char *argv[])
{
  struct sockaddr_in serv_addr_in;
  struct sockaddr_un serv_addr_un;
  char *programName;
  int sockfd, opt, port = -1, protocol = 0;
  programName = argv[0];
  topology();
  while ((opt = getopt(argc, argv, "l:s:P:h")) > 0)
    {
      switch (opt)
        {
          case 'l':
            port = atoi(optarg);
            break;
          case 's':
            usocket = optarg;
            break;
          case 'P':
            if (strcmp(optarg, "basic") == 0)
              protocol = 1;
            else if (strcmp(optarg, "textline") == 0)
              protocol = 2;
            else if (strcmp(optarg, "http") == 0)
              protocol = 10;
            else if (strcmp(optarg, "smtp") == 0)
              protocol = 11;
            else if (strcmp(optarg, "irc") == 0)
              protocol = 12;
            else
              return usage(programName);
            break;
          default:
            return usage(programName);
        }
    }
  if (port <0 && usocket==0)
    {
      usage(programName);
      return 0;
    }
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, sig_int_handler);
  signal(SIGTERM, sig_int_handler);
  switch (protocol)
    {
      case 2:
        protocol_textline(argc, argv);
        break;
      case 10:
        protocol_http(argc, argv);
        break;
      case 11:
        protocol_smtp(argc, argv);
        break;
      case 12:
        protocol_irc(argc, argv);
        break;
      case 1:
      default:
        protocol_basic(argc, argv);
    }
  //Start
  if (port>=0) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
      fprintf(stderr, "ERROR opening TCP socket\n");
      return 1;
    }
    bzero((char *) &serv_addr_in, sizeof(serv_addr_in));
    serv_addr_in.sin_family = AF_INET;
    serv_addr_in.sin_addr.s_addr = INADDR_ANY;
    serv_addr_in.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *) &serv_addr_in, sizeof(serv_addr_in)) < 0)
    {
      fprintf(stderr, "ERROR on binding\n");
      return 1;
    }
  } else {
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
      fprintf(stderr, "ERROR opening unix socket\n");
      return 1;
    }
    unlink(usocket);
    serv_addr_un.sun_family = AF_UNIX;
    strncpy(serv_addr_un.sun_path, usocket,
        (sizeof(struct sockaddr_un) - sizeof(short)));
    if (bind(sockfd, (struct sockaddr *) &serv_addr_un, sizeof(serv_addr_un)) < 0)
    {
      fprintf(stderr, "ERROR on binding unix socket\n");
      return 1;
    }
  }
  server(sockfd);
  return 0;
}
