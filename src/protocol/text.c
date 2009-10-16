/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * text.c
 * Copyright (C) Pau Rodriguez 2009 <prodrigestivill@gmail.com>
 * 
 * text.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * text.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#define LINE_MAX_LEN 2048
#include "../topology.h"
#include "../server.h"
#include <unistd.h>
#include <string.h>

void protocol_text_input(struct client *from, char buf[], int len){
	struct subscriber *s;
    struct publisher *p;
	int r;
    switch (from->state){
        case 0: // Setup mode
			//Remove tailing '\n'
			while(len>1 && (buf[len-1]=='\n' || buf[len-1]=='\r'))
				len--;
			//RECV FROM:
            if (len > 10 && strncmp("RECV FROM:", buf, 10)==0){
				s = subscriber(from, get_topic(buf+10, len-10));
				r = write(from->connection, "250 Subscribed\r\n", 16);
				s->state = 1;
				break;
			}
            //RCPT TO:
		    if (len > 8 && strncmp("RCPT TO:", buf, 8)==0){
				p = publisher(from, get_topic(buf+8, len-8));
				r = write(from->connection, "250 Publisher set\r\n", 19);
				p->state = 1;
				break;
			}
            //DATA
		    if (len > 3 && strncmp("DATA", buf, 4)==0){
				r = write(from->connection, "354 End data with <CR><LF>.<CR><LF>\r\n", 37);
				from->state = 1;
				break;
			}
		    //QUIT
		    if (len > 3 && strncmp("QUIT", buf, 4)==0){
				remove_client(from);
				r = write(from->connection, "221 Bye\r\n", 9);
				server_close(from->connection);
				break;
			}
		    //HELO
		    if (len > 3 && strncmp("HELO", buf, 4)==0){
				r = write(from->connection, "250 Helo\r\n", 10);
				break;
			}
			r = write(from->connection, "502 Command not implemented\r\n", 29);
        	break;
		case 2: //Streaming (last ended \n)
			if (len>0 && buf[0]=='.' && (len==2 || (len==3 && buf[1]=='\r'))){
				from->state = 0;
				r = write(from->connection, "250 Ok\r\n", 8);
				break;
			}
			//Don't break here!!!
        case 1: // Streaming mode
            client_list_for_each(p, &from->publishers){
				if (p->state>0){
					server_send(from, p->topic, buf, len);
				}
            }
	        break;
    }
}
 
int procotol_text_read(struct client *c){
    char buf[LINE_MAX_LEN];
	int i, l, n, len;
	len = read(c->connection, buf, LINE_MAX_LEN);
	n = 0;
	l = 0;
	for (i=0; i<len; i++){
		if (buf[i] == '\n'){
			protocol_text_input(c, &buf[l], i-l+1);
			if (c->state==1)
				c->state = 2;
			l=i+1;
			n++;
		}
	}
	if (l<len){
		protocol_text_input(c, &buf[l], len-l);
		n++;
		if (c->state==2)
			c->state = 1;
	}
    return n;
   	//TODO permit char-by-char commands?.
    /*
    n = 0;
    len = 0;
    do{
        c = &buf[len++];
        r = read(c->connection, c, 1);
        if (r==1 && *c == '\n'){
            protocol_text_input(c, buf, len);
            n++;
            len = 0;
        }
    }while(r>0 && len<LINE_MAX_LEN);
    if (r<0)
        return -1;
	*/
}

int procotol_text_write
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

struct client *procotol_text_newclient(int fd){
	int r;
	r = write(fd, "220 PubSub Server\r\n", 19);
	return get_client(fd);
}

void protocol_text(){
    server_read = procotol_text_read;
	server_write = procotol_text_write;
	server_newclient = procotol_text_newclient;
}
