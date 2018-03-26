#!/usr/bin/env ksh
# set -x
if [ $# -ne 3 ]; then
	exit 1
fi
srcfile=$1
if [ "$2" = "ksz" ]; then
	cond1=1
	cond2=0
	newfile="$3"
elif [ "$2" = "noksz" ]; then
	cond1=0
	cond2=1
	newfile="$3"
else
	exit 1
fi

awk -vcond1=$cond1 -vcond2=$cond2 '
BEGIN { write=1; }
/^#ifdef VARIANT_ksz/ { write = cond1; next;}
/^#else/              { write = cond2; next;}
/^#endif/             { write = 1; next;}
{
	if ( write == 1 ) print $0;
}' $srcfile > $newfile

if cmp -s $newfile $srcfile; then
	rm -vf $newfile
fi
