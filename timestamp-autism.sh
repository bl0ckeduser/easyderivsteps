for x in `ls *.c *.h`
do
	echo $x
	if ! (diff -q ../symdiff-js-1.0/symdiff-js/$x $x | grep differ >/dev/null)
	then
		cp -a ../symdiff-js-1.0/symdiff-js/$x .
	fi
done

for x in `ls rules/*`
do
	if ! (diff -q ../symdiff-js-1.0/symdiff-js/$x $x | grep differ >/dev/null)
	then
		cp -a ../symdiff-js-1.0/symdiff-js/$x rules/
	fi
done

for x in `ls pres-rules/*`
do
	if ! (diff -q ../symdiff-js-1.0/symdiff-js/$x $x | grep differ >/dev/null)
	then
		cp -a ../symdiff-js-1.0/symdiff-js/$x pres-rules/
	fi
done
