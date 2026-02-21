#include <gtest/gtest.h>
#include <cstdint>
#include <random>
#include <climits>
#include <chrono>
#include "../include/data_lab.hpp"

class BaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        gen = std::mt19937(static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    }
    int32_t rand_range(int32_t min, int32_t max) { return std::uniform_int_distribution<int32_t>(min, max)(gen); }
    int32_t rand_full() { return rand_range(INT32_MIN, INT32_MAX); }
    int32_t rand_safe() { return rand_range(INT32_MIN/2, INT32_MAX/2); }
    int32_t rand_pos() { return rand_range(1, INT32_MAX); }
    int32_t rand_neg() { return rand_range(INT32_MIN, -1); }
    int32_t rand_nonzero() { int32_t v; do { v = rand_full(); } while(v == 0); return v; }
    int32_t ref_add(int32_t a, int32_t b) { return (int32_t)((uint32_t)a + (uint32_t)b); }
    int32_t ref_sub(int32_t a, int32_t b) { return (int32_t)((uint32_t)a - (uint32_t)b); }
    int32_t ref_mul(int32_t a, int32_t b) { return (int32_t)((uint32_t)a * (uint32_t)b); }
    std::mt19937 gen;
};

class AddTest : public BaseTest {};
class SubtractTest : public BaseTest {};
class MultiplyTest : public BaseTest {};
class DivideTest : public BaseTest {};
class ModuloTest : public BaseTest {};

TEST_F(AddTest, Basic) {
    EXPECT_EQ(data_lab::add(5, 3), 8);
    EXPECT_EQ(data_lab::add(-5, -3), -8);
    EXPECT_EQ(data_lab::add(10, -3), 7);
    EXPECT_EQ(data_lab::add(0, 0), 0);
    EXPECT_EQ(data_lab::add(0x55555555, 0xAAAAAAAA), -1);
    EXPECT_EQ(data_lab::add(0xFFFFFFFF, 1), 0);
}
TEST_F(AddTest, Boundaries) {
    EXPECT_EQ(data_lab::add(INT32_MAX, 0), INT32_MAX);
    EXPECT_EQ(data_lab::add(INT32_MIN, 0), INT32_MIN);
    EXPECT_EQ(data_lab::add(INT32_MAX - 1, 1), INT32_MAX);
    EXPECT_EQ(data_lab::add(INT32_MIN + 1, -1), INT32_MIN);
}
TEST_F(AddTest, Flow) {
    EXPECT_EQ(data_lab::add(INT32_MAX, 1), INT32_MIN);
    EXPECT_EQ(data_lab::add(INT32_MIN, -1), INT32_MAX);
    EXPECT_EQ(data_lab::add(2000000000, 2000000000), -294967296);
    EXPECT_EQ(data_lab::add(INT32_MIN, INT32_MIN), 0);
    for (int i = 0; i < 100; ++i) {
        int32_t a = rand_range(INT32_MAX - 1000, INT32_MAX);
        int32_t b = rand_range(1001, 10000);
        EXPECT_EQ(data_lab::add(a, b), ref_add(a, b));
    }
    for (int i = 0; i < 100; ++i) {
        int32_t a = rand_range(INT32_MIN, INT32_MIN + 1000);
        int32_t b = rand_range(-10000, -1001);
        EXPECT_EQ(data_lab::add(a, b), ref_add(a, b));
    }
}
TEST_F(AddTest, Properties) {
    for (int i = 0; i < 100; ++i) {
        int32_t a = rand_full(), b = rand_full();
        EXPECT_EQ(data_lab::add(a, b), ref_add(a, b));
        EXPECT_EQ(data_lab::add(a, b), data_lab::add(b, a));
        int32_t x = rand_safe()/3, y = rand_safe()/3, z = rand_safe()/3;
        EXPECT_EQ(data_lab::add(data_lab::add(x, y), z), data_lab::add(x, data_lab::add(y, z)));
    }
}

