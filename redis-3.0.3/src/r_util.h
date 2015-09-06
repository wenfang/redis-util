#ifndef __R_UTIL_H
#define __R_UTIL_H

#include "redis.h"

#define R_REPLY_OK  0
#define R_REPLY_ERR -1

redisClient* createSubClient(int start, int len, int cap, robj** argv);
void freeSubClient(redisClient* c);

int getLongLongReply(redisClient* c, long long* value);
int getBulkReply(redisClient* c, sds* value);
int getArrayReply(redisClient* c, sds** value, long long* len);

#endif
