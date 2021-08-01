if [ -z $WATCOM ]
then
	echo "No OpenWatcom toolchain specified - set path in WATCOM environment variable."
	exit 1
fi

export PATH=$WATCOM/binl64:$WATCOM/binl:$PATH
export EDPATH=$WATCOM/eddat
export INCLUDE=$WATCOM/lh

scriptDir=`dirname $(readlink -f $0)`
failures=0

function processFile () {
	echo "Recording $1.obj in linker file"
	echo "FIL $1.obj" >> bootstrap.lk1

	echo "Compiling $1.c"
	wcc386 "$scriptDir/$1.c" -i="$WATCOM/lh" -d__STDC_WANT_LIB_EXT1__=1 -dPLATFORM_LINUX=1 -w4 -we -e25 -zq -ze -od -d2 -6r -bt=linux -fo=.obj -mf

	if [ $? -ne 0 ]; then
		echo "Compilation was not successful for $1.c"
		failures=$((failures+1))
	fi
}

> bootstrap.lk1

echo "Processing"
processFile "main"
processFile "bstfile"
processFile "bstparse"
processFile "path"
processFile "options"
processFile "mkscript"

if [ $failures -ne 0 ]; then
	echo "$failures file(s) failed to compile."
	exit 1
fi

echo "Linking"
wlink name bootstrap d all sys linux op m op maxe=25 op q op symf @bootstrap.lk1

if [ $? -ne 0 ]; then
	echo "Linking was unsuccessful."
	exit 1
fi

echo "Done"