#include <gtest/gtest.h>

// Open private members for tests
#define SCOPE(scope) public

#include "microprop.h"

using namespace microprop;

class test_class {
};

//TEST(Microprop, Types) {
//    Microprop prop;
//
//    bool b = false;
//    uint8_t byte = 0;
//    int8_t byte2 = 0;
//    uint16_t word = 0;
//    int16_t word2 = 0;
//    volatile uint32_t dword = 0;
//    const uint64_t ddword = 0;
//    float f = 0;
//    double d = 0;
//    uint8_t blob[10] = {0};
//    uint16_t a16[10] = {0};
//    int32_t a32[10] = {0};
//    uint64_t a64[10] = {0};
//    float af[10] = {0};
//    double ad[10] = {0};
//    uint8_t *blob2 = &blob[0];
//    const char *str = "Test string";
//
//    EXPECT_EQ(Microprop::Type::Bool, Microprop::CalcDataType(b));
//    EXPECT_EQ(Microprop::Type::Byte, Microprop::CalcDataType(byte));
//    EXPECT_EQ(Microprop::Type::Byte, Microprop::CalcDataType(byte2));
//    EXPECT_EQ(Microprop::Type::Word, Microprop::CalcDataType(word));
//    EXPECT_EQ(Microprop::Type::Word, Microprop::CalcDataType(word2));
//    EXPECT_EQ(Microprop::Type::DWord, Microprop::CalcDataType(dword));
//    EXPECT_EQ(Microprop::Type::DDWord, Microprop::CalcDataType(ddword));
//    EXPECT_EQ(Microprop::Type::Float, Microprop::CalcDataType(f));
//    EXPECT_EQ(Microprop::Type::Double, Microprop::CalcDataType(d));
//    EXPECT_EQ(Microprop::Type::Array16, Microprop::CalcDataType(a16));
//    EXPECT_EQ(Microprop::Type::Array32, Microprop::CalcDataType(a32));
//    EXPECT_EQ(Microprop::Type::Array64, Microprop::CalcDataType(a64));
//    EXPECT_EQ(Microprop::Type::ArrayFloat, Microprop::CalcDataType(af));
//    EXPECT_EQ(Microprop::Type::ArrayDouble, Microprop::CalcDataType(ad));
//    //    test_class aclass[10];
//    //    EXPECT_EQ(Microprop::Type::Error, Microprop::CalcDataType(aclass)); // Fail compile
//    //    EXPECT_EQ(Microprop::Type::Error, Microprop::CalcDataType(aclass[0])); // Fail compile
//
//
//    EXPECT_EQ(Microprop::Type::String, Microprop::CalcDataType(str));
//    EXPECT_EQ(Microprop::Type::Blob, Microprop::CalcDataType(blob));
//    ASSERT_EQ(Microprop::Type::Error, Microprop::CalcDataType(blob2)); // Fail reference
//
//    EXPECT_EQ(1, Microprop::CalcDataSize(b));
//    EXPECT_EQ(1, Microprop::CalcDataSize(byte));
//    EXPECT_EQ(2, Microprop::CalcDataSize(word));
//    EXPECT_EQ(4, Microprop::CalcDataSize(dword));
//    EXPECT_EQ(8, Microprop::CalcDataSize(ddword));
//    EXPECT_EQ(4, Microprop::CalcDataSize(f));
//    EXPECT_EQ(8, Microprop::CalcDataSize(d));
//
//    EXPECT_EQ(12, Microprop::CalcDataSize(str));
//    EXPECT_EQ(10, Microprop::CalcDataSize(blob, 10));
//    ASSERT_EQ(10, Microprop::CalcDataSize(blob2, 10));
//    EXPECT_EQ(10, Microprop::CalcDataSize(blob));
//    ASSERT_EQ(20, Microprop::CalcDataSize(a16));
//    ASSERT_EQ(40, Microprop::CalcDataSize(a32));
//    ASSERT_EQ(80, Microprop::CalcDataSize(a64));
//    EXPECT_EQ(40, Microprop::CalcDataSize(af));
//    EXPECT_EQ(80, Microprop::CalcDataSize(ad));
//
//}

