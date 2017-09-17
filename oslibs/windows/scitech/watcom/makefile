# Open Watcom makefile to generate mglfxi.lib
# def2lbc.awk (from OW source tree) requires 'awk'
# run 'wmake'

all: mglfxi.lib

mglfx.exp: mglfx.def def2lbc.awk
	awk -v cpu=386 -f def2lbc.awk mglfx.def > mglfx.exp

mglfxi.lib: mglfx.exp
	wlib -q -b -n -pa -s -t -zld -ii -io mglfxi.lib @mglfx.exp
