#!/bin/bash

echo -n "Enter FTP server username: "
read user

cmd="mirrordir -v -X ./_darcs/current -X ./build -X ./_darcs/pristine . ftp://$user@tnlgame.net/darcs/TnL-source"

echo -e "Will execute:\n\n  $cmd\n"
echo "Ctrl-C to abort, ENTER to continue."
read

exec $cmd

