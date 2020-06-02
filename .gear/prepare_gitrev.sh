#!/bin/sh -e
../scripts/make_gitrev.sh
GITREV="../include/zfs_gitrev.h"
[ -f "$GITREV" ] || exit 1
cat "$GITREV" >> gitrevision.h
