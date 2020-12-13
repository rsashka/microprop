#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#include <gtest/gtest.h>

// Open private members for tests
#define SCOPE(scope) public

#include "microprop.h"

using namespace microprop;

TEST(Microprop, Bool) {

    uint8_t buffer[17];
    Encoder enc(buffer, sizeof (buffer));
    EXPECT_EQ(sizeof (buffer), enc.GetFree());

    bool b = false;
    EXPECT_TRUE(enc.Write(1, b));
    EXPECT_EQ(2, enc.GetUsed());

    EXPECT_TRUE(enc.Write(12, true));
    EXPECT_EQ(4, enc.GetUsed());

    EXPECT_TRUE(enc.Write(123, false));
    EXPECT_EQ(6, enc.GetUsed());

    EXPECT_TRUE(enc.Write(12345, true));
    EXPECT_EQ(10, enc.GetUsed());

    EXPECT_TRUE(enc.Write(65537, false));
    EXPECT_EQ(16, enc.GetUsed());

    Decoder dec(buffer, sizeof (buffer));
    bool res = true;

    EXPECT_TRUE(dec.FieldFind(1));
    EXPECT_TRUE(dec.Read(1, res));
    EXPECT_EQ(false, res);

    EXPECT_TRUE(dec.FieldFind(12));
    EXPECT_TRUE(dec.Read(12, res));
    EXPECT_EQ(true, res);

    EXPECT_TRUE(dec.FieldFind(123));
    EXPECT_TRUE(dec.Read(123, res));
    EXPECT_EQ(false, res);

    EXPECT_TRUE(dec.FieldFind(12345));
    EXPECT_TRUE(dec.Read(12345, res));
    EXPECT_EQ(true, res);

    EXPECT_TRUE(dec.FieldFind(65537));
    EXPECT_TRUE(dec.Read(65537, res));
    EXPECT_EQ(false, res);
}

TEST(Microprop, FixedSize) {

    uint8_t buffer[500];
    Encoder enc(buffer, sizeof (buffer));

    EXPECT_EQ(sizeof (buffer), enc.GetFree());
    EXPECT_EQ(0, enc.GetUsed());

    bool b = true;
    int8_t byte = -25;
    uint16_t word = 333;
    volatile uint32_t dword = 400000;
    const uint64_t ddword = 0xFFFFFFFFFFFFFFFF - 1;
    float f = 1.123f;
    double d = 0.123;

    EXPECT_TRUE(enc.Write(1, b));
    EXPECT_EQ(2, enc.GetUsed());
    EXPECT_TRUE(enc.Write(2, 0));
    EXPECT_EQ(4, enc.GetUsed());
    EXPECT_TRUE(enc.Write(3, byte));
    EXPECT_EQ(6, enc.GetUsed());

    EXPECT_TRUE(enc.Write(30, static_cast<uint16_t> (30)));
    EXPECT_EQ(8, enc.GetUsed());
    EXPECT_TRUE(enc.Write(300, word));
    EXPECT_EQ(14, enc.GetUsed());

    EXPECT_TRUE(enc.Write(3000, 3333));
    EXPECT_EQ(20, enc.GetUsed());
    EXPECT_TRUE(enc.Write(4000, dword));
    EXPECT_EQ(28, enc.GetUsed());

    EXPECT_TRUE(enc.Write(500000, static_cast<int64_t> (4)));
    EXPECT_EQ(34, enc.GetUsed());
    EXPECT_TRUE(enc.Write(500001, ddword));
    EXPECT_EQ(48, enc.GetUsed());

    EXPECT_TRUE(enc.Write(0xF, static_cast<float> (0.123456)));
    EXPECT_EQ(54, enc.GetUsed());
    EXPECT_TRUE(enc.Write(0xFFFF, f));
    EXPECT_EQ(62, enc.GetUsed());

    EXPECT_TRUE(enc.Write(0xDD, 0.121212));
    EXPECT_EQ(73, enc.GetUsed());
    EXPECT_TRUE(enc.Write(0xDDDD, d));
    ASSERT_EQ(85, enc.GetUsed());


    Decoder dec(buffer, enc.GetUsed());

    bool read_b;
    uint8_t read_byte = 0xFF;
    uint16_t read_word;
    uint32_t read_dword;
    uint64_t read_ddword;
    float read_f;
    double read_d;

    EXPECT_TRUE(dec.Read(1, read_b));
    EXPECT_EQ(b, read_b);

    EXPECT_TRUE(dec.Read(2, read_byte));
    EXPECT_EQ(read_byte, 0);
    EXPECT_TRUE(dec.Read(3, read_ddword));
    EXPECT_EQ(-25, read_ddword);

    EXPECT_TRUE(dec.Read(30, read_word));
    EXPECT_EQ(read_word, 30);
    EXPECT_TRUE(dec.Read(300, read_word));
    EXPECT_EQ(333, read_word);

    read_byte = 1;
    EXPECT_FALSE(dec.Read(300, read_byte)); // size overflow
    EXPECT_EQ(1, read_byte);

    EXPECT_TRUE(dec.Read(3000, read_dword));
    EXPECT_EQ(3333, read_dword);
    EXPECT_TRUE(dec.Read(4000, read_dword));
    EXPECT_EQ(dword, read_dword);


    EXPECT_TRUE(dec.Read(500000, read_ddword));
    EXPECT_EQ(4, read_ddword);
    EXPECT_TRUE(dec.Read(500001, read_ddword));
    EXPECT_EQ(ddword, read_ddword);


    EXPECT_TRUE(dec.Read(0xF, read_f));
    EXPECT_FLOAT_EQ(read_f, 0.123456f);
    EXPECT_TRUE(dec.Read(0xFFFF, read_f));
    EXPECT_FLOAT_EQ(f, read_f);

    EXPECT_TRUE(dec.Read(0xDD, read_d));
    EXPECT_FLOAT_EQ(read_d, 0.121212f);
    EXPECT_TRUE(dec.Read(0xDDDD, read_d));
    EXPECT_FLOAT_EQ(d, read_d);

    EXPECT_TRUE(dec.Read(0xF, read_d)); // float -> double
    EXPECT_FLOAT_EQ(read_d, 0.123456f);
    EXPECT_TRUE(dec.Read(0xFFFF, read_d));
    EXPECT_FLOAT_EQ(f, read_d);

}

