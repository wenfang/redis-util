#include "r_util.h"

void exampleCommand(redisClient* c) {
  int res;
  sds reply = sdsempty();
  redisClient *subClient;

  subClient = createSubClient(1, 1, 2, c->argv);
  getCommand(subClient);
  res = getBulkReply(subClient, &reply);
  freeSubClient(subClient);

  if (res != R_REPLY_OK) {
    addReplyError(c, "example error");
    return;
  }
  addReplySds(c, reply);
}
