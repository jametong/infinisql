/*
 * Copyright (c) 2013 Mark Travis <mtravis15432+src@gmail.com>
 * All rights reserved. No warranty, explicit or implicit, provided.
 *
 * This file is part of InfiniSQL(tm).
 
 * InfiniSQL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * InfiniSQL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with InfiniSQL. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MBOX_HPP
#define MBOX_HPP

#include "infinisql_Message.h"
#include "infinisql_Topology.h"

using std::vector;
using std::string;

class Mbox
{
public:
  Mbox();
  virtual ~Mbox();

  class Message *receive(int);  //int is wait

  struct pointer_s
  {
    class Message *ptr;
    uint64_t count;
  };

  static __int128 getInt128FromPointer(class Message *, uint64_t);
  static class Message *getPtr(__int128);
  static uint64_t getCount(__int128);

  friend class MboxProducer;

private:
  pthread_mutex_t mutexLast;

  class Message *firstMsg;
  class Message *currentMsg;
  class Message *lastMsg;
  class Message *myLastMsg; // not to be modified by producer

  __int128 head;
  __int128 tail;
  __int128 current;
  __int128 mytail;

  uint64_t counter;
};

class MboxProducer
{
public:
  MboxProducer();
  MboxProducer(class Mbox *, int16_t);
  virtual ~MboxProducer();
  void sendMsg(class Message &);

  class Mbox *mbox;
  int16_t nodeid;
  class MessageBatchSerialized *obBatchMsg;
  class Mboxes *mboxes;

  friend class Mboxes;
};

/** This class lets each actor keep track of destinations in a consistent way */
class Mboxes
{
public:
  struct location_s
  {
    Topology::addressStruct address;
    class MboxProducer *destmbox;
  };

  Mboxes();
  Mboxes(int64_t);
  virtual ~Mboxes();

  void update(class Topology &);
  void update(class Topology &, int64_t);
  void toActor(const Topology::addressStruct &,
               const Topology::addressStruct &, class Message &);
  void toUserSchemaMgr(const Topology::addressStruct &, class Message &);
  void toDeadlockMgr(const Topology::addressStruct &, class Message &);
  void toPartition(const Topology::addressStruct &, int64_t, class Message &);
  int64_t toAllOfType(actortypes_e, const Topology::addressStruct &,
                      class Message &);
  int64_t toAllOfTypeThisReplica(actortypes_e, const Topology::addressStruct &,
                                 class Message &);
  void sendObBatch();

  int64_t nodeid;

  class MboxProducer topologyMgr;
  class MboxProducer userSchemaMgr;
  class MboxProducer deadlockMgr;
  class MboxProducer listener;
  class MboxProducer obGateway;
  class MboxProducer ibGateway;
  vector<class MboxProducer> transactionAgents;
  vector<class MboxProducer> engines;

  // new
  vector<class MboxProducer *> actoridToProducers;
  vector<class MboxProducer *> transactionAgentPtrs;
  vector<class MboxProducer *> enginePtrs;
  class MboxProducer *topologyMgrPtr;
  location_s userSchemaMgrLocation;
  location_s deadlockMgrLocation;
  class MboxProducer *listenerPtr;
  class MboxProducer *obGatewayPtr;

  vector<location_s> partitionToProducers;
  // allActors[nodeid][actorid] = actortype
  vector< vector<int> > allActors;
  boost::unordered_map< int16_t, vector<int> > allActorsThisReplica;
};

// put this in each actor's class definition (except ObGw)
#define REUSEMESSAGES class Message reuseMessage; \
    class MessageSocket reuseMessageSocket; \
    class MessageUserSchema reuseMessageUserSchema; \
    class MessageDeadlock reuseMessageDeadlock; \
    class MessageSubtransactionCmd reuseMessageSubtransactionCmd; \
    class MessageCommitRollback reuseMessageCommitRollback; \
    class MessageDispatch reuseMessageDispatch; \
    class MessageAckDispatch reuseMessageAckDispatch; \
    class MessageApply reuseMessageApply; \
    class MessageAckApply reuseMessageAckApply;

