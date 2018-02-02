#!/bin/sh
# Copyright (c) 2017-2017 The Catoshi developers
# Distributed under the AGPLv3 software license, see the accompanying
# file COPYING or http://www.gnu.org/licenses/agpl-3.0.html 

if [ $# -gt 1 ]; then
    cd "$2"
fi
if [ $# -gt 0 ]; then
    FILE="$1"
    shift
    if [ -f "$FILE" ]; then
        INFO="$(head -n 1 "$FILE")"
    fi
else
    echo "Usage: $0 <filename> <srcroot>"
    exit 1
fi

if [ -e "$(which hg)" ]; then
    # get a string like "472fdd7d1efd+"
    DESC="$(hg identify -i)"

    # get a string like "2012-04-10 16:27:19 +0200"
    TIME="$(hg tip --template "{date|isodate}\n")"
fi

if [ -n "$DESC" ]; then
    NEWINFO="#define BUILD_DESC \"$DESC\""
elif [ -n "$SUFFIX" ]; then
    NEWINFO="#define BUILD_SUFFIX $SUFFIX"
else
    NEWINFO="// No build information available"
fi

# only update build.h if necessary
if [ "$INFO" != "$NEWINFO" ]; then
    cat > "$FILE" <<EOF
$NEWINFO
#define BUILD_DATE "$TIME"
EOF
fi
