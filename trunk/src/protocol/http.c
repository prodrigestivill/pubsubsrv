/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * http.c
 * Copyright (C) Pau Rodriguez 2009 <prodrigestivill@gmail.com>
 * 
 * http.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * http.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#define LINE_MAX_LEN 2048
#define MIME_MAX_SIZE 512
#define BUF_MAX_SIZE 1024
#include "../topology.h"
#include "../server.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

struct protocol_http_topic_data{
	char mimetype[MIME_MAX_SIZE];
};

void protocol_http_input(struct client *from, char buf[], int len){
	char buf2[BUF_MAX_SIZE];
	struct topic *t;
	struct subscriber *s;
    struct publisher *p;
	int r;
	if (from->state<200)
		while(len>1 && (buf[len-1]=='\n' || buf[len-1]=='\r'))
			len--;
    switch (from->state){
        case 0: // Initial
			//GET
            if (len > 4 && strncmp("GET ", buf, 4)==0){
				for (r=6;r<len;r++)
					if (buf[r]==' ')
						break;
				t = get_existent_topic(buf+5, r-5);
				if (t==0){
					strcpy(buf2,
						"HTTP/1.1 404 Not Found\r\nServer: PubSub Server\r\nConnection: keep-alive\r\nContent-Length: 0\r\n\r\n");
					from->state = 1;
				}else{
					s = subscriber(from, t);
					s->state = 1;
					snprintf(buf2, BUF_MAX_SIZE, 
						"HTTP/1.0 200 OK\r\nServer: PubSub Server\r\nConnection: close\r\nContent-Type:%s\r\n\r\n",
						((struct protocol_http_topic_data*)t->data)->mimetype);
					from->state = 2;
				}
				r = write(from->connection, buf2, strlen(buf2));
				break;
			}
			//PUT
            if (len > 4 && strncmp("PUT ", buf, 4)==0){
				for (r=6;r<len;r++)
					if (buf[r]==' ')
						break;
				t = get_existent_topic(buf+5, r-5);
				if (t==0){
					t = add_topic(buf+5, r-5);
					t->data = malloc(sizeof(struct protocol_http_topic_data));
					strcpy(((struct protocol_http_topic_data *)t->data)->mimetype, " application/octet-stream");
				}
				p = publisher(from, t);
				p->state = 1;
				from->state = 3;
				break;
			}
			//ERROR
			from->state = 5;
			break;
		case 1: //GET 404 Not Found: Headers...
			if (len==0 || (len==1 && (buf[0]=='\r' || buf[0]=='\n')))
				from->state=0;
			break;
		case 2: //GET 200 OK: Final
			break;
		case 3: //PUT: Pending of Mime-Type
            if (len > 13 && strncmp("Content-Type:", buf, 13)==0){
				if (len-12>MIME_MAX_SIZE)
					len=MIME_MAX_SIZE+12;
				memcpy(((struct protocol_http_topic_data*)t->data)->mimetype, buf+13, len-13);
				((struct protocol_http_topic_data*)t->data)->mimetype[len-13]='\0';
			}else if (len==0 || (len==1 && (buf[0]=='\r' || buf[0]=='\n'))){
				from->state=200;
				strcpy(buf2,"HTTP/1.0 100 Continue\r\n");
				r = write(from->connection, buf2, strlen(buf2));
			}
			break;
		case 5: //ERROR
			if (len==0 || (len==1 && (buf[0]=='\r' || buf[0]=='\n'))){
				from->state=0;
				strcpy(buf2,
					"HTTP/1.1 501 Not Implemented\r\nServer: PubSub Server\r\nConnection: keep-alive\r\nContent-Length: 0\r\n\r\n");
				r = write(from->connection, buf2, strlen(buf2));
			}
			break;
		case 200: //PUT: Streaming
            client_list_for_each(p, &from->publishers){
				if (p->state>0){
					server_send(from, p->topic, buf, len);
				}
            }
	        break;
    }
}
 
int protocol_http_read(struct client *c){
    char buf[LINE_MAX_LEN];
	int i, l, len;
	len = read(c->connection, buf, LINE_MAX_LEN);
	if (c->state>=200)
		protocol_http_input(c, buf, len);
	else{
		l = 0;
		for (i=0; i<len; i++){
			if (buf[i] == '\n'){
				protocol_http_input(c, &buf[l], i-l+1);
				l=i+1;
			}
		}
		if (l<len)
			protocol_http_input(c, &buf[l], len-l);
	}
	return 1;
}

int protocol_http_write
	(struct client *from, struct client *to, char buf[], int len){
	int r;
	if (from!=to && to->state>0){
		r = write(to->connection, buf, len);
		if (r==len)
			return 1;
		if (r<0)
			return r;
	}
		return 0;
}

struct client *protocol_http_newclient(int fd){
	return get_client(fd);
}

void protocol_http_endclient(struct client* c){
	struct subscriber *s;
	struct publisher *p;
    client_list_for_each(p, &c->publishers){
		//If last one, close connections...
		if (&p->topic->publishers!=p->topic->publishers.next &&
			p->topic->publishers.prev==p->topic->publishers.next){
			topic_list_for_each(s, &p->topic->subscribers){
				server_close(s->client->connection);
			}
			remove_topic(p->topic);
		}
    }
}

void protocol_http_free_client_data(struct client* c){
}

void protocol_http_free_topic_data(struct topic* t){
	free(t->data);
}

void protocol_http(){
    server_read = protocol_http_read;
	server_write = protocol_http_write;
	server_newclient = protocol_http_newclient;
	server_endclient = protocol_http_endclient;
	free_client_data = protocol_http_free_client_data;
	free_topic_data = protocol_http_free_topic_data;
}
