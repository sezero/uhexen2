gcc -c -O3 wss.c
ar rvs libwss.a wss.o
ranlib libwss.a
rm wss.o