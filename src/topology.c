/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * topology.c
 * Copyright (C) Pau Rodriguez 2009 <prodrigestivill@gmail.com>
 * 
 * topology.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * topology.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include "topology.h"

#include <stdlib.h>
#include <string.h>

void topology(){
	INIT_LIST_HEAD(&clients);
	INIT_LIST_HEAD(&topics);
}

struct client *add_client(int s){
        struct client *c = (struct client *) malloc(sizeof(struct client));
        c->connection = s;
		c->state = 0;
		INIT_LIST_HEAD(&c->publishers);
		INIT_LIST_HEAD(&c->subscribers);
        list_add_tail(&c->item, &clients);
        return c;
}

struct client *get_client(int s){
        struct client *c;
        item_list_for_each(c, &clients){
                if (c->connection==s)
                        return c;
        }
        return add_client(s);
}

struct client *get_existent_client(int s){
        struct client *c;
        item_list_for_each(c, &clients){
                if (c->connection==s)
                        return c;
        }
        return 0;
}

struct topic *add_topic(char *name, int len){
        struct topic *t = (struct topic *) malloc(sizeof(struct topic));
        t->name = malloc(len+1);
		memcpy(t->name, name, len);
		t->name[len]='\0';
		INIT_LIST_HEAD(&t->publishers);
		INIT_LIST_HEAD(&t->subscribers);
        list_add_tail(&t->item, &topics);
        return t;
}

struct topic *get_topic(char *name, int len){
        struct topic *t;
        item_list_for_each(t, &topics){
                if (strncmp(t->name, name, len)==0)
                        return t;
        }
        return add_topic(name, len);
}

struct subscriber *subscriber(struct client *c, struct topic *t){
        struct subscriber *s = (struct subscriber *) malloc(sizeof(struct subscriber));
        s->state = 0;
        s->client = c;
        s->topic = t;
        list_add_tail(&s->clientitem, &c->subscribers);
        list_add_tail(&s->topicitem, &t->subscribers);
        return s;
}

struct publisher *publisher(struct client *c, struct topic *t){
        struct publisher *p = (struct publisher *) malloc(sizeof(struct publisher));
        p->state = 0;
        p->client = c;
        p->topic = t;
        list_add_tail(&p->clientitem, &c->publishers);
        list_add_tail(&p->topicitem, &t->publishers);
        return p;
}

void unsubscriber(struct subscriber *s){
	s->state = 0;
	list_del(&s->topicitem);
	list_del(&s->clientitem);
	free(s);
}

void unpublisher(struct publisher *p){
	p->state = 0;
	list_del(&p->topicitem);
	list_del(&p->clientitem);
	free(p);
}

void remove_client(struct client *c){
	struct publisher *p;
	struct subscriber *s;
	c->state=0;
    client_list_for_each(p, &c->publishers){
		unpublisher(p);
	}
	client_list_for_each(s, &c->subscribers){
		unsubscriber(s);
	}
	free_client_data(c);
	list_del(&c->item);
}

void remove_client_fd(int s){
	struct client *c = get_existent_client(s);
	if (c!=0)
		remove_client(c);
}
