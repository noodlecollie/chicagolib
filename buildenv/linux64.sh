if [ -z $WATCOM ]
then
	echo "No OpenWatcom toolchain specified - set path in WATCOM environment variable."
	exit 1
fi

export PATH=$WATCOM/binl64:$WATCOM/binl:$PATH
export EDPATH=$WATCOM/eddat
export INCLUDE=$WATCOM/lh
