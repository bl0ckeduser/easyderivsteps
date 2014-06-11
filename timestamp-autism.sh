for x in `ls *`
do
	if ! (diff -q ../symdiff-js-1.0/symdiff-js/$x $x | grep differ >/dev/null)
	then
		cp -a ../symdiff-js-1.0/symdiff-js/$x .
	fi
done
