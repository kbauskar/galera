#!/bin/bash
#
# lp:928150
# https://bugs.launchpad.net/codership-mysql/+bug/928150
#
# TEST SETUP
#
# This test starts the cluster and runs 100% autocommit load.
#
# PARAMETERS
#
# Duration of test run
DURATION=${DURATION:-"3600"}
#
#


declare -r DIST_BASE=$(cd $(dirname $0)/../..; pwd -P)
TEST_BASE=${TEST_BASE:-"$DIST_BASE"}

. $TEST_BASE/conf/main.conf
declare -r SCRIPTS="$DIST_BASE/scripts"
. $SCRIPTS/jobs.sh
. $SCRIPTS/action.sh
. $SCRIPTS/kill.sh
. $SCRIPTS/misc.sh

echo "##################################################################"
echo "##             regression test for lp:928150"
echo "##################################################################"

echo "restarting cluster to clean state"
restart


echo "starting load for $DURATION" seconds
SQLGEN=${SQLGEN:-"$DIST_BASE/bin/sqlgen"}

$SQLGEN --user $DBMS_TEST_USER --pswd $DBMS_TEST_PSWD --host $DBMS_HOST \
    --port 3306 --users $DBMS_CLIENTS --duration $DURATION \
    --stat-interval 30 --sess-min 999999 --sess-max 999999 \
    --rollbacks 0.1 --ac-frac 100

echo "checking consistency"
check

echo "stopping cluster"
stop