TEST(Microprop, Bool) {

    uint8_t buffer[17];
    Encoder enc(buffer, sizeof (buffer));
    EXPECT_EQ(sizeof (buffer), enc.GetSize());

    bool b = false;
    EXPECT_TRUE(enc.Write(1, b));
    EXPECT_EQ(2, enc.GetUsed());

    EXPECT_TRUE(enc.Write(12, true));
    EXPECT_EQ(4, enc.GetUsed());

    EXPECT_TRUE(enc.Write(123, false));
    EXPECT_EQ(7, enc.GetUsed());

    EXPECT_TRUE(enc.Write(12345, true));
    EXPECT_EQ(11, enc.GetUsed());

    EXPECT_TRUE(enc.Write(65537, false));
    EXPECT_EQ(17, enc.GetUsed());



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

    EXPECT_EQ(sizeof (buffer), enc.GetSize());
    EXPECT_EQ(0, enc.GetUsed());

    bool b = true;
    uint8_t byte = 25;
    uint16_t word = 333;
    volatile uint32_t dword = 400000;
    const uint64_t ddword = 0xFFFFFFFFFFFFFFFF;
    float f = 1.123;
    double d = 0.123;

    EXPECT_TRUE(enc.Write(1, b));
    ASSERT_EQ(2, enc.GetUsed());
    EXPECT_TRUE(enc.Write(2, 0));
    ASSERT_EQ(4, enc.GetUsed());
    EXPECT_TRUE(enc.Write(3, byte));
    ASSERT_EQ(7, enc.GetUsed());

    EXPECT_TRUE(enc.Write(30, (uint16_t) 30));
    EXPECT_EQ(11, enc.GetUsed());
    EXPECT_TRUE(enc.Write(300, word));
    EXPECT_EQ(17, enc.GetUsed());

    EXPECT_TRUE(enc.Write(3000, 3333));
    EXPECT_EQ(23, enc.GetUsed());
    EXPECT_TRUE(enc.Write(4000, dword));
    EXPECT_EQ(31, enc.GetUsed());

    EXPECT_TRUE(enc.Write(500000, (int64_t) 4));
    EXPECT_EQ(37, enc.GetUsed());
    EXPECT_TRUE(enc.Write(500001, ddword));
    EXPECT_EQ(51, enc.GetUsed());

    //    EXPECT_TRUE(prop.Append("float", 5, 0.123456f));
    //    EXPECT_EQ(100, prop.GetUsed());
    //    EXPECT_TRUE(prop.Append("float2", f));
    //    EXPECT_EQ(111, prop.GetUsed());
    //
    //    EXPECT_TRUE(prop.Append("double", 0.121212));
    //    EXPECT_EQ(126, prop.GetUsed());
    //    EXPECT_TRUE(prop.Append("double2", d));
    //    EXPECT_EQ(142, prop.GetUsed());
    //
    //
    //    EXPECT_TRUE(prop.Append(123456, 0.121212)); // 4 bytes in key
    //    EXPECT_EQ(155, prop.GetUsed());
    //    EXPECT_TRUE(prop.Append((uint16_t) 123456, d)); // 2 bytes in key
    //    EXPECT_EQ(166, prop.GetUsed());
    //
    //    const uint8_t *next_ptr = nullptr;
    //
    //    next_ptr = prop.GetBuffer();
    //    ASSERT_EQ(next_ptr, prop.m_data);
    //    next_ptr = prop.FieldNext(next_ptr);
    //    ASSERT_EQ(next_ptr, &prop.m_data[6]);
    //    next_ptr = prop.FieldNext(next_ptr);
    //    ASSERT_EQ(next_ptr, &prop.m_data[13]);
    //    next_ptr = prop.FieldNext(next_ptr);
    //    ASSERT_EQ(next_ptr, &prop.m_data[20]);
    //    next_ptr = prop.FieldNext(next_ptr);
    //    ASSERT_EQ(next_ptr, &prop.m_data[28]);
    //    next_ptr = prop.FieldNext(next_ptr);
    //    ASSERT_EQ(next_ptr, &prop.m_data[36]);
    //    next_ptr = prop.FieldNext(next_ptr);
    //    ASSERT_EQ(next_ptr, &prop.m_data[47]);
    //    next_ptr = prop.FieldNext(next_ptr);
    //    ASSERT_EQ(next_ptr, &prop.m_data[58]);
    //    next_ptr = prop.FieldNext(next_ptr);
    //    ASSERT_EQ(next_ptr, &prop.m_data[74]);
    //    next_ptr = prop.FieldNext(next_ptr);
    //    ASSERT_EQ(next_ptr, &prop.m_data[90]);
    //    next_ptr = prop.FieldNext(next_ptr);
    //    ASSERT_EQ(next_ptr, &prop.m_data[100]);
    //    next_ptr = prop.FieldNext(next_ptr);
    //    ASSERT_EQ(next_ptr, &prop.m_data[111]);
    //    next_ptr = prop.FieldNext(next_ptr);
    //    ASSERT_EQ(next_ptr, &prop.m_data[126]);
    //    next_ptr = prop.FieldNext(next_ptr);
    //    ASSERT_EQ(next_ptr, &prop.m_data[142]);
    //    next_ptr = prop.FieldNext(next_ptr);
    //    ASSERT_EQ(next_ptr, &prop.m_data[155]);
    //    next_ptr = prop.FieldNext(next_ptr);
    //    ASSERT_EQ(next_ptr, &prop.m_data[166]);
    //    next_ptr = prop.FieldNext(next_ptr);
    //    ASSERT_EQ(next_ptr, nullptr);
    //

    Decoder dec(buffer, sizeof (buffer));

    bool read_b;
    uint8_t read_byte = 0xFF;
    uint16_t read_word;
    uint32_t read_dword;
    uint64_t read_ddword;
    //    float read_f;
    //    double read_d;

    EXPECT_TRUE(dec.Read(1, read_b));
    EXPECT_EQ(b, read_b);

    EXPECT_TRUE(dec.Read(2, read_byte));
    EXPECT_EQ(read_byte, 0);
    EXPECT_TRUE(dec.Read(3, read_ddword));
    EXPECT_EQ(read_ddword, 25);

    EXPECT_TRUE(dec.Read(30, read_word));
    EXPECT_EQ(read_word, 30);
    EXPECT_TRUE(dec.Read(300, read_word));
    EXPECT_EQ(read_word, 333);

    read_byte = 1;
    EXPECT_FALSE(dec.Read(300, read_byte)); // size overflow
    EXPECT_EQ(read_byte, 1);

    EXPECT_TRUE(dec.Read(3000, read_dword));
    EXPECT_EQ(3333, read_dword);
    EXPECT_TRUE(dec.Read(4000, read_dword));
    EXPECT_EQ(dword, read_dword);


    EXPECT_TRUE(dec.Read(500000, read_ddword));
    EXPECT_EQ(4, read_ddword);
    EXPECT_TRUE(dec.Read(500001, read_ddword));
    EXPECT_EQ(ddword, read_ddword);


    //    EXPECT_TRUE(prop.Read("word1", 5, read_word));
    //    ASSERT_EQ(Microprop::Type::Word, Microprop::FieldType(prop.FieldExist("word1")));
    //    EXPECT_EQ(read_word, 2);
    //    EXPECT_TRUE(prop.Read("word1", read_ddword));
    //    EXPECT_EQ(read_ddword, 2);
    //    EXPECT_TRUE(prop.Read("word2", read_word));
    //    EXPECT_EQ(read_word, word);
    //
    //    EXPECT_TRUE(prop.Read("dword1", 6, read_dword));
    //    ASSERT_EQ(Microprop::Type::DWord, Microprop::FieldType(prop.FieldExist("dword1")));
    //    EXPECT_EQ(read_dword, 3);
    //    EXPECT_TRUE(prop.Read("dword1", read_ddword));
    //    EXPECT_EQ(read_ddword, 3);
    //    EXPECT_TRUE(prop.Read("dword2", read_dword));
    //    EXPECT_EQ(read_dword, dword);
    //
    //    EXPECT_TRUE(prop.Read("ddword1", read_ddword));
    //    ASSERT_EQ(Microprop::Type::DDWord, Microprop::FieldType(prop.FieldExist("ddword1")));
    //    EXPECT_EQ(read_ddword, 4);
    //    EXPECT_TRUE(prop.Read("ddword2", read_ddword));
    //    EXPECT_EQ(read_ddword, ddword);
    //
    //    EXPECT_FALSE(prop.Read("ddword1_bad", read_ddword));
    //    EXPECT_EQ(read_ddword, ddword);
    //    EXPECT_FALSE(prop.Read("ddword2_bad", read_ddword));
    //    EXPECT_EQ(read_ddword, ddword);
    //
    //
    //    EXPECT_TRUE(prop.Read("float", read_f));
    //    ASSERT_EQ(Microprop::Type::Float, Microprop::FieldType(prop.FieldExist("float")));
    //    EXPECT_FLOAT_EQ(read_f, 0.123456f);
    //    EXPECT_TRUE(prop.Read("float2", read_f));
    //    EXPECT_FLOAT_EQ(read_f, f);
    //
    //    EXPECT_TRUE(prop.Read("double", read_d));
    //    ASSERT_EQ(Microprop::Type::Double, Microprop::FieldType(prop.FieldExist("double")));
    //    EXPECT_DOUBLE_EQ(read_d, 0.121212);
    //    EXPECT_TRUE(prop.Read("double2", read_d));
    //    EXPECT_DOUBLE_EQ(read_d, d);
    //
    //
    //    EXPECT_TRUE(prop.Read(123456, read_d)); // 4 bytes in key
    //    EXPECT_DOUBLE_EQ(read_d, 0.121212);

}

