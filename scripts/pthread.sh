#!/bin/sh
# based on configure.in of SDL library

if test $# -ne 2; then
	exit 1
fi

host=`sh $1 2> /dev/null`
if test $? -ne 0; then
	exit 1
fi

case "$host" in
    *-*-linux*|*-*-uclinux*)
        pthread_cflags="-D_REENTRANT"
        pthread_lib="-lpthread"
        ;;
    *-*-bsdi*)
        pthread_cflags="-D_REENTRANT -D_THREAD_SAFE"
        pthread_lib=""
        ;;
    *-*-darwin*)
        pthread_cflags="-D_THREAD_SAFE"
# causes Carbon.p complaints?
#       pthread_cflags="-D_REENTRANT -D_THREAD_SAFE"
        pthread_lib="-pthread"
        ;;
    *-*-freebsd*|*-*-dragonfly*)
        pthread_cflags="-D_REENTRANT -D_THREAD_SAFE"
        pthread_lib="-pthread"
        ;;
    *-*-netbsd*)
        pthread_cflags="-D_REENTRANT -D_THREAD_SAFE"
        pthread_lib="-lpthread"
        ;;
    *-*-openbsd*)
        pthread_cflags="-D_REENTRANT"
        pthread_lib="-pthread"
        ;;
    *-*-solaris*)
        pthread_cflags="-D_REENTRANT"
        pthread_lib="-lpthread -lposix4"
        ;;
    *-*-sysv5*)
        pthread_cflags="-D_REENTRANT -Kthread"
        pthread_lib=""
        ;;
    *-*-irix*)
        pthread_cflags="-D_SGI_MP_SOURCE"
        pthread_lib="-lpthread"
        ;;
    *-*-aix*)
        pthread_cflags="-D_REENTRANT -mthreads"
        pthread_lib="-lpthread"
        ;;
    *-*-hpux11*)
        pthread_cflags="-D_REENTRANT"
        pthread_lib="-L/usr/lib -lpthread"
        ;;
    *-*-qnx*)
        pthread_cflags=""
        pthread_lib=""
        ;;
    *-*-osf*)
        pthread_cflags="-D_REENTRANT"
        if test x$ac_cv_prog_gcc = xyes; then
            pthread_lib="-lpthread -lrt"
        else
            pthread_lib="-lpthread -lexc -lrt"
        fi
        ;;
    *)
        pthread_cflags="-D_REENTRANT"
        pthread_lib="-lpthread"
        ;;
esac

if test "$2" = "--cflags"; then
	echo $pthread_cflags
elif test "$2" = "--libs"; then
	echo $pthread_lib
#else
#	echo "bad argument"
fi

