#!/bin/sh

TEMPDIR="$(mktemp -d)"
OUTPUT="$(realpath $1)"
shift
cp $@ $TEMPDIR
cd $TEMPDIR
# link files into an object file
ld -r -b binary -o $OUTPUT *
# clean up
rm *
rmdir $TEMPDIR
