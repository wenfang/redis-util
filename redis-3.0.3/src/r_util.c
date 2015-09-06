#include "r_util.h"

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

static sds getBulk(sds replySds) {
  long long bulklen;
  char* p;

  p = strchr(replySds, '\r');
  string2ll(replySds+1, p-replySds-1, &bulklen);
  if (bulklen == -1) {
    return NULL;
  }
  return sdsnewlen(p+2, bulklen);
}

/*
int getArrayReply(redisClient* c, sds** value, long long* len) {
  sds reply;
  char* p;
  int i;

  reply = getReply(c);
  if (getReplyType(reply) != R_TYPE_ARRAY) {
    return R_REPLY_ERR;
  }

  p = strchr(reply, '\r');
  string2ll(reply+1, p-reply-1, len);
  p += 2;
  if (*len == -1) {
    return R_REPLY_ERR;
  }
  *value = zcalloc(sizeof(sds)*(*len));
  if (*value == NULL) {
    return R_REPLY_ERR;
  }

  for (i=0; i<len; i++) {
    (*value)[i] = getOneBulk(p);
  }


  if (reply != c->buf) sdsfree(reply);
  return R_REPLY_OK;
}
*/

int getrReply(redisClient* c, r_reply* reply) {
  int res = R_REPLY_OK;
  sds replySds;

  replySds = getReplySds(c);
  reply->type = getReplyType(replySds);
  if (reply->type == R_TYPE_LONGLONG) {
    reply->value = getLongLong(replySds);
  } else if (reply->type == R_TYPE_STATUS || reply->type == R_TYPE_ERROR) {
    reply->mbulks = 1;
    reply->bulks = zcalloc(sizeof(sds)*reply->mbulks);
    reply->bulks[0] = getStatus(replySds); 
  } else if (reply->type == R_TYPE_BULK) {
    reply->mbulks = 1;
    reply->bulks = zcalloc(sizeof(sds)*reply->mbulks);
    reply->bulks[0] = getBulk(replySds);
    if (reply->bulks[0] == NULL) res = R_REPLY_ERR;
  } else if (reply->type == R_TYPE_ARRAY) {
  }

  if (replySds != c->buf) sdsfree(replySds);
  return res;
}

void initrReply(r_reply* reply) {
  reply->type = R_TYPE_UNKNOWN;
  reply->value = 0;
  reply->bulks = NULL;
  reply->mbulks = 0; 
}

void resetrReply(r_reply* reply) {
  int i;

  reply->type = R_TYPE_UNKNOWN;
  reply->value = 0;

  for (i=0; i<reply->mbulks; i++) {
    sdsfree(reply->bulks[i]);
  }
  reply->bulks = NULL;

  reply->mbulks = 0;
}