TEST(Microprop, String) {

    uint8_t buffer[30];
    Encoder enc(buffer, sizeof (buffer));
    EXPECT_EQ(sizeof (buffer), enc.GetSize());

    const char *str = "string";
    EXPECT_TRUE(enc.WriteAsString(0, str));
    EXPECT_EQ(9, enc.GetUsed());

    EXPECT_TRUE(enc.WriteAsString((uint32_t) 1234, str));
    EXPECT_EQ(20, enc.GetUsed());

    const char *str2 = "str";
    EXPECT_TRUE(enc.WriteAsString(123456, str2));
    EXPECT_EQ(30, enc.GetUsed());


    Decoder dec(buffer, enc.GetUsed());

    size_t str_len;
    const char *res;

    EXPECT_TRUE(dec.FieldFind(0));
    res = dec.ReadAsString(0, &str_len);
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
    EXPECT_EQ(sizeof (buffer), enc.GetSize());

    uint8_t b1[1] = {0};
    uint8_t b5[5] = {5, 5, 5, 5, 5};
    uint8_t b10[10] = {1, 2, 3, 4, 5, 6};
    uint8_t b20[20] = {2, 2, 2, 2, 2, 2, 2, 2, 3, 4, 5, 6};
    uint8_t large[3000];

    EXPECT_TRUE(enc.Write(1, b1, 1));
    EXPECT_EQ(3, enc.GetUsed());

    EXPECT_TRUE(enc.Write(55, b5, sizeof (b5)));
    EXPECT_EQ(11, enc.GetUsed());

    EXPECT_TRUE(enc.Write(1000, b10, sizeof (b10)));
    EXPECT_EQ(25, enc.GetUsed());

    EXPECT_TRUE(enc.Write(222222, b20, sizeof (b20)));
    EXPECT_EQ(51, enc.GetUsed());

    EXPECT_FALSE(enc.Write(1, large, sizeof (large)));
    EXPECT_EQ(51, enc.GetUsed());

    Decoder dec(buffer, enc.GetUsed());

    EXPECT_TRUE(dec.Read(1, large, sizeof (b1)));
    EXPECT_TRUE(memcmp(large, b1, sizeof (b1)) == 0);

    EXPECT_TRUE(dec.Read(55, large, sizeof (large)));
    EXPECT_TRUE(memcmp(large, b5, sizeof (b5)) == 0);

    EXPECT_TRUE(dec.Read(1000, large, sizeof (b10)));
    EXPECT_TRUE(memcmp(large, b10, sizeof (b10)) == 0);

    EXPECT_TRUE(dec.Read(222222, large, sizeof (large)));
    EXPECT_TRUE(memcmp(large, b20, sizeof (b20)) == 0);

    EXPECT_FALSE(dec.Read(0, large, sizeof (large)));
    EXPECT_FALSE(dec.Read(99999999, large, sizeof (large)));
}

