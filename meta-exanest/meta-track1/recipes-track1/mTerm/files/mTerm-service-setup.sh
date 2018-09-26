#!/bin/sh

# PLATFORM MAY CHOOSE TO OVERRIDE THIS FILE
echo "Starting mTerm console server..."
runsv /etc/sv/mTerm1 > /dev/null 2>&1 &
runsv /etc/sv/mTerm2 > /dev/null 2>&1 &
runsv /etc/sv/mTerm3 > /dev/null 2>&1 &
runsv /etc/sv/mTerm4 > /dev/null 2>&1 &
runsv /etc/sv/mTerm5 > /dev/null 2>&1 &
runsv /etc/sv/mTerm6 > /dev/null 2>&1 &
runsv /etc/sv/mTerm7 > /dev/null 2>&1 &
runsv /etc/sv/mTerm8 > /dev/null 2>&1 &
runsv /etc/sv/mTerm9 > /dev/null 2>&1 &
runsv /etc/sv/mTerm10 > /dev/null 2>&1 &
runsv /etc/sv/mTerm11 > /dev/null 2>&1 &
runsv /etc/sv/mTerm12 > /dev/null 2>&1 &
# Fix permissions, now done in the service files.
#sleep 2
#chmod g+w /var/run/mTerm*_socket
#chgrp serialoverlan /var/run/mTerm*_socket
echo "done."
