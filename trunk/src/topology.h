/***************************************************************************
 *            topology.h
 *
 *  Thu Oct 15 13:48:54 2009
 *  Copyright  2009  plue
 *  <plue@<host>>
 ****************************************************************************/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */
 
#ifndef _TOPOLOGY_H
#define _TOPOLOGY_H
#include "list.h"

struct topic {
        struct list_head item;
        struct list_head publishers, subscribers;
        char *name;
};

struct publisher{
        struct list_head clientitem;
		struct list_head topicitem;
        struct client *client;
        struct topic *topic;
        int state;
};

struct subscriber{
        struct list_head clientitem;
		struct list_head topicitem;
        struct client *client;
        struct topic *topic;
        int state;
};

struct client{
        struct list_head item;
        int connection;
        int state;
        struct list_head publishers, subscribers;
};

void topology();
struct client *add_client(int s);
struct topic *add_topic(char *name, int len);
struct client *get_client(int s);
struct topic *get_topic(char *name, int len);
struct subscriber *subscriber(struct client *c, struct topic *t);
struct publisher *publisher(struct client *c, struct topic *t);
void unsubscriber(struct subscriber *s);
void unpublisher(struct publisher *p);
void remove_client(struct client *c);
void remove_client_fd(int s);

#define item_list_for_each(pos, head) \
	list_for_each_entry(pos, head, item)
#define topic_list_for_each(pos, head) \
	list_for_each_entry(pos, head, topicitem)
#define client_list_for_each(pos, head) \
	list_for_each_entry(pos, head, clientitem)

#endif
