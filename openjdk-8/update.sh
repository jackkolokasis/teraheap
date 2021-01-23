#!/usr/bin/env bash

/opt/spark/spark-2.3.0-kolokasis/sbin/stop-all.sh
cd openjdk8
make
cd -
cd /usr/lib/jvm/java-8-kolokasis/build
sudo rm -rf linux-x86_64-normal-server-release
cd -
cd openjdk8
sudo cp -r build/linux-x86_64-normal-server-release /usr/lib/jvm/java-8-kolokasis/build
cd -

rm -rf /opt/spark/spark-2.3.0-kolokasis/work/app-*

/opt/spark/spark-2.3.0-kolokasis/sbin/start-all.sh