//TEST(Microprop, Array) {
//
//    uint8_t buffer[1000];
//    Microprop prop(buffer, sizeof (buffer));
//    EXPECT_EQ(sizeof (buffer), prop.GetSize());
//
//    uint8_t a8[5] = {1, 2, 3, 4, 5};
//    uint16_t a16[5] = {10, 20, 30, 40, 50};
//    uint32_t a32[5] = {100, 200, 300, 400, 500};
//    uint64_t a64[5] = {1000, 2000, 3000, 4000, 5000};
//    float f[5] = {1.1, 2.2, 3.3, 4.4, 5.5};
//    double d[5] = {1.11, 2.22, 3.33, 4.44, 5.55};
//
//    EXPECT_TRUE(prop.Append("a8", a8));
//    EXPECT_EQ(5 + 2 + 1 + 1, prop.GetUsed());
//
//    EXPECT_TRUE(prop.Append("a16", 3, a16));
//    EXPECT_EQ(9 + 10 + 3 + 1 + 1, prop.GetUsed());
//
//    EXPECT_TRUE(prop.Append("a32", a32));
//    EXPECT_EQ(24 + 20 + 3 + 1 + 1, prop.GetUsed());
//
//    EXPECT_TRUE(prop.Append("a64", a64));
//    EXPECT_EQ(49 + 40 + 3 + 1 + 1, prop.GetUsed());
//
//    EXPECT_TRUE(prop.Append("a8_2", a8, sizeof (a8)));
//    EXPECT_EQ(94 + 5 + 4 + 1 + 1, prop.GetUsed());
//
//    EXPECT_TRUE(prop.Append("f", f));
//    EXPECT_TRUE(prop.Append("d", d));
//
//    EXPECT_TRUE(prop.Append(123456, f));
//    EXPECT_TRUE(prop.Append((uint16_t) 100, d));
//
//    EXPECT_TRUE(prop.FieldExist("a8", 2));
//    EXPECT_EQ(&buffer[0], prop.FieldExist("a8", 2));
//    uint8_t a8_res[5];
//    EXPECT_EQ(5, prop.Read("a8", 2, a8_res));
//    EXPECT_TRUE(memcmp(a8, a8_res, sizeof (a8)) == 0);
//
//    EXPECT_TRUE(prop.FieldExist("a16"));
//    uint16_t a16_res[5];
//    EXPECT_EQ(10, prop.Read("a16", a16_res));
//    EXPECT_TRUE(memcmp(a16, a16_res, sizeof (a16)) == 0);
//
//    EXPECT_TRUE(prop.FieldExist("a32"));
//    uint32_t a32_res[10]; // Большем, чем исходный
//    EXPECT_EQ(20, prop.Read("a32", a32_res));
//    EXPECT_TRUE(memcmp(a32, a32_res, sizeof (a32)) == 0);
//
//    EXPECT_EQ(0, prop.Read("a64", 3, a32_res)); // Другой тип массива
//
//    EXPECT_TRUE(prop.FieldExist("a64"));
//    uint64_t a64_res[20]; // Большем, чем исходный
//    EXPECT_EQ(40, prop.Read("a64", 3, a64_res));
//    EXPECT_TRUE(memcmp(a64, a64_res, sizeof (a64)) == 0);
//
//    EXPECT_TRUE(prop.FieldExist("f"));
//    float fres[20]; // Большем, чем исходный
//    EXPECT_EQ(20, prop.Read("f", fres));
//    EXPECT_TRUE(memcmp(f, fres, sizeof (f)) == 0);
//
//    EXPECT_TRUE(prop.FieldExist("d"));
//    double dres[20]; // Большем, чем исходный
//    EXPECT_EQ(40, prop.Read("d", dres));
//    EXPECT_TRUE(memcmp(d, dres, sizeof (d)) == 0);
//
//    EXPECT_TRUE(prop.FieldExist(123456));
//    EXPECT_EQ(20, prop.Read(123456, fres));
//    EXPECT_TRUE(memcmp(f, fres, sizeof (f)) == 0);
//
//    EXPECT_TRUE(prop.FieldExist((uint16_t) 100));
//    EXPECT_EQ(40, prop.Read((uint16_t) 100, dres));
//    EXPECT_TRUE(memcmp(d, dres, sizeof (d)) == 0);
//}

#include <cstdio>

GTEST_API_ int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#include <gtest/googletest/src/gtest-all.cc>
