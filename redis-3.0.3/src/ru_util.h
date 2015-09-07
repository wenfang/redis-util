#ifndef __R_UTIL_H
#define __R_UTIL_H

#include "redis.h"

#define RU_TYPE_STATUS    0
#define RU_TYPE_ERROR     1
#define RU_TYPE_LONGLONG  2
#define RU_TYPE_BULK      3
#define RU_TYPE_ARRAY     4
#define RU_TYPE_UNKNOWN   5

#define RU_REPLY_OK   0
#define RU_REPLY_ERR  -1

typedef struct ru_reply {
  int       type;
  long long value;
  sds*      bulks;
  long long mbulks;
} ru_reply;

redisClient* createSubClient(int start, int len, int cap, robj** argv);
void freeSubClient(redisClient* c);

ru_reply* createRuReply(redisClient* c);
void freeRuReply(ru_reply* reply);

#endif
