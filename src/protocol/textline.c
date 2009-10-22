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
#include "../topology.h"
#include "../server.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

struct topic *protocol_textline_get_topic(struct client *from, char *name,
                                          int len)
{
  struct publisher *p;
  client_list_for_each(p, &from->publishers)
  {
    if (strncmp(p->topic->name, name, len) == 0)
      return p->topic;
  }
  return 0;
}

void protocol_textline_input(struct client *from, char buf[], int len)
{
  struct subscriber *s;
  struct publisher *p;
  struct topic *t;
  char buf2[LINE_MAX_LEN];
  int l, r, r2;
  if (len > 10 && strncmp("SUBSCRIBE ", buf, 10) == 0)
    {
      //Remove tailing '\n'
      while (len > 1 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
        len--;
      s = subscriber(from, get_topic(buf + 10, len - 10));
      s->state = 1;
      return;
    }
  if (len > 8 && strncmp("PUBLISH ", buf, 8) == 0)
    {
      //Remove tailing '\n'
      while (len > 1 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
        len--;
      p = publisher(from, get_topic(buf + 8, len - 8));
      p->state = 1;
      return;
    }
  if (len > 3 && strncmp("QUIT", buf, 4) == 0)
    {
      remove_client(from);
      server_close(from->connection);
      return;
    }
  for (l = 0; l < len; l++)
    if (buf[l] == '>')
      break;
  if (l >= len)
    l = -1;
  if (l < 1 || (l == 1 && buf[0] == '*'))
    {
      //BROADCAST
      client_list_for_each(p, &from->publishers)
      {
        if (p->state > 0)
          {
            r = strlen(p->topic->name);
            memcpy(buf2, p->topic->name, r);
            if (l < 0)
              buf2[r++] = '>';
            r2 = (r + len - l > LINE_MAX_LEN ? LINE_MAX_LEN - r : len - l);
            memcpy(buf2 + r, buf + l, r2);
            server_send(from, p->topic, buf2, r + r2);
          }
      }
    }
  else
    {
      t = protocol_textline_get_topic(from, buf, l);
      if (t != 0)
        server_send(from, t, buf, len);
      else
        r =
          write(from->connection, "ERR: Topic not set for publishing.\r\n",
                36);
    }
}

int protocol_textline_read(struct client *c)
{
  char buf[LINE_MAX_LEN];
  int i, l, n, len;
  len = read(c->connection, buf, LINE_MAX_LEN);
  n = 0;
  l = 0;
  for (i = 0; i < len; i++)
    {
      if (buf[i] == '\n')
        {
          protocol_textline_input(c, &buf[l], i - l + 1);
          l = i + 1;
          n++;
        }
    }
  if (l < len)
    {
      protocol_textline_input(c, &buf[l], len - l);
      n++;
    }
  return n;
}

int protocol_textline_write
  (struct topic *topicfrom, struct client *from, struct client *to, char buf[], int len)
{
  int r;
  if (from != to)
    {
      r = write(to->connection, buf, len);
      if (r == len)
        return 1;
      if (r < 0)
        return r;
    }
  return 0;
}

struct client *protocol_textline_newclient(int fd)
{
  return get_client(fd);
}

void protocol_textline_endclient(struct client *c)
{
}

void protocol_textline_free_client_data(struct client *c)
{
}

void protocol_textline_free_topic_data(struct topic *t)
{
}

void protocol_textline(int argc, char *argv[])
{
  server_read = protocol_textline_read;
  server_write = protocol_textline_write;
  server_newclient = protocol_textline_newclient;
  server_endclient = protocol_textline_endclient;
  free_client_data = protocol_textline_free_client_data;
  free_topic_data = protocol_textline_free_topic_data;
}
