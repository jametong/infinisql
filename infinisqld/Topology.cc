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

#include "infinisql_Topology.h"
#line 22 "Topology.cc"

Topology::Topology() : nodeid(0), numreplicas(1), activereplica(-1),
  numtransactionagents(0),
  numengines(0), numobgateways(0), numpartitions(0), userSchemaMgrNode(0),
  userSchemaMgrMbox(0), deadlockMgrNode(0), deadlockMgrMbox(0)
{
}

Topology::Topology(const Topology &orig)
{
}

Topology::~Topology()
{
}

Topology::partitionAddress *Topology::newActor(actortypes_e type,
    class Mbox *mbox, int epollfd, const string &argstring,
    int64_t actorid, const vector<string> &nodes,
    const vector<string> &services)
{
  partitionAddress *addr = new partitionAddress();
  partitionAddress &addrRef = *addr;
  addrRef.type = type;
  addrRef.mbox = mbox;
  addrRef.address.nodeid = nodeid;
  addrRef.address.actorid = actorid;
  addrRef.epollfd = epollfd;
  addrRef.nodes = nodes;
  addrRef.services = services;
  addrRef.argstring = argstring;

  if (addr==NULL)
  {
    printf("%s %i ERROR\n", __FILE__, __LINE__);
  }

  return addr;
}
