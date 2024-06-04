CXX=gcc
CFLAGS=-Wall -Wextra -Werror -std=c99 -pedantic -g

.PHONY: all static shared customer_static customer_dynamic customer_shared clean

all: static shared customer_static customer_dynamic customer_shared

static:
	$(CXX) -c bibl.c
	ar rcs bibl.a bibl.o

shared:
	$(CXX) -fPIC -c bibl.c
	$(CXX) -shared bibl.o -o bibl.so

customer_static:
	$(CXX) $(CFLAGS) customer.c bibl.a -o customer_st

customer_dynamic:
	$(CXX) $(CFLAGS) -DUSE_DYNAMIC_LOADING customer.c -o customer_dyn -ldl

customer_shared:
	$(CXX) $(CFLAGS) customer.c -L. -l:bibl.so -Wl,-rpath=. -o customer_sh

customer_static_OO:
	$(CXX) -O0 customer.c bibl.a -o customer_st_OO

customer_dynamic_OO:
	$(CXX) -O0 -DUSE_DYNAMIC_LOADING customer.c -o customer_dyn_OO -ldl

customer_shared_OO:
	$(CXX) -O0 customer.c -L. -l:bibl.so -Wl,-rpath=. -o customer_sh_OO

clean:
	-rm *.o *.so *a customer_st customer_dyn customer_sh customer_dyn_OO customer_st_OO customer_sh_OO