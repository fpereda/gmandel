#!/usr/bin/env bash

if test "xyes" = x"${BASH_VERSION}" ; then
	echo "This is not bash!"
	exit 127
fi

trap 'echo "exiting." ; exit 250' 15
KILL_PID=$$

run() {
	echo ">>> $@" 1>&2
	if ! $@ ; then
		echo "oops!" 1>&2
		exit 127
	fi
}

get() {
	local n=${1} v=
	shift

	for p in ${n} g${n} ; do
		for v in ${@} ; do
			type ${p}-${v}    &>/dev/null && echo ${p}-${v}   && return
			type ${p}${v//.}  &>/dev/null && echo ${p}${v//.} && return
		done
		type ${p}             &>/dev/null && echo ${p}        && return
	done
	echo "Could not find ${n}" 1>&2
	kill $KILL_PID
}

run mkdir -p config
run $(get libtoolize 1.5 ) --copy --force --automake
rm -f config.cache

case $(uname -s) in
	Darwin)
		run $(get aclocal 1.10 1.9 ) -I /opt/local/share/aclocal
	;;
	*)
		run $(get aclocal 1.10 1.9 )
	;;
esac

run $(get autoheader 2.61 2.60 2.59 )
run $(get autoconf 2.61 2.60 2.59 )
run $(get automake 1.10 1.9 ) -a --copy --foreign