TEST_F(SubtractTest, Basic) {
    EXPECT_EQ(data_lab::subtract(10, 3), 7);
    EXPECT_EQ(data_lab::subtract(-10, -3), -7);
    EXPECT_EQ(data_lab::subtract(0, 5), -5);
    EXPECT_EQ(data_lab::subtract(5, 0), 5);
}
TEST_F(SubtractTest, Boundaries) {
    EXPECT_EQ(data_lab::subtract(INT32_MAX, 0), INT32_MAX);
    EXPECT_EQ(data_lab::subtract(INT32_MIN, 0), INT32_MIN);
    EXPECT_EQ(data_lab::subtract(0, INT32_MIN), INT32_MIN); 
    EXPECT_EQ(data_lab::subtract(INT32_MAX, 1), INT32_MAX - 1);
    EXPECT_EQ(data_lab::subtract(INT32_MIN, -1), INT32_MIN + 1);
}
TEST_F(SubtractTest, Flow) {
    EXPECT_EQ(data_lab::subtract(INT32_MIN, 1), INT32_MAX); 
    EXPECT_EQ(data_lab::subtract(INT32_MAX, -1), INT32_MIN);
    EXPECT_EQ(data_lab::subtract(INT32_MAX, INT32_MAX), 0);
    for (int i = 0; i < 100; ++i) {
        int32_t a = rand_range(INT32_MIN, INT32_MIN + 1000);
        int32_t b = rand_range(1, 1000); 
        EXPECT_EQ(data_lab::subtract(a, b), ref_sub(a, b));
    }
    for (int i = 0; i < 100; ++i) {
        int32_t a = rand_range(INT32_MAX - 1000, INT32_MAX);
        int32_t b = rand_range(-1000, -1);
        EXPECT_EQ(data_lab::subtract(a, b), ref_sub(a, b));
    }
}
TEST_F(SubtractTest, Relations) {
    for (int i = 0; i < 100; ++i) {
        int32_t a = rand_full(), b = rand_full();
        EXPECT_EQ(data_lab::subtract(a, b), ref_sub(a, b));
        EXPECT_EQ(data_lab::subtract(a, b), data_lab::add(a, -b));
    }
}

TEST_F(MultiplyTest, Basic) {
    EXPECT_EQ(data_lab::multiply(0, 0), 0);
    EXPECT_EQ(data_lab::multiply(5, 3), 15);
    EXPECT_EQ(data_lab::multiply(-5, -3), 15);
    EXPECT_EQ(data_lab::multiply(5, -3), -15);
    EXPECT_EQ(data_lab::multiply(7, 1), 7);
    EXPECT_EQ(data_lab::multiply(7, -1), -7);
}
TEST_F(MultiplyTest, Boundaries) {
    EXPECT_EQ(data_lab::multiply(INT32_MAX, 0), 0);
    EXPECT_EQ(data_lab::multiply(INT32_MIN, 0), 0);
    EXPECT_EQ(data_lab::multiply(INT32_MAX, 1), INT32_MAX);
    EXPECT_EQ(data_lab::multiply(INT32_MIN, 1), INT32_MIN);
}
TEST_F(MultiplyTest, Flow) {
    EXPECT_EQ(data_lab::multiply(INT32_MAX, 2), -2);
    EXPECT_EQ(data_lab::multiply(INT32_MIN, -1), INT32_MIN); 
    EXPECT_EQ(data_lab::multiply(INT32_MAX, INT32_MAX), 1);
    EXPECT_EQ(data_lab::multiply(65536, 65536), 0);
    EXPECT_EQ(data_lab::multiply(INT32_MAX, -1), INT32_MIN + 1);
    for (int i = 0; i < 50; ++i) {
        int32_t a = rand_range(100000, 200000);
        int32_t b = rand_range(100000, 200000);
        EXPECT_EQ(data_lab::multiply(a, b), ref_mul(a, b));
    }
    for (int i = 0; i < 50; ++i) {
        int32_t a = rand_range(-200000, -100000);
        int32_t b = rand_range(-200000, -100000);
        EXPECT_EQ(data_lab::multiply(a, b), ref_mul(a, b));
    }
}
TEST_F(MultiplyTest, Properties) {
    for (int i = 0; i < 100; ++i) {
        int32_t a = rand_range(-1000, 1000), b = rand_range(-1000, 1000), c = rand_range(-1000, 1000);
        EXPECT_EQ(data_lab::multiply(a, b), data_lab::multiply(b, a));
        EXPECT_EQ(data_lab::multiply(a, 1), a);
        EXPECT_EQ(data_lab::multiply(a, 0), 0);
        EXPECT_EQ(data_lab::multiply(a, data_lab::add(b, c)), 
                  data_lab::add(data_lab::multiply(a, b), data_lab::multiply(a, c)));
        int32_t x = rand_full(), y = rand_full();
        EXPECT_EQ(data_lab::multiply(x, y), ref_mul(x, y));
    }
}