TEST(Microprop, String) {

    uint8_t buffer[30];
    Encoder enc(buffer, sizeof (buffer));
    EXPECT_EQ(sizeof (buffer), enc.GetFree());

    const char *str = "string";
    EXPECT_TRUE(enc.WriteAsString(6, str));
    EXPECT_EQ(9, enc.GetUsed());

    EXPECT_TRUE(enc.WriteAsString(static_cast<uint32_t> (1234), str));
    EXPECT_EQ(20, enc.GetUsed());

    const char *str2 = "str";
    EXPECT_TRUE(enc.WriteAsString(123456, str2));
    EXPECT_EQ(30, enc.GetUsed());


    Decoder dec(buffer, enc.GetUsed());

    size_t str_len;
    const char *res;

    EXPECT_TRUE(dec.FieldFind(6));
    res = dec.ReadAsString(6, &str_len);
    EXPECT_EQ(7, str_len);
    EXPECT_STREQ(res, str);


    EXPECT_TRUE(dec.FieldFind(1234));
    res = dec.ReadAsString(1234, &str_len);
    EXPECT_EQ(7, str_len);
    EXPECT_STREQ(res, str);

    EXPECT_TRUE(dec.FieldFind(123456));
    res = dec.ReadAsString(123456, &str_len);
    EXPECT_EQ(4, str_len);
    EXPECT_STREQ(res, str2);

    EXPECT_FALSE(dec.FieldFind(-1));
    res = dec.ReadAsString(-1, &str_len);
    EXPECT_EQ(0, str_len);
    EXPECT_EQ(res, nullptr);
}

TEST(Microprop, Blob) {

    uint8_t buffer[1000];
    Encoder enc(buffer, sizeof (buffer));
    EXPECT_EQ(sizeof (buffer), enc.GetFree());

    uint8_t b1[1] = {0};
    uint8_t b5[5] = {5, 5, 5, 5, 5};
    uint8_t b10[10] = {1, 2, 3, 4, 5, 6};
    uint8_t b20[20] = {2, 2, 2, 2, 2, 2, 2, 2, 3, 4, 5, 6};
    uint8_t large[3000];

    EXPECT_TRUE(enc.Write(1, b1, 1));
    EXPECT_EQ(4, enc.GetUsed());

    EXPECT_TRUE(enc.Write(55, b5, sizeof (b5)));
    EXPECT_EQ(12, enc.GetUsed());

    EXPECT_TRUE(enc.Write(1000, b10, sizeof (b10)));
    EXPECT_EQ(27, enc.GetUsed());

    EXPECT_TRUE(enc.Write(222222, b20, sizeof (b20)));
    EXPECT_EQ(54, enc.GetUsed());

    EXPECT_TRUE(enc.Write(33, b10, 3));
    EXPECT_EQ(60, enc.GetUsed());

    EXPECT_FALSE(enc.Write(999, large, sizeof (large)));
    EXPECT_EQ(60, enc.GetUsed());

    Decoder dec(buffer, enc.GetUsed());

    EXPECT_EQ(1, dec.Read(1, large, sizeof (b1)));
    EXPECT_TRUE(memcmp(large, b1, sizeof (b1)) == 0);

    EXPECT_EQ(5, dec.Read(55, large, sizeof (large)));
    EXPECT_TRUE(memcmp(large, b5, sizeof (b5)) == 0);

    EXPECT_EQ(10, dec.Read(1000, large, sizeof (b10)));
    EXPECT_TRUE(memcmp(large, b10, sizeof (b10)) == 0);

    EXPECT_EQ(20, dec.Read(222222, large, sizeof (large)));
    EXPECT_TRUE(memcmp(large, b20, sizeof (b20)) == 0);

    EXPECT_EQ(3, dec.Read(33, large, sizeof (large)));
    EXPECT_TRUE(memcmp(large, b10, 3) == 0);

    EXPECT_FALSE(dec.Read(999, large, sizeof (large)));
}

