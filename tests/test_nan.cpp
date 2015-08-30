#include <limits>
#include <cstdio>

int main(int , char**) {
    float qNaN = std::numeric_limits<float>::quiet_NaN();

    float neg = -qNaN;

    float sub1 = 6.0f - qNaN;
    float sub2 = qNaN - 6.0f;
    float sub3 = qNaN - qNaN;

    float add1 = 6.0f + qNaN;
    float add2 = qNaN + qNaN;

    float div1 = 6.0f / qNaN;
    float div2 = qNaN / 6.0f;
    float div3 = qNaN / qNaN;

    float mul1 = 6.0f * qNaN;
    float mul2 = qNaN * qNaN;

    printf(
        "neg: %f\nsub: %f %f %f\nadd: %f %f\ndiv: %f %f %f\nmul: %f %f\n",
        neg, sub1,sub2,sub3, add1,add2, div1,div2,div3, mul1,mul2
    );

    return 0;
}

