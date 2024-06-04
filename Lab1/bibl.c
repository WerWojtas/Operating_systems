

int collatz_conjecture(int input){
    if(input%2 == 0){
        input = input/2;
    }
    else{
        input = input*3 + 1;
    }
}

int test_collatz_convergence(int input, int max_iter){
    int flag = 0;
    while((flag < max_iter) && (input != 1)){
        input = collatz_conjecture(input);
        flag+=1;
    }
    if(input != 1){
        return -1;
    }
    return flag;
}