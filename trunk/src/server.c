/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * server.c
 * Copyright (C) Pau Rodriguez 2009 <prodrigestivill@gmail.com>
 * 
 * server.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * server.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define NEW_CLIENTS 10
#define MAX_CLIENTS 10000
#define POLL_TO 60000

#include "topology.h"
#include "server.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <poll.h>
#ifndef POLLRDHUP
#define POLLRDHUP 0x2000
#endif

int pollfdslen, pollfdsmax;
struct pollfd *pollfds;
struct client **pollclients;

void server(int sockfd)
{
  struct sockaddr_in cliaddr;
  socklen_t cliaddrlen;
  int i, j, r;
  pollfdslen = 1;
  pollfdsmax = NEW_CLIENTS + 1;
  pollfds = malloc(pollfdsmax * sizeof(struct pollfd));
  pollclients = malloc(pollfdsmax * sizeof(struct client *));
  pollfds[0].fd = sockfd;
  pollfds[0].events = POLLIN;
  pollfds[0].revents = 0;
  for (i = 1; i < pollfdsmax; i++)
    {
      pollfds[i].fd = -1;
      pollfds[i].events = 0;
    }
  listen(sockfd, NEW_CLIENTS);
  cliaddrlen = sizeof(cliaddr);
  while (1)
    {
      r = poll(pollfds, pollfdsmax, POLL_TO);
      if (r > 0)
        {
          if (pollfds[0].revents != 0 && pollfdslen < MAX_CLIENTS)
            {
              if (pollfdslen + 1 > pollfdsmax)
                {
                  pollfdsmax += NEW_CLIENTS;
                  pollfds =
                    realloc(pollfds, pollfdsmax * sizeof(struct pollfd));
                  pollclients =
                    realloc(pollclients,
                            pollfdsmax * sizeof(struct client *));
                  for (i = pollfdsmax - NEW_CLIENTS; i < pollfdsmax; i++)
                    {
                      pollfds[i].fd = -1;
                      pollfds[i].events = 0;
                    }
                  i = pollfdslen;
                }
              else
                {
                  for (i = 1; i < pollfdslen; i++)
                    if (pollfds[i].fd < 0)
                      break;
                }
              pollfds[i].fd =
                accept(sockfd, (struct sockaddr *) &cliaddr, &cliaddrlen);
              pollfds[i].events = POLLIN | POLLRDHUP;
              pollfds[i].revents = 0;
              pollclients[i] = server_newclient(pollfds[i].fd);
              if (pollclients[i] == 0)
                {
                  server_close(pollfds[i].fd);
                  pollfds[i].fd = -1;
                }
              else
                {
                  memcpy(&pollclients[i]->connection_addr, &cliaddr,
                         cliaddrlen);
                  pollfdslen++;
                }
              r--;
            }
          pollfds[0].revents = 0;
          for (i = 1; i < pollfdsmax && r > 0; i++)
            {
              if (pollfds[i].fd != -1 && pollfds[i].revents != 0)
                {
                  if (pollfds[i].revents & POLLHUP ||
                      pollfds[i].revents & POLLRDHUP ||
                      pollfds[i].revents & POLLERR ||
                      pollfds[i].revents & POLLNVAL)
                    {
                      server_endclient(pollclients[i]);
                      remove_client_fd(pollfds[i].fd);
                      server_close(pollfds[i].fd);
                      pollfds[i].fd = -1;
                      pollfdslen--;
                    }
                  else
                    server_read(pollclients[i]);
                  pollfds[i].revents = 0;
                  r--;
                }
            }
        }
      //Shrink pollfds & pollclients arrays
      if (pollfdsmax - pollfdslen >= NEW_CLIENTS * 2)
        {
          for (i = pollfdsmax - NEW_CLIENTS; i < pollfdsmax; i++)
            if (pollfds[i].fd >= 0)
              for (j = 1; j < pollfdsmax - NEW_CLIENTS; j++)
                if (pollfds[j].fd < 0)
                  {
                    pollfds[j].fd = pollfds[i].fd;
                    pollfds[j].events = pollfds[i].events;
                    pollfds[j].revents = 0;
                    pollclients[j] = pollclients[i];
                  }
          pollfdsmax -= NEW_CLIENTS;
          pollfds = realloc(pollfds, pollfdsmax * sizeof(struct pollfd));
          pollclients =
            realloc(pollclients, pollfdsmax * sizeof(struct client *));
        }
    }
}

void server_send(struct client *from, struct topic *to, char buf[],
                 int len)
{
  struct subscriber *s;
  topic_list_for_each(s, &to->subscribers)
  {
    if (s->state > 0)
      server_write(to, from, s->client, buf, len);
  }
}

void server_close(int fd)
{
  close(fd);
}
