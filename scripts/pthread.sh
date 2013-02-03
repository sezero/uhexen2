#!/bin/sh
# based on configure.in of the SDL library

if test $# -ne 2; then
	echo "Usage: pthread.sh <target_triplet> <option>"
	echo "target triplet e.g.: i686-pc-linux"
	echo "option is either --cflags or --libs"
	exit 1
fi

case $1 in
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

case $2 in
    --cflags)
        echo $pthread_cflags
        exit
        ;;
    --libs)
        echo $pthread_lib
        exit
        ;;
    *)
        echo "invalid argument"
        exit 1
        ;;
esac

