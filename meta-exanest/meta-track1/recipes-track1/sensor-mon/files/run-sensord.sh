#!/bin/sh
. /usr/local/fbpackages/utils/ast-functions
. /usr/local/fbpackages/utils/gpio_names.sh
echo -n "Setup sensor monitoring for track1... "

# Check for the slots present and run sensord for those slots only.
SLOTS=

for i in `seq 0 ${MAX_SITE}`
do
  if [ $(is_server_prsnt ${i}) == "1" ]; then
    SLOTS="$SLOTS ${DB_SLOT_NAMES[$i]}"
  fi
done

SLOTS="$SLOTS bmc"
echo "Starting Sensor Monitoring of ${SLOTS}"
exec /usr/local/bin/sensord $SLOTS
