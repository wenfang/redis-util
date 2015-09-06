#include "r_util.h"

void exampleCommand(redisClient* c) {
  int res;
  r_reply reply;
  redisClient *subClient;

  initrReply(&reply);
  subClient = createSubClient(1, 2, 3, c->argv);
  mgetCommand(subClient);
  res = getrReply(subClient, &reply);
  freeSubClient(subClient);

  if (res != R_REPLY_OK || reply.type != R_TYPE_ARRAY) {
    addReplyError(c, "inner error");
    resetrReply(&reply);
    return;
  }
  addReplyLongLongWithPrefix(c, sdslen(reply.bulks[0]) + sdslen(reply.bulks[1]), '$');
  addReplySds(c, reply.bulks[0]);
  addReplySds(c, reply.bulks[1]);
  addReply(c, shared.crlf);

  resetrReply(&reply);
}
