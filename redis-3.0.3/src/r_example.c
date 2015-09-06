#include "r_util.h"

void exampleCommand(redisClient* c) {
  int res;
  r_reply reply;
  redisClient *subClient;

  initrReply(&reply);
  subClient = createSubClient(1, 1, 2, c->argv);
  incrCommand(subClient);
  res = getrReply(subClient, &reply);
  freeSubClient(subClient);

  if (res != R_REPLY_OK || reply.type != R_TYPE_LONGLONG) {
    addReplyError(c, "inner error");
    resetrReply(&reply);
    return;
  }
  resetrReply(&reply);

  subClient = createSubClient(2, 1, 2, c->argv);
  incrCommand(subClient);
  res = getrReply(subClient, &reply);
  freeSubClient(subClient);

  if (res != R_REPLY_OK || reply.type != R_TYPE_LONGLONG) {
    addReplyError(c, "inner error");
    resetrReply(&reply);
    return;
  }
  resetrReply(&reply);

  addReply(c, shared.ok);
}