TEST(Microprop, Array) {

    uint8_t buffer[1000];
    Encoder enc(buffer, sizeof (buffer));
    EXPECT_EQ(sizeof (buffer), enc.GetFree());

    uint8_t a8[5] = {1, 2, 3, 4, 5};
    uint16_t a16[5] = {10, 20, 30, 40, 50};
    uint32_t a32[6] = {100, 200, 300, 400, 500, 100};
    uint64_t a64[5] = {1000, 2000, 3000, 4000, 5000000000};
    float f[5] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f};
    double d[5] = {1.11, 2.22, 3.33, 4.44, 5.55};

    EXPECT_TRUE(enc.Write(8, a8));
    EXPECT_EQ(7, enc.GetUsed());

    EXPECT_TRUE(enc.Write(16, a16));
    EXPECT_EQ(14, enc.GetUsed());

    EXPECT_TRUE(enc.Write(32, a32, 5));
    EXPECT_EQ(28, enc.GetUsed());

    EXPECT_TRUE(enc.Write(64, a64));
    EXPECT_EQ(51, enc.GetUsed());

    EXPECT_TRUE(enc.Write(0xFFFF, f));
    EXPECT_TRUE(enc.Write(0xDDDD, d));

    EXPECT_TRUE(enc.Write(1, f));
    EXPECT_TRUE(enc.Write(2, d));

    Decoder dec(buffer, enc.GetUsed());

    EXPECT_TRUE(dec.FieldFind(8));
    uint8_t a8_res[5];
    ASSERT_EQ(5, dec.Read(8, a8_res));
    EXPECT_TRUE(memcmp(a8, a8_res, sizeof (a8)) == 0);

    EXPECT_TRUE(dec.FieldFind(16));
    uint16_t a16_res[5];
    EXPECT_EQ(5, dec.Read(16, a16_res));
    EXPECT_TRUE(memcmp(a16, a16_res, sizeof (a16)) == 0);

    EXPECT_TRUE(dec.FieldFind(32));
    uint32_t a32_res[10]; // Larger than the original
    EXPECT_EQ(5, dec.Read(32, a32_res));
    EXPECT_TRUE(memcmp(a32, a32_res, sizeof (uint32_t)*5) == 0);

    EXPECT_TRUE(dec.FieldFind(64));
    uint64_t a64_res[20]; // Larger than the original
    EXPECT_EQ(5, dec.Read(64, a64_res));
    EXPECT_TRUE(memcmp(a64, a64_res, sizeof (a64)) == 0);

    EXPECT_EQ(0, dec.Read(64, a32_res)); // Large number for array type
    EXPECT_EQ(5, dec.Read(32, a64_res)); // Another, albeit suitable array type
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(a32[i], a64_res[i]);
    }

    EXPECT_TRUE(dec.FieldFind(0xFFFF));
    float fres[20]; // Larger than the original
    EXPECT_EQ(5, dec.Read(0xFFFF, fres));
    EXPECT_TRUE(memcmp(f, fres, sizeof (f)) == 0);

    EXPECT_TRUE(dec.FieldFind(0xFFFF));
    double dres[20]; // Larger than the original
    EXPECT_EQ(5, dec.Read(0xDDDD, dres));
    EXPECT_TRUE(memcmp(d, dres, sizeof (d)) == 0);

    EXPECT_EQ(5, dec.Read(0xFFFF, dres)); // Read float to double array
    for (int i = 0; i < 5; i++) {
        EXPECT_FLOAT_EQ(f[i], dres[i]);
    }
}

// Full enumeration of all possible of keys and types values

TEST(Microprop, DISABLED_StressTest) {

    uint8_t buffer[32];

    Decoder dec(buffer, sizeof (buffer));

    memset(buffer, 0, sizeof (buffer));

    int64_t read_64;
    double read_double;
    int64_t a_64[1000];
    double a_double[1000];

    for (int val = 0xFFFF; val >= 0; val--) {

        for (size_t i = 0; i<sizeof (buffer); i += 2) {
            *reinterpret_cast<uint16_t *> (&buffer[i]) = val;
        }

        std::cout << val << "\r";
        std::cout.flush();

        for (size_t key = 0; key < val; key++) {
            if(dec.FieldFind(key)) {
                dec.Read(key, read_64);
                dec.Read(key, read_double);
                dec.Read(key, a_64);
                dec.Read(key, a_double);
            }
        }
    }
}


#include <cstdio>

GTEST_API_ int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#include <googletest/googletest/src/gtest-all.cc>

#pragma GCC diagnostic pop
