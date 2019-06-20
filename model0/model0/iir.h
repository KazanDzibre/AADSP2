#ifndef IIR_H
#define IIR_H
#include <stdio.h>

double first_order_IIR(double input, double* coefficients, double* x_history, double* y_history);
#endif