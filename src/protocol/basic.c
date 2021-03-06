/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * basic.c
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

#include "../topology.h"
#include "../server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int protocol_basic_read_size;

void
protocol_basic_input (struct client *from, char buf[], int len)
{
  struct subscriber *s;
  struct publisher *p;
  int r;
  switch (from->state)
    {
    case 0:			// Setup mode
      //Remove tailing '\n'
      while (len > 1 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
	len--;
      if (len > 10 && strncmp ("SUBSCRIBE ", buf, 10) == 0)
	{
	  s = subscriber (from, get_topic (buf + 10, len - 10));
	  s->state = 1;
	  break;
	}
      if (len > 8 && strncmp ("PUBLISH ", buf, 8) == 0)
	{
	  p = publisher (from, get_topic (buf + 8, len - 8));
	  p->state = 1;
	  break;
	}
      if (len > 4 && strncmp ("START", buf, 5) == 0)
	{
	  from->state = 1;
	  break;
	}
      if (len > 3 && strncmp ("QUIT", buf, 4) == 0)
	{
	  server_close_client (from);
	  break;
	}
      r =
	write (from->connection,
	       "COMMANDS: SUBSCRIBE,PUBLISH,START,QUIT\r\n", 40);
      if (r < 0)
	{
	  server_close_client (from);
	  return;
	}
      break;
    case 1:			// Streaming mode
      client_list_for_each (p, &from->publishers)
      {
	if (p->state > 0)
	  {
	    server_send (from, p->topic, buf, len);
	  }
      }
      break;
    }
}

int
protocol_basic_read (struct client *c)
{
  char buf[protocol_basic_read_size];
  int i, l, n, len;
  len = read (c->connection, buf, protocol_basic_read_size);
  if (len < 0)
    return -1;
  if (c->state > 0)
    {
      protocol_basic_input (c, buf, len);
      n = 1;
    }
  else
    {
      n = 0;
      l = 0;
      for (i = 0; i < len; i++)
	{
	  if (buf[i] == '\n')
	    {
	      protocol_basic_input (c, &buf[l], i - l + 1);
	      l = i + 1;
	      n++;
	    }
	}
      if (l < len)
	{
	  protocol_basic_input (c, &buf[l], len - l);
	  n++;
	}
    }
  return n;
}

int protocol_basic_write
  (struct topic *topicfrom, struct client *from, struct client *to,
   char buf[], int len)
{
  int r;
  if (from != to && to->state > 0)
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
protocol_basic_newclient (int fd)
{
  return get_client (fd);
}

void
protocol_basic_endclient (struct client *c)
{
}

void
protocol_basic_free_client_data (struct client *c)
{
}

void
protocol_basic_free_topic_data (struct topic *t)
{
}

void
protocol_basic (int argc, char *argv[])
{
  int opt;
  server_read = protocol_basic_read;
  server_write = protocol_basic_write;
  server_newclient = protocol_basic_newclient;
  server_endclient = protocol_basic_endclient;
  free_client_data = protocol_basic_free_client_data;
  free_topic_data = protocol_basic_free_topic_data;

  protocol_basic_read_size = 4096;
  while ((opt = getopt (argc, argv, "s:h")) > 0)
    {
      switch (opt)
	{
	case 's':
	  protocol_basic_read_size = atoi (optarg);
	  break;
	default:
	  fprintf (stderr,
		   "Basic protcol options: usage in [protocol options]\n"
		   "  -s<size>        : Read size\n"
		   "  -h              : Show this help message\n");
	  exit (0);
	}
    }
}
