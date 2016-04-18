#!/bin/sh

# This script is called by uscan as
#     debian/edit-orig-tar.sh --upstream-version $VERSION $FILENAME

set -e

VERSION=$2
FILE=$3

NEWFILE=../bro_$VERSION+dfsg.orig.tar.xz

echo "Editing $FILE > $NEWFILE ..."

zcat $FILE \
    | tar --verbose --wildcards --delete -f - '*-minimal/doc' \
    | xz -c > $NEWFILE.t

mv $NEWFILE.t $NEWFILE
