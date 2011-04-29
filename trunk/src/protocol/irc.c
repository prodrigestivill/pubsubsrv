/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * irc.c
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

#define NICK_MAX_LEN 64
#define RECV_MAX_LEN 2048
#define SEND_MAX_LEN 4096
#include "../topology.h"
#include "../server.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

struct protocol_irc_client_data
{
  char nick[NICK_MAX_LEN];
  char user[NICK_MAX_LEN];
  char host[256];
};

struct client *
protocol_irc_get_client (char *name, int len)
{
  struct client *c;
  item_list_for_each (c, &clients)
  {
    if (strncmp
	(((struct protocol_irc_client_data *) c->data)->nick, name, len) == 0)
      return c;
  }
  return 0;
}

struct topic *
protocol_irc_get_topic (struct client *from, char *name, int len)
{
  struct publisher *p;
  client_list_for_each (p, &from->publishers)
  {
    if (strncmp (p->topic->name, name, len) == 0)
      return p->topic;
  }
  return 0;
}

int
protocol_irc_is_topic(char firstchar)
{
	switch (firstchar){
		case '#':
		case '&':
		case '!':
		case '+':
		case '.':
		case '~':
			return 1;
		default:
			return 0;
	}
}

void
protocol_irc_input (struct client *from, char buf[], int len)
{
  char buf2[SEND_MAX_LEN];
  struct client *c;
  struct topic *t;
  struct subscriber *s;
  struct publisher *p;
  int r;
  //Remove tailing '\n'
  while (len > 1 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
    len--;
  if (len < RECV_MAX_LEN)
    buf[len] = '\0';
  else
    buf[RECV_MAX_LEN - 1] = '\0';
  if (len > 6 && strncmp ("JOIN ", buf, 5) == 0 && protocol_irc_is_topic(buf[5]))
    {
      t = get_topic (buf + 5, len - 5);
      s = subscriber (from, t);
      //TODO: Send a message to all presents
      s->state = 1;
      //Moderate?
      p = publisher (from, t);
      p->state = 1;
      return;
    }
  if (len > 8 && strncmp ("PRIVMSG ", buf, 8) == 0)
    {
      snprintf (buf2, SEND_MAX_LEN, ":%s!n=%s@%s %s\r\n",
		((struct protocol_irc_client_data *) from->data)->nick,
		((struct protocol_irc_client_data *) from->data)->user,
		((struct protocol_irc_client_data *) from->data)->host, buf);
      for (r = 9; r < len; r++)
	if (buf[r] == ' ' || buf[r] == ':')
	  break;
      if (protocol_irc_is_topic(buf[8]))
	{
	  t = protocol_irc_get_topic (from, &buf[8], r - 8);
	  if (t != 0)
	    server_send (from, t, buf2, strlen (buf2));
	}
      else
	{
	  c = protocol_irc_get_client (&buf[8], r - 8);
	  if (c != 0)
	    server_write (0, from, c, buf2, strlen (buf2));
	}
      return;
    }
  if (len > 4 && strncmp ("PING ", buf, 5) == 0)
    {
      snprintf (buf2, SEND_MAX_LEN, ":server PONG server : %s\r\n", &buf[5]);
      r = write (from->connection, buf2, strlen (buf2));
      if (r < 0)
	{
	  server_close_client (from);
	  return;
	}
      return;
    }
  if (len > 5 && strncmp ("NICK ", buf, 5) == 0 && !protocol_irc_is_topic(buf[5]))
    {
      if (len - 4 > NICK_MAX_LEN)
	len = NICK_MAX_LEN + 4;
      memcpy (((struct protocol_irc_client_data *) from->data)->nick,
	      &buf[5], len - 5);
      ((struct protocol_irc_client_data *) from->data)->nick[len - 5] = '\0';
      from->state = 1;
      return;
    }
  if (len > 5 && strncmp ("USER ", buf, 5) == 0)
    {
      for (r = 6; r < len; r++)
	if (buf[r] == ' ')
	  break;
      if (r - 4 > NICK_MAX_LEN)
	r = NICK_MAX_LEN + 4;
      memcpy (((struct protocol_irc_client_data *) from->data)->user,
	      &buf[5], r - 5);
      ((struct protocol_irc_client_data *) from->data)->user[r - 5] = '\0';
      from->state = 1;
      return;
    }
    if (len > 3 && strncmp("QUIT", buf, 4)==0){
    (void) write(from->connection, "221 Bye\r\n", 9);
    server_close_client(from);
    return;
    }
    r = write(from->connection, ": Command not implemented\r\n", 27);
    if (r<0){
    server_close_client(from);
    return;
    }
}

int
protocol_irc_read (struct client *c)
{
  char buf[RECV_MAX_LEN];
  int i, l, n, len;
  len = read (c->connection, buf, RECV_MAX_LEN);
  if (len < 0)
    return -1;
  n = 0;
  l = 0;
  for (i = 0; i < len; i++)
    {
      if (buf[i] == '\n')
	{
	  protocol_irc_input (c, &buf[l], i - l + 1);
	  l = i + 1;
	  n++;
	}
    }
  if (l < len)
    {
      protocol_irc_input (c, &buf[l], len - l);
      n++;
    }
  return n;
}

int protocol_irc_write
  (struct topic *topicfrom, struct client *from, struct client *to,
   char buf[], int len)
{
  int r;
  if (to->state > 0)
    {
      r = write (to->connection, buf, len);
      if (r == len)
	return 1;
      if (r < 0)
	return r;
    }
  return 0;
}

struct client *
protocol_irc_newclient (int fd)
{
  struct client *c = get_client (fd);
  c->data = malloc (sizeof (struct protocol_irc_client_data));
  ((struct protocol_irc_client_data *) c->data)->nick[0] = '\0';
  ((struct protocol_irc_client_data *) c->data)->user[0] = '\0';
  ((struct protocol_irc_client_data *) c->data)->host[0] = '\0';
  return c;
}

void
protocol_irc_endclient (struct client *c)
{
//TODO send PART messages to all subscriptions
}

void
protocol_irc_free_client_data (struct client *c)
{
  free (c->data);
}

void
protocol_irc_free_topic_data (struct topic *t)
{
}

void
protocol_irc (int argc, char *argv[])
{
  server_read = protocol_irc_read;
  server_write = protocol_irc_write;
  server_newclient = protocol_irc_newclient;
  server_endclient = protocol_irc_endclient;
  free_client_data = protocol_irc_free_client_data;
  free_topic_data = protocol_irc_free_topic_data;
  //TODO Register an alert to send PING to clients
}
