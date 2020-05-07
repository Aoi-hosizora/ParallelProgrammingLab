double f(double a) {
    return a * a * a;
}

double trap(double a, double b, int n) {
    double integral, h;
    int k;

    h = (b - a) / n;
    integral = (f(a) + f(b)) / 2.0;
    for (k = 1; k <= n - 1; k++) {
        integral += f(a + k * h);
    }
    integral *= h;
    return integral;
}
