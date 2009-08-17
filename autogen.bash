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

case $(uname -s) in
	Darwin)
		export PATH=/opt/local/bin:$PATH
	;;
esac

run mkdir -p config
run $(get libtoolize 2.2 ) --copy --force --automake
rm -f config.cache

case $(uname -s) in
	Darwin)
		run $(get aclocal 1.11 ) -I /opt/local/share/aclocal
	;;
	*)
		run $(get aclocal 1.11 )
	;;
esac

run $(get autoheader 2.64 )
run $(get autoconf 2.64 )
run $(get automake 1.11 ) -a --copy --foreign
