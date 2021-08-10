#!/bin/sh -e
pushd ../
scripts/make_gitrev.sh
popd
GITREV="../include/zfs_gitrev.h"
[ -f "$GITREV" ] || exit 1
cat "$GITREV" > gitrevision.h
