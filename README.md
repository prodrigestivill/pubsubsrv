# pubsubsrv
Simple Server for Publish/Subscribe of streams designed to support easily various protocols.

Any suggestion will be welcomed (new protocols, changing current protocols, ...).

### What's Publish/Subscribe? ###
  * From [Wikipedia](http://en.wikipedia.org/wiki/Publish/subscribe).


### Currently implements the following protocols: ###
  * _basic_: simple basic protocol (binary compatible).
  * _textline_: simple text protocol multi-topic capable.
  * _http_: HTTP streaming server (using PUT/GET).
  * _smtp_: close to SMTP, but oriented to streams.
  * _irc_: simple IRC server (partial done).


### Implementation details: ###
  * No queues are implemented.
  * Small footprint.
  * Entirely in C, using poll (without threads).
  * Low latency.

See [GitHub Wiki](https://github.com/prodrigestivill/pubsubsrv/wiki) for more information.
