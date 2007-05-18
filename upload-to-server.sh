#!/bin/bash

echo -n "Enter FTP server username: "
read user

cmd="mirrordir -v --exclude ./_darcs/current . ftp://$user@tnlgame.net/darcs/TnL-source"

echo -e "Will execute:\n\n  $cmd\n"
echo "Ctrl-C to abort, ENTER to continue."
read

exec $cmd

