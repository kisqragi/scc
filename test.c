int fib(int n) {
    if (n <= 1) 
        return n;
    return fib(n-1) + fib(n-2);
}

int main() {
    int n = 8;
    printf("fib(%d) : %d\n", n, fib(n));
    return 0;
}