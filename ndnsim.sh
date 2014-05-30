#!/bin/bash

cp /tmp/icn-cache-privacy/*.sh /ndnSIM/ns-3
cp /tmp/icn-cache-privacy/*.cc /ndnSIM/ns-3/scratch
cp /tmp/icn-cache-privacy/extensions/* /ndnSIM/ns-3/src/ndnSIM/apps
cp /tmp/icn-cache-privacy/*.py /ndnSIM/ns-3
pushd topology-generator
python gen.py > topology.txt
mv topology.txt /ndnSIM/ns-3
