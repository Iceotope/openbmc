#!/bin/bash

if [ $# -lt 1 ]; then
  echo "Syntax: <CSV register set file>"
  echo "CSV file has the following shape"
  echo "<I2C bus (e.g. 1)>,<I2C address>,<Register address>,<Register value>"
  echo "All values are hexadecimal"
  echo "Lines starting with # are ignored"
  exit -1
fi

echo "Parsing register set file <$1>..."

SAVEIFS=$IFS

while read -r line
do
  IFS=, read -r I2C_DEV I2C_ADDR REG_ADDR REG_VAL <<<"$line"
  if [[ $I2C_DEV =~ \#.* ]]; then
    echo "Skip <$line>"
    continue
  fi
  echo "i2cset -y $I2C_DEV $I2C_ADDR $REG_ADDR $REG_V"

done < "$1"

IFS=$SAVEIFS
