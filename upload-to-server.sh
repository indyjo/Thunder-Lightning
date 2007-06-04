#!/bin/bash

echo -n "Enter FTP server username: "
read user

cmd="mirrordir -v --exclude ./fonts --exclude ./lod-terrains --exclude ./models --exclude ./scripts --exclude ./sounds --exclude ./textures --exclude ./_darcs/current --exclude ./share . ftp://$user@tnlgame.net/darcs/TnL-data"

echo -e "Will execute:\n\n  $cmd\n"
echo "Ctrl-C to abort, ENTER to continue."
read

echo $cmd

exec $cmd

