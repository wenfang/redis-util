#include "r_util.h"
#include "redis.h"

redisClient* createFackClient(int start, int len, int cap, robj** argv) {
  int i;
  redisClient *fackClient;

  fackClient = createClient(-1);
  fackClient->flags |= REDIS_LUA_CLIENT;
  fackClient->argc = 0;
  fackClient->argv = zmalloc(sizeof(robj*)*cap);
  fackClient->argv[fackClient->argc++] = createStringObject("fack", 4);
  for (i=0; i<len; i++) {
    fackClient->argv[fackClient->argc++] = dupStringObject(argv[start+i]);
  }

  return fackClient;
}

static sds getReply(redisClient* c) {
  sds reply;
  if (listLength(c->reply) == 0 && c->bufpos < REDIS_REPLY_CHUNK_BYTES) {
    c->buf[c->bufpos] = '\0';
    reply = c->buf;
    c->bufpos = 0;
  } else {
    reply = sdsnewlen(c->buf, c->bufpos);
    c->bufpos = 0;
    while (listLength(c->reply)) {
      robj* o = listNodeValue(listFirst(c->reply));
      reply = sdscatlen(reply, o->ptr, sdslen(o->ptr));
      listDelNode(c->reply, listFirst(c->reply));
    }
  }
  return reply;
}

long long getLongLongReply(redisClient* c) {
  char* p;
  long long value;
  sds reply;

  reply = getReply(c);
  p = strchr(reply, '\r');
  string2ll(reply+1, p-reply-1, &value);
  if (reply != c->buf) sdsfree(reply);
  return value;
}
