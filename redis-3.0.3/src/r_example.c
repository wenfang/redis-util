#include "r_util.h"

void exampleCommand(redisClient* c) {
  int res;
  r_reply reply;
  redisClient *subClient;

  initrReply(&reply);
  subClient = createSubClient(1, 1, 2, c->argv);
  getCommand(subClient);
  res = getrReply(subClient, &reply);
  freeSubClient(subClient);

  if (res != R_REPLY_OK || reply.type != R_TYPE_BULK) {
    addReplyError(c, "inner error");
    return;
  }
  addReplyLongLongWithPrefix(c, sdslen(reply.bulks[0]), '$');
  addReplySds(c, reply.bulks[0]);
  addReply(c, shared.crlf);

  resetrReply(&reply);
}
