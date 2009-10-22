/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * textline.c
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

#define LINE_MAX_LEN 4096
#define COMMAND_MAX_LEN 1024

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int pubsub_textline_subscribe(int fd, char *name){
	int r;
	char buf[COMMAND_MAX_LEN];
	r = snprintf(buf, COMMAND_MAX_LEN, "SUBSCRIBE %s\r\n", name);
	return write(fd, buf, r);
}

int pubsub_textline_publish(int fd, char *name){
	int r;
	char buf[COMMAND_MAX_LEN];
	r = snprintf(buf, COMMAND_MAX_LEN, "PUBLISH %s\r\n", name);
	return write(fd, buf, r);
}

int pubsub_textline_send(int fd, char *topic, char *text){
	int r;
	char buf[LINE_MAX_LEN];
	r = snprintf(buf, LINE_MAX_LEN, "%s>%s\r\n", topic, text);
	return write(fd, buf, r);
}

int pubsub_textline_recive(int fd, char *topic, int tlen, char *text, int len){
	int r,i,l;
	l = tlen+len+3;
	char buf[l];
	r = read(fd, buf, l);
	if (r>0){
		for (i=0; i<l;i++)
			if (buf[i]=='>')
				break;
		if (i==l){
			i=0;
		}else{
			memcpy(topic, buf, i);
			topic[i]='\0';
			i++;
		}
		//Remove tailing '\n'
		while (r > 1 && (buf[r - 1] == '\n' || buf[r - 1] == '\r'))
			r--;
		l = r-i;
		memcpy(text, buf+i, l);
		return l;
	}else
		return r;
}
