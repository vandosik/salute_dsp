#!/usr/bin/env ksh
# set -x
if [ $# -lt 3 ]; then
	exit 1
fi
srcfile="$1"
newfile="$2"
shift 2
variants_cond="function variants_cond(line)
{
	result=2
"
for var in $@; do
variant="${var#no}"
if [ "${variant}" = "$var" ]; then
	cond1=1
# 	cond2=0
else
	cond1=0
# 	cond2=1
fi

# /^#ifdef VARIANT_${variant}/ { write = $cond1; next; variant=${variant}; variant_level++; }
variants_cond="${variants_cond}
	if (line ~ /^#ifdef VARIANT_${variant}/) {
		result=$cond1
	}
"
done
variants_cond="${variants_cond}
	return result
}"
# echo "${variants_cond}" > variants.cond
# exit 1
awk "
${variants_cond}
BEGIN { write=1; i=0 }
{
result=variants_cond(\$0)
if ( result != 2 ) {
# 	print \"result for\" \$0 \"=\" result
	write=result
	curresult_arr[i]=result
# 	print \"curresult_arr[i] for\" i \"=\" curresult_arr[i]
	curresult=curresult_arr[i]
	i++
	next
}
if (\$0 ~ /^#else/) {
	i--
	curresult=curresult_arr[i]
# 	print \"curresult_arr[i] for\" i \"=\" curresult_arr[i]
	write = curresult ? 0 : 1
# 	print \"write for\" \$0 \"=\" write
	next
}
if (\$0 ~ /^#endif/) {
	write = 1
	next
}
if ( write == 1 ) print \$0;
}" $srcfile > $newfile

if cmp -s $newfile $srcfile; then
	rm -vf $newfile
fi
