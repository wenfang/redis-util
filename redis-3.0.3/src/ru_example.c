#include "ru_util.h"

void exampleCommand(redisClient* c) {
  ru_reply* reply;
  redisClient *subClient;

  subClient = createSubClient(1, 1, 2, c->argv);
  incrCommand(subClient);
  reply = createRuReply(subClient);
  freeSubClient(subClient);

  if (reply->type != RU_TYPE_LONGLONG) {
    addReplyError(c, "inner error 1");
    freeRuReply(reply);
    return;
  }
  freeRuReply(reply);

  subClient = createSubClient(2, 1, 2, c->argv);
  incrCommand(subClient);
  reply = createRuReply(subClient);
  freeSubClient(subClient);

  if (reply->type != RU_TYPE_LONGLONG) {
    addReplyError(c, "inner error 2");
    freeRuReply(reply);
    return;
  }
  freeRuReply(reply);

  addReply(c, shared.ok);
}
