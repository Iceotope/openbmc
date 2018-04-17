#!/bin/sh
. /usr/local/fbpackages/utils/ast-functions
. /usr/local/fbpackages/utils/gpio_names.sh
echo -n "Setup sensor monitoring for track1... "

# Check for the slots present and run sensord for those slots only.
SLOTS=

# Slots start at 1
for i in `seq 1 ${MAX_SITE}`
do
  if [ $(is_server_prsnt ${i}) == "1" ]; then
    SLOTS="$SLOTS slot${i}"
  fi
done

SLOTS="$SLOTS bmc"
echo "Starting Sensor Monitoring of ${SLOTS}"
exec /usr/local/bin/sensord $SLOTS
