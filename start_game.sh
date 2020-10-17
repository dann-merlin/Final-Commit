#!/bin/bash
SCRIPTPATH=$(readlink -f "$0")
DIR=$(dirname "$SCRIPTPATH")
cd "$DIR/applications/freestyle" || exit
./freestyle "$@" >/dev/null 2>&1 & disown
cd - >/dev/null || exit
