#!/bin/bash

echo -n "Enter FTP server username: "
read user

cmd="mirrordir -v --exclude ./_darcs/current _darcs ftp://$user@tnlgame.net/darcs/TnL-tools/_darcs"

echo -e "Will execute:\n\n  $cmd\n"
echo "Ctrl-C to abort, ENTER to continue."
read

echo $cmd

exec $cmd