TEST_F(DivideTest, Basic) {
    EXPECT_EQ(data_lab::divide(0, 5), 0);
    EXPECT_EQ(data_lab::divide(15, 3), 5);
    EXPECT_EQ(data_lab::divide(-15, -3), 5);
    EXPECT_EQ(data_lab::divide(-15, 3), -5);
    EXPECT_EQ(data_lab::divide(7, -2), -3); 
    EXPECT_EQ(data_lab::divide(-7, 2), -3);
    EXPECT_EQ(data_lab::divide(1, 2), 0);
}
TEST_F(DivideTest, Boundaries) {
    EXPECT_EQ(data_lab::divide(INT32_MAX, 1), INT32_MAX);
    EXPECT_EQ(data_lab::divide(INT32_MAX, -1), -INT32_MAX);
    EXPECT_EQ(data_lab::divide(INT32_MIN, 1), INT32_MIN);
    EXPECT_EQ(data_lab::divide(INT32_MIN, INT32_MAX), -1);
    EXPECT_EQ(data_lab::divide(INT32_MAX, INT32_MIN), 0);
    EXPECT_EQ(data_lab::divide(INT32_MIN, INT32_MIN), 1);
    EXPECT_EQ(data_lab::divide(INT32_MAX, INT32_MAX), 1);
}
TEST_F(DivideTest, Flow) {
    EXPECT_EQ(data_lab::divide(INT32_MIN, -1), INT32_MIN);
    EXPECT_EQ(data_lab::divide(INT32_MAX, 2), 1073741823);
    EXPECT_EQ(data_lab::divide(INT32_MIN, 2), -1073741824);
    for (int i = 0; i < 100; ++i) {
        int32_t a = INT32_MIN;
        int32_t b = rand_nonzero();
        if (b == -1) continue;
        EXPECT_EQ(data_lab::divide(a, b), a / b);
    }
}
TEST_F(DivideTest, Relation) {
    for (int i = 0; i < 100; ++i) {
        int32_t a = rand_full(), b = rand_nonzero();
        if (a == INT32_MIN && b == -1) continue; 
        EXPECT_EQ(data_lab::divide(a, b), a / b);
        int32_t q = data_lab::divide(a, b), r = data_lab::modulo(a, b);
        EXPECT_EQ(data_lab::add(data_lab::multiply(q, b), r), a);
    }
}

TEST_F(ModuloTest, Basic) {
    EXPECT_EQ(data_lab::modulo(0, 5), 0);
    EXPECT_EQ(data_lab::modulo(17, 5), 2);
    EXPECT_EQ(data_lab::modulo(-17, 5), -2);
    EXPECT_EQ(data_lab::modulo(17, -5), 2);
    EXPECT_EQ(data_lab::modulo(-17, -5), -2);
}
TEST_F(ModuloTest, Boundaries) {
    EXPECT_EQ(data_lab::modulo(INT32_MAX, 1), 0);
    EXPECT_EQ(data_lab::modulo(INT32_MIN, 1), 0);
    EXPECT_EQ(data_lab::modulo(INT32_MAX, -1), 0);
    EXPECT_EQ(data_lab::modulo(INT32_MIN, -1), 0);
    EXPECT_EQ(data_lab::modulo(INT32_MIN, INT32_MAX), -1);
    EXPECT_EQ(data_lab::modulo(INT32_MAX, INT32_MIN), INT32_MAX);
    EXPECT_EQ(data_lab::modulo(INT32_MIN, INT32_MIN), 0);
}
TEST_F(ModuloTest, Flow) {
    EXPECT_EQ(data_lab::modulo(INT32_MIN, -1), 0);
    EXPECT_EQ(data_lab::modulo(INT32_MAX, INT32_MAX), 0);
    EXPECT_EQ(data_lab::modulo(INT32_MIN, INT32_MIN), 0);
    EXPECT_EQ(data_lab::modulo(INT32_MIN, INT32_MAX), -1);
    EXPECT_EQ(data_lab::modulo(INT32_MAX, INT32_MIN), INT32_MAX);
    for (int i = 0; i < 100; ++i) {
        int32_t a = INT32_MIN;
        int32_t b = rand_nonzero();
        if (b == -1) continue;
        EXPECT_EQ(data_lab::modulo(a, b), a % b);
    }
}
TEST_F(ModuloTest, Consistency) {
    for (int i = 0; i < 100; ++i) {
        int32_t a = rand_full(), b = rand_nonzero();
        if (a == INT32_MIN && b == -1) continue;
        EXPECT_EQ(data_lab::modulo(a, b), a % b);
        int32_t r = data_lab::modulo(a, b);
        EXPECT_TRUE(abs(r) < abs(b) || r == 0);
        if (r != 0) EXPECT_TRUE((a > 0) == (r > 0));
    }
}