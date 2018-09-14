# ~/.profile: executed by Bourne-compatible login shells.

if [ -f ~/.bashrc ]; then
  . ~/.bashrc
fi

# path set by /etc/profile
# export PATH

mesg n

## Work out the serial port to use, based on user name!

echo "user = ${USER}"
case ${USER} in
  qfdb_a)
    SITE=QFDB_A_2
    ;;

  qfdb_b)
    SITE=QFDB_B_2
    ;;

  qfdb_c)
    SITE=QFDB_C_2
    ;;

  qfdb_d)
    SITE=QFDB_D_2
    ;;
esac

if [ -z ${SITE} ]; then
  echo "Bad Site Selected!"
  exit
else

  echo "Site = ${SITE}"
  /usr/local/bin/mTerm_client ${SITE}
  exit
fi
