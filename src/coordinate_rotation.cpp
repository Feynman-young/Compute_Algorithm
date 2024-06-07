#include <iostream>
#include <cmath>
#include <cstdint>
#include <iomanip>

/*
 * @ CORDIC algorithm
 * implement for compute sin(x)
*/

#define ADD_OP 1
#define SUB_OP 2
#define NUL_OP 0
#define ERR 1e-06
#define X 1
#define Y 0

struct TrigonometricValue {
    TrigonometricValue() : cosine(0), sine(0) {}
    double cosine;
    double sine;
};

// The const radian table, specifically tan(\theta_i) = 2^(-i)
const double thetas[20] = {
        0.7853981633974483,
        0.4636476090008061,
        0.24497866312686414,
        0.12435499454676144,
        0.06241880999595735,
        0.031239833430268277,
        0.015623728620476831,
        0.007812341060101111,
        0.0039062301319669718,
        0.0019531225164788188,
        0.0009765621895593195,
        0.0004882812111948983,
        0.00024414062014936177,
        0.00012207031189367021,
        6.103515617420877e-05,
        3.0517578115526096e-05,
        1.5258789061315762e-05,
        7.62939453110197e-06,
        3.814697265606496e-06,
        1.907348632810187e-06,
};

// The cosine values of table thetas.
const double cosine[20] = {
        0.7071067811865476,
        0.8944271909999159,
        0.9701425001453319,
        0.9922778767136676,
        0.9980525784828885,
        0.9995120760870788,
        0.9998779520346953,
        0.9999694838187878,
        0.9999923706927791,
        0.9999980926568242,
        0.9999995231631829,
        0.9999998807907318,
        0.999999970197679,
        0.9999999925494195,
        0.9999999981373549,
        0.9999999995343387,
        0.9999999998835847,
        0.9999999999708962,
        0.999999999992724,
        0.999999999998181,
};

// Use the values in the table to approximate the input x
// 0 <= alpha <= \pi / 2
// If alpha is out of bounds, we can use trigonometric transformations.
int approximate(double alpha,  uint8_t operations[20]) {
    double sum = 0;
    for (int i = 0; i < 20; i++) {
        double sum_add = sum + thetas[i];
        double sum_sub = sum - thetas[i];
        double distance_add = std::abs(alpha - sum_add);
        double distance_sub = std::abs(alpha - sum_sub);
        double distance_nul = std::abs(alpha - sum);

        if (distance_add < distance_sub) {
            if (distance_add < distance_nul) {
                operations[i] = ADD_OP;
                sum = sum_add;
            } else {
                operations[i] = NUL_OP;
            }
        } else {
            if (distance_sub < distance_nul) {
                operations[i] = SUB_OP;
                sum = sum_sub;
            } else {
                operations[i] = NUL_OP;
            }
        }

        if (std::abs(alpha - sum) < ERR) return (i + 1);
    }
    return 20;
}

// Modify the exponential digits of floating-point numbers directly
double right_shift (double value, int shift) {
    int exponent;
    double significand = std::frexp(value, &exponent);
    exponent -= shift;
    return std::ldexp(significand, exponent);
}

// computer the sin
TrigonometricValue c_sin(double x) {

    double x_in = X, x_out;
    double y_in = Y, y_out;
    double mul = 1;

    uint8_t operations[20];
    int len = approximate(x, operations);

    for (int i = 0; i < len; i++) {
        switch (operations[i]) {
            // Although C++ doesn't allow the shift operator for double,
            // it can be implemented anyway.
            // for example: use frexp() and ldexp()
            // There is an interesting question:
            // What is the performance difference between / and right_shift() ?
            case ADD_OP: {
                x_out = x_in - right_shift(y_in, i);
                y_out = right_shift(x_in, i) + y_in;
                break;
            }
            case SUB_OP: {
                x_out = x_in + right_shift(y_in, i);
                y_out = y_in - right_shift(x_in, i);
                break;
            }
            default:
                break;
        }

        if (operations[i] != NUL_OP) {
            x_in = x_out;
            y_in = y_out;
            mul *= cosine[i];
        }
    }

    struct TrigonometricValue trigonometricValue{};
    trigonometricValue.cosine = x_in * mul;
    trigonometricValue.sine = y_in * mul;
    return trigonometricValue;
}

int main() {
    TrigonometricValue trigonometricValue;
    std::cout << std::fixed << std::setprecision(10);
    std::cout << "Angle : " << 70 << std::endl;
    std::cout << "Radian: " << 70 * 3.1415926 / 180 << std::endl;
    trigonometricValue = c_sin(70 * 3.1415926 / 180);
    std::cout << "Cosine: " << trigonometricValue.cosine << std::endl;
    std::cout << "Sine  : " << trigonometricValue.sine << std::endl;

    // Compare the results with std::math
    double math_cosine = std::cos(70 * 3.1415926 / 180);
    double diff = trigonometricValue.cosine - math_cosine;
    double rate = diff / math_cosine;
    std::cout << std::fixed << std::setprecision(8);
    std::cout << "Err  : " << rate * 100 << "%" << std::endl;
    return 0;
}
