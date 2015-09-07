#include "ru_util.h"

redisClient* createSubClient(int start, int len, int cap, robj** argv) {
  int i;
  redisClient *subClient;

  subClient = createClient(-1);
  subClient->flags |= REDIS_LUA_CLIENT;
  subClient->argc = 0;
  subClient->argv = zmalloc(sizeof(robj*)*cap);
  subClient->argv[subClient->argc++] = createStringObject("sub", 3);
  for (i=0; i<len; i++) {
    subClient->argv[subClient->argc++] = dupStringObject(argv[start+i]);
  }

  return subClient;
}

void freeSubClient(redisClient* c) {
  freeClient(c);
}

static sds getReplySds(redisClient* c) {
  sds replySds;
  if (listLength(c->reply) == 0 && c->bufpos < REDIS_REPLY_CHUNK_BYTES) {
    c->buf[c->bufpos] = '\0';
    replySds = c->buf;
    c->bufpos = 0;
  } else {
    replySds = sdsnewlen(c->buf, c->bufpos);
    c->bufpos = 0;
    while (listLength(c->reply)) {
      robj* o = listNodeValue(listFirst(c->reply));
      replySds = sdscatlen(replySds, o->ptr, sdslen(o->ptr));
      listDelNode(c->reply, listFirst(c->reply));
    }
  }
  return replySds;
}

static int getReplyType(sds replySds) {
  char* p = replySds;
  switch (*p) {
  case '+':
    return RU_TYPE_STATUS;
  case '-':
    return RU_TYPE_ERROR;
  case ':':
    return RU_TYPE_LONGLONG;
  case '$':
    return RU_TYPE_BULK;
  case '*':
    return RU_TYPE_ARRAY;
  }
  return RU_TYPE_UNKNOWN;
}

static long long getLongLong(sds replySds) {
  char* p;
  long long value;

  p = strchr(replySds, '\r');
  string2ll(replySds+1, p-replySds-1, &value);

  return value;
}

static sds getStatus(sds replySds) {
  char* p = strchr(replySds, '\r');
  return sdsnewlen(replySds+1, p-replySds-1);
}

static sds getBulk(sds replySds, long long *step) {
  long long bulklen;
  char* p;

  p = strchr(replySds, '\r');
  string2ll(replySds+1, p-replySds-1, &bulklen);
  if (bulklen == -1) {
    if (step != NULL) *step = p+2-replySds;
    return sdsempty();
  }
  if (step != NULL) *step = p+2+bulklen+2-replySds;
  return sdsnewlen(p+2, bulklen);
}

ru_reply* createRuReply(redisClient* c) {
  sds replySds;
  ru_reply* reply;
  char* p;
  int i;
  long long step;

  reply = zmalloc(sizeof(ru_reply));
  if (!reply) {
    return NULL;
  }

  replySds = getReplySds(c);
  reply->type = getReplyType(replySds);
  switch (reply->type) {
  case RU_TYPE_STATUS:
  case RU_TYPE_ERROR:
    reply->mbulks = 1;
    reply->bulks = zmalloc(sizeof(sds)*reply->mbulks);
    reply->bulks[0] = getStatus(replySds);
    break;
  case RU_TYPE_LONGLONG:
    reply->mbulks = 0;
    reply->bulks = NULL;
    reply->value = getLongLong(replySds);
    break;
  case RU_TYPE_BULK:
    reply->mbulks = 1;
    reply->bulks = zmalloc(sizeof(sds)*reply->mbulks);
    reply->bulks[0] = getBulk(replySds, NULL); 
    break;
  case RU_TYPE_ARRAY:
    p = strchr(replySds, '\r');
    string2ll(replySds+1, p-replySds-1, &reply->mbulks);
    reply->bulks = zmalloc(sizeof(sds)*reply->mbulks);
    p += 2;
    for (i=0; i<reply->mbulks; i++) {
      reply->bulks[i] = getBulk(p, &step);
      p += step;
    } 
    break;
  default:
    reply->mbulks = 0;
    reply->bulks = NULL;
    reply->value = 0;
  }

  if (replySds != c->buf) sdsfree(replySds);
  return reply;
}

void freeRuReply(ru_reply* reply) {
  int i;
  if (reply->mbulks) {
    for (i=0; i<reply->mbulks; i++) {
      sdsfree(reply->bulks[i]);
    }
    zfree(reply->bulks);
  }
  zfree(reply);
}
