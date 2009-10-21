/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * server.h
 * Copyright (C) Pau Rodriguez 2009 <prodrigestivill@gmail.com>
 * 
 * server.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * server.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SERVER_H
#define _SERVER_H
#include "topology.h"

void server(int sockfd);
void server_send(struct client *from, struct topic *to, char *buf,
                 int len);
struct client *(*server_newclient) (int);       //fd
void (*server_endclient) (struct client *);     //from
int (*server_read) (struct client *);   //from
int (*server_write) (struct client *, struct client *, char *, int);    //from, to, buf, len
void server_close(int fd);

#endif
