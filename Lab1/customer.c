#include <stdio.h>
#include <dlfcn.h>

extern int test_collatz_convergence(int input, int max_iter);

int main(){
    int number;
    #ifdef USE_DYNAMIC_LOADING
        void *handle = dlopen("./bibl.so", RTLD_LAZY);
        if(!handle){
            fprintf(stderr, "Error: %s\n", dlerror());
            return -1;
        }

        int (*lib_fun)(int, int);


        *(void **)(&lib_fun) = dlsym(handle, "test_collatz_convergence");

        for(int i=10; i<20; i++){
            number = (*lib_fun)(i,i*2);
            printf("%d\n", number);
        }
    #else
        for(int i=10; i<20; i++){
            number = test_collatz_convergence(i,i*2);
            printf("%d\n", number);
        }
    #endif
    return 0;
}