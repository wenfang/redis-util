#include "r_util.h"

#define R_TYPE_STATUS    0
#define R_TYPE_ERROR     1
#define R_TYPE_LONGLONG  2
#define R_TYPE_BULK      3
#define R_TYPE_ARRAY     4
#define R_TYPE_UNKNOWN   5

redisClient* createSubClient(int start, int len, int cap, robj** argv) {
  int i;
  redisClient *subClient;

  subClient = createClient(-1);
  subClient->flags |= REDIS_LUA_CLIENT;
  subClient->argc = 0;
  subClient->argv = zmalloc(sizeof(robj*)*cap);
  subClient->argv[subClient->argc++] = createStringObject("fack", 4);
  for (i=0; i<len; i++) {
    subClient->argv[subClient->argc++] = dupStringObject(argv[start+i]);
  }

  return subClient;
}

void freeSubClient(redisClient* c) {
  freeClient(c);
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

static int getReplyType(char* reply) {
  char* p = reply;
  switch (*p) {
  case '+':
    return R_TYPE_STATUS;
  case '-':
    return R_TYPE_ERROR;
  case ':':
    return R_TYPE_LONGLONG;
  case '$':
    return R_TYPE_BULK;
  case '*':
    return R_TYPE_ARRAY;
  }
  return R_TYPE_UNKNOWN;
}

int getLongLongReply(redisClient* c, long long* value) {
  sds reply;
  char* p;

  reply = getReply(c);
  if (getReplyType(reply) != R_TYPE_LONGLONG) {
    return R_REPLY_ERR;
  }

  p = strchr(reply, '\r');
  string2ll(reply+1, p-reply-1, value);
  if (reply != c->buf) sdsfree(reply);
  return R_REPLY_OK;
}

int getBulkReply(redisClient* c, sds* value) {
  sds reply;
  char* p;
  long long bulklen;
  
  reply = getReply(c);
  if (getReplyType(reply) != R_TYPE_BULK) {
    return R_REPLY_ERR;
  }

  p = strchr(reply, '\r');
  string2ll(reply+1, p-reply-1, &bulklen);
  if (bulklen == -1) {
    return R_REPLY_ERR;
  }
  *value = sdscatlen(*value, p+2, bulklen);

  if (reply != c->buf) sdsfree(reply);
  return R_REPLY_OK;
}
