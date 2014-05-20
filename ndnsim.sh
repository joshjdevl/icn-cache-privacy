cp /tmp/icn-cache-privacy/*.sh /ndnSIM/ns-3
cp /tmp/icn-cache-privacy/*.cc /ndnSIM/ns-3/scratch
cp /tmp/icn-cache-privacy/extensions/* /ndnSIM/ns-3/src/ndnSIM/apps
pushd topology-generator
python gen.py > topology.txt
mv topology.txt /ndnSIM/ns-3
