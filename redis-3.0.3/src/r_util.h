#ifndef __R_UTIL_H
#define __R_UTIL_H

#include "redis.h"

redisClient* createSubClient(int start, int len, int cap, robj** argv);
void freeSubClient(redisClient* c);

long long getLongLongReply(redisClient* c);

#endif
