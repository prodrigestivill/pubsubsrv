/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * topology.h
 * Copyright (C) Pau Rodriguez 2009 <prodrigestivill@gmail.com>
 * 
 * topology.h is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * topology.h is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TOPOLOGY_H
#define _TOPOLOGY_H
#include "list.h"
#include <netinet/in.h>

struct list_head clients;
struct list_head topics;

struct topic
{
  struct list_head item;
  struct list_head publishers, subscribers;
  char *name;
  void *data;
};

struct publisher
{
  struct list_head clientitem;
  struct list_head topicitem;
  struct client *client;
  struct topic *topic;
  int state;
};

struct subscriber
{
  struct list_head clientitem;
  struct list_head topicitem;
  struct client *client;
  struct topic *topic;
  int state;
};

struct client
{
  struct list_head item;
  int connection;
  struct sockaddr_in connection_addr;
  struct list_head publishers, subscribers;
  int state;
  void *data;
};

void topology();
struct client *add_client(int s);
struct topic *add_topic(char *name, int len);
struct client *get_client(int s);
struct client *get_existent_client(int s);
struct topic *get_topic(char *name, int len);
struct topic *get_existent_topic(char *name, int len);
struct subscriber *subscriber(struct client *c, struct topic *t);
struct publisher *publisher(struct client *c, struct topic *t);
void unsubscriber(struct subscriber *s);
void unpublisher(struct publisher *p);
void remove_client(struct client *c);
void remove_client_fd(int s);

void (*free_client_data) (struct client *);     //client
void (*free_topic_data) (struct topic *);       //toppic

#define item_list_for_each(pos, head) \
	list_for_each_entry(pos, head, item)
#define topic_list_for_each(pos, head) \
	list_for_each_entry(pos, head, topicitem)
#define client_list_for_each(pos, head) \
	list_for_each_entry(pos, head, clientitem)

#endif
