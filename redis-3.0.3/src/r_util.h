#ifndef __R_UTIL_H
#define __R_UTIL_H

#include "redis.h"

#define R_TYPE_STATUS    0
#define R_TYPE_ERROR     1
#define R_TYPE_LONGLONG  2
#define R_TYPE_BULK      3
#define R_TYPE_ARRAY     4
#define R_TYPE_UNKNOWN   5

#define R_REPLY_OK  0
#define R_REPLY_ERR -1

typedef struct r_reply {
  int       type;
  long long value;
  sds*      bulks;
  int       mbulks;
} r_reply;

redisClient* createSubClient(int start, int len, int cap, robj** argv);
void freeSubClient(redisClient* c);

void initrReply(r_reply* reply);
int getrReply(redisClient* c, r_reply* reply);
void resetrReply(r_reply* reply);

#endif