// do this instead of mymbox.receive (except ObGw)
#define GETMSG(X, Y, Z) \
      X=Y->receive(Z); \
      if (X != NULL && X->messageStruct.topic==TOPIC_SERIALIZED) \
      { \
        SerializedMessage serobj(((class MessageSerialized *)X)->data); \
        switch (serobj.getpayloadtype()) \
        { \
          case PAYLOADMESSAGE: \
          { \
            reuseMessage=Message(); \
            reuseMessage.unpack(serobj); \
            if (serobj.data->size() != serobj.pos) \
            { \
              fprintf(logfile, "unpack %i size %lu pos %lu\n", serobj.getpayloadtype(), serobj.data->size(), serobj.pos); \
            } \
            X=&reuseMessage; \
          } \
          break; \
          case PAYLOADSOCKET: \
          { \
            reuseMessageSocket=MessageSocket(); \
            reuseMessageSocket.unpack(serobj); \
            if (serobj.data->size() != serobj.pos) \
            { \
              fprintf(logfile, "unpack %i size %lu pos %lu\n", serobj.getpayloadtype(), serobj.data->size(), serobj.pos); \
            } \
            X=&reuseMessageSocket; \
          } \
          break; \
          case PAYLOADUSERSCHEMA: \
          { \
            reuseMessageUserSchema=MessageUserSchema(); \
            reuseMessageUserSchema.unpack(serobj); \
            if (serobj.data->size() != serobj.pos) \
            { \
              fprintf(logfile, "unpack %i size %lu pos %lu\n", serobj.getpayloadtype(), serobj.data->size(), serobj.pos); \
            } \
            X=&reuseMessageUserSchema; \
          } \
          break; \
          case PAYLOADDEADLOCK: \
          { \
            reuseMessageDeadlock=MessageDeadlock(); \
            reuseMessageDeadlock.unpack(serobj); \
            if (serobj.data->size() != serobj.pos) \
            { \
              fprintf(logfile, "unpack %i size %lu pos %lu\n", serobj.getpayloadtype(), serobj.data->size(), serobj.pos); \
            } \
            X=&reuseMessageDeadlock; \
          } \
          break; \
          case PAYLOADSUBTRANSACTION: \
           { \
            reuseMessageSubtransactionCmd=MessageSubtransactionCmd(); \
            reuseMessageSubtransactionCmd.unpack(serobj); \
            if (serobj.data->size() != serobj.pos) \
            { \
              fprintf(logfile, "unpack %i size %lu pos %lu\n", serobj.getpayloadtype(), serobj.data->size(), serobj.pos); \
            } \
            X=&reuseMessageSubtransactionCmd; \
          } \
          break; \
          case PAYLOADCOMMITROLLBACK: \
          { \
            reuseMessageCommitRollback=MessageCommitRollback(); \
            reuseMessageCommitRollback.unpack(serobj); \
            if (serobj.data->size() != serobj.pos) \
            { \
              fprintf(logfile, "unpack %i size %lu pos %lu\n", serobj.getpayloadtype(), serobj.data->size(), serobj.pos); \
            } \
            X=&reuseMessageCommitRollback; \
          } \
          break; \
          case PAYLOADDISPATCH: \
          { \
            reuseMessageDispatch=MessageDispatch(); \
            reuseMessageDispatch.unpack(serobj); \
            if (serobj.data->size() != serobj.pos) \
            { \
              fprintf(logfile, "unpack %i size %lu pos %lu\n", serobj.getpayloadtype(), serobj.data->size(), serobj.pos); \
            } \
            X=&reuseMessageDispatch; \
          } \
          break; \
          case PAYLOADACKDISPATCH: \
          { \
            reuseMessageAckDispatch=MessageAckDispatch(); \
            reuseMessageAckDispatch.unpack(serobj); \
            if (serobj.data->size() != serobj.pos) \
            { \
              fprintf(logfile, "unpack %i size %lu pos %lu\n", serobj.getpayloadtype(), serobj.data->size(), serobj.pos); \
            } \
            X=&reuseMessageAckDispatch; \
          } \
          break; \
          case PAYLOADAPPLY: \
          { \
            reuseMessageApply=MessageApply(); \
            reuseMessageApply.unpack(serobj); \
            if (serobj.data->size() != serobj.pos) \
            { \
              fprintf(logfile, "unpack %i size %lu pos %lu\n", serobj.getpayloadtype(), serobj.data->size(), serobj.pos); \
            } \
            X=&reuseMessageApply; \
          } \
          break; \
          case PAYLOADACKAPPLY: \
          { \
            reuseMessageAckApply=MessageAckApply(); \
            reuseMessageAckApply.unpack(serobj); \
            if (serobj.data->size() != serobj.pos) \
            { \
              fprintf(logfile, "unpack %i size %lu pos %lu\n", serobj.getpayloadtype(), serobj.data->size(), serobj.pos); \
            } \
            X=&reuseMessageAckApply; \
          } \
          break; \
          default: \
            printf("%s %i anomaly %i\n", __FILE__, __LINE__, serobj.getpayloadtype()); \
        } \
        delete serobj.data; \
      }

#endif  /* MBOX_HPP */
