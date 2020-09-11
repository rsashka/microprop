#include <gtest/gtest.h>

// Open private members for tests
#define SCOPE(scope) public

#include "microprop.h"

class test_class {
};

TEST(Microprop, Types) {
    Microprop prop;

    bool b = false;
    uint8_t byte = 0;
    int8_t byte2 = 0;
    uint16_t word = 0;
    int16_t word2 = 0;
    volatile uint32_t dword = 0;
    const uint64_t ddword = 0;
    float f = 0;
    double d = 0;
    uint8_t blob[10] = {0};
    uint16_t a16[10] = {0};
    int32_t a32[10] = {0};
    uint64_t a64[10] = {0};
    float af[10] = {0};
    double ad[10] = {0};
    uint8_t *blob2 = &blob[0];
    const char *str = "Test string";

    EXPECT_EQ(Microprop::Type::Bool, Microprop::CalcDataType(b));
    EXPECT_EQ(Microprop::Type::Byte, Microprop::CalcDataType(byte));
    EXPECT_EQ(Microprop::Type::Byte, Microprop::CalcDataType(byte2));
    EXPECT_EQ(Microprop::Type::Word, Microprop::CalcDataType(word));
    EXPECT_EQ(Microprop::Type::Word, Microprop::CalcDataType(word2));
    EXPECT_EQ(Microprop::Type::DWord, Microprop::CalcDataType(dword));
    EXPECT_EQ(Microprop::Type::DDWord, Microprop::CalcDataType(ddword));
    EXPECT_EQ(Microprop::Type::Float, Microprop::CalcDataType(f));
    EXPECT_EQ(Microprop::Type::Double, Microprop::CalcDataType(d));
    EXPECT_EQ(Microprop::Type::Array16, Microprop::CalcDataType(a16));
    EXPECT_EQ(Microprop::Type::Array32, Microprop::CalcDataType(a32));
    EXPECT_EQ(Microprop::Type::Array64, Microprop::CalcDataType(a64));
    EXPECT_EQ(Microprop::Type::ArrayFloat, Microprop::CalcDataType(af));
    EXPECT_EQ(Microprop::Type::ArrayDouble, Microprop::CalcDataType(ad));
    //    test_class aclass[10];
    //    EXPECT_EQ(Microprop::Type::Error, Microprop::CalcDataType(aclass)); // Fail compile
    //    EXPECT_EQ(Microprop::Type::Error, Microprop::CalcDataType(aclass[0])); // Fail compile


    EXPECT_EQ(Microprop::Type::String, Microprop::CalcDataType(str));
    EXPECT_EQ(Microprop::Type::Blob, Microprop::CalcDataType(blob));
    ASSERT_EQ(Microprop::Type::Error, Microprop::CalcDataType(blob2)); // Fail reference

    EXPECT_EQ(1, Microprop::CalcDataSize(b));
    EXPECT_EQ(1, Microprop::CalcDataSize(byte));
    EXPECT_EQ(2, Microprop::CalcDataSize(word));
    EXPECT_EQ(4, Microprop::CalcDataSize(dword));
    EXPECT_EQ(8, Microprop::CalcDataSize(ddword));
    EXPECT_EQ(4, Microprop::CalcDataSize(f));
    EXPECT_EQ(8, Microprop::CalcDataSize(d));

    EXPECT_EQ(12, Microprop::CalcDataSize(str));
    EXPECT_EQ(10, Microprop::CalcDataSize(blob, 10));
    ASSERT_EQ(10, Microprop::CalcDataSize(blob2, 10));
    EXPECT_EQ(10, Microprop::CalcDataSize(blob));
    ASSERT_EQ(20, Microprop::CalcDataSize(a16));
    ASSERT_EQ(40, Microprop::CalcDataSize(a32));
    ASSERT_EQ(80, Microprop::CalcDataSize(a64));
    EXPECT_EQ(40, Microprop::CalcDataSize(af));
    EXPECT_EQ(80, Microprop::CalcDataSize(ad));

    EXPECT_EQ(4 + 1, Microprop::CalcFieldSize("bool", b));
    EXPECT_EQ(4 + 1, Microprop::CalcFieldSize("byte", byte));
    EXPECT_EQ(4 + 2, Microprop::CalcFieldSize("word", word));
    EXPECT_EQ(5 + 4, Microprop::CalcFieldSize("dword", dword));
    EXPECT_EQ(6 + 8, Microprop::CalcFieldSize("ddword", ddword));
    EXPECT_EQ(5 + 4, Microprop::CalcFieldSize("float", f));
    EXPECT_EQ(6 + 8, Microprop::CalcFieldSize("double", d));

    EXPECT_EQ(3 + 12 + 1, Microprop::CalcFieldSize("str", str));
    EXPECT_EQ(4 + 10 + 1, Microprop::CalcFieldSize("blob", blob, sizeof (blob)));

    EXPECT_EQ(3 + 20 + 1, Microprop::CalcFieldSize("a16", a16));
    EXPECT_EQ(3 + 40 + 1, Microprop::CalcFieldSize("a32", a32));
    EXPECT_EQ(3 + 80 + 1, Microprop::CalcFieldSize("a64", a64));
    EXPECT_EQ(2 + 40 + 1, Microprop::CalcFieldSize("af", af));
    EXPECT_EQ(2 + 80 + 1, Microprop::CalcFieldSize("ad", ad));

}

TEST(Microprop, Bool) {

    uint8_t buffer[16];
    Microprop prop(buffer, sizeof (buffer));
    EXPECT_EQ(sizeof (buffer), prop.GetSize());

    bool b = false;
    EXPECT_TRUE(prop.Append("false", b));
    EXPECT_EQ(1 + 5 + 1, prop.GetUsed());

    ASSERT_TRUE(prop.Append("true", true));
    EXPECT_EQ(7 + 1 + 4 + 1, prop.GetUsed());

    EXPECT_FALSE(prop.Append("lg", false));
    EXPECT_TRUE(prop.Append("short name", 1, b));
    EXPECT_EQ(16, prop.GetUsed());

    bool res = true;

    EXPECT_TRUE(prop.FieldExist("false"));
    EXPECT_EQ(buffer, prop.FieldExist("false"));
    EXPECT_TRUE(prop.Read("false", res));
    EXPECT_FALSE(res);


    EXPECT_TRUE(prop.FieldExist("true"));
    EXPECT_EQ(&buffer[7], prop.FieldExist("true"));
    EXPECT_TRUE(prop.Read("true", res));
    EXPECT_TRUE(res);

    EXPECT_TRUE(prop.FieldExist("s", 1));
    EXPECT_EQ(&buffer[13], prop.FieldExist("s", 1));
    EXPECT_TRUE(prop.Read("s", 1, res));
    EXPECT_FALSE(res);
}

TEST(Microprop, FixedSize) {

    uint8_t buffer[500];
    Microprop prop(buffer, sizeof (buffer));
    EXPECT_EQ(sizeof (buffer), prop.GetSize());

    bool b = true;
    uint8_t byte = rand() % 0xFF;
    uint16_t word = rand() % 0xFFFF;
    volatile uint32_t dword = rand() % 0xFFFFFFFF;
    const uint64_t ddword = rand() % 0xFFFFFFFFFFFFFFFF;
    float f = 1.123;
    double d = 0.123;

    EXPECT_TRUE(prop.Append("bool", b));
    ASSERT_EQ(6, prop.GetUsed());
    EXPECT_TRUE(prop.Append("byte1", (int8_t) 1));
    ASSERT_EQ(13, prop.GetUsed());
    EXPECT_TRUE(prop.Append("byte2", byte));
    ASSERT_EQ(20, prop.GetUsed());

    EXPECT_TRUE(prop.Append("word1", (uint16_t) 2));
    ASSERT_EQ(28, prop.GetUsed());
    EXPECT_TRUE(prop.Append("word2", word));
    ASSERT_EQ(36, prop.GetUsed());

    EXPECT_TRUE(prop.Append("dword1", 3));
    ASSERT_EQ(47, prop.GetUsed());
    EXPECT_TRUE(prop.Append("dword2", dword));
    EXPECT_EQ(58, prop.GetUsed());

    EXPECT_TRUE(prop.Append("ddword1", (int64_t) 4));
    EXPECT_EQ(74, prop.GetUsed());
    EXPECT_TRUE(prop.Append("ddword2", ddword));
    EXPECT_EQ(90, prop.GetUsed());

    EXPECT_TRUE(prop.Append("float", 5, 0.123456f));
    EXPECT_EQ(100, prop.GetUsed());
    EXPECT_TRUE(prop.Append("float2", f));
    EXPECT_EQ(111, prop.GetUsed());

    EXPECT_TRUE(prop.Append("double", 0.121212));
    EXPECT_EQ(126, prop.GetUsed());
    EXPECT_TRUE(prop.Append("double2", d));
    EXPECT_EQ(142, prop.GetUsed());


    EXPECT_TRUE(prop.Append(123456, 0.121212)); // 4 bytes in key
    EXPECT_EQ(155, prop.GetUsed());
    EXPECT_TRUE(prop.Append((uint16_t) 123456, d)); // 2 bytes in key
    EXPECT_EQ(166, prop.GetUsed());

    const uint8_t *next_ptr = nullptr;

    next_ptr = prop.GetBuffer();
    ASSERT_EQ(next_ptr, prop.m_data);
    next_ptr = prop.FieldNext(next_ptr);
    ASSERT_EQ(next_ptr, &prop.m_data[6]);
    next_ptr = prop.FieldNext(next_ptr);
    ASSERT_EQ(next_ptr, &prop.m_data[13]);
    next_ptr = prop.FieldNext(next_ptr);
    ASSERT_EQ(next_ptr, &prop.m_data[20]);
    next_ptr = prop.FieldNext(next_ptr);
    ASSERT_EQ(next_ptr, &prop.m_data[28]);
    next_ptr = prop.FieldNext(next_ptr);
    ASSERT_EQ(next_ptr, &prop.m_data[36]);
    next_ptr = prop.FieldNext(next_ptr);
    ASSERT_EQ(next_ptr, &prop.m_data[47]);
    next_ptr = prop.FieldNext(next_ptr);
    ASSERT_EQ(next_ptr, &prop.m_data[58]);
    next_ptr = prop.FieldNext(next_ptr);
    ASSERT_EQ(next_ptr, &prop.m_data[74]);
    next_ptr = prop.FieldNext(next_ptr);
    ASSERT_EQ(next_ptr, &prop.m_data[90]);
    next_ptr = prop.FieldNext(next_ptr);
    ASSERT_EQ(next_ptr, &prop.m_data[100]);
    next_ptr = prop.FieldNext(next_ptr);
    ASSERT_EQ(next_ptr, &prop.m_data[111]);
    next_ptr = prop.FieldNext(next_ptr);
    ASSERT_EQ(next_ptr, &prop.m_data[126]);
    next_ptr = prop.FieldNext(next_ptr);
    ASSERT_EQ(next_ptr, &prop.m_data[142]);
    next_ptr = prop.FieldNext(next_ptr);
    ASSERT_EQ(next_ptr, &prop.m_data[155]);
    next_ptr = prop.FieldNext(next_ptr);
    ASSERT_EQ(next_ptr, &prop.m_data[166]);
    next_ptr = prop.FieldNext(next_ptr);
    ASSERT_EQ(next_ptr, nullptr);


    bool read_b;
    uint8_t read_byte;
    uint16_t read_word;
    uint32_t read_dword;
    uint64_t read_ddword;
    float read_f;
    double read_d;

    EXPECT_TRUE(prop.Read("bool", read_b));
    EXPECT_EQ(b, read_b);

    EXPECT_TRUE(prop.Read("byte1", read_byte));
    ASSERT_EQ(Microprop::Type::Byte, Microprop::FieldType(prop.FieldExist("byte1")));
    EXPECT_EQ(read_byte, 1);
    EXPECT_TRUE(prop.Read("byte1", read_ddword));
    EXPECT_EQ(read_ddword, 1);
    EXPECT_TRUE(prop.Read("byte2", read_byte));
    EXPECT_EQ(Microprop::Type::Byte, Microprop::FieldType(prop.FieldExist("byte2")));
    EXPECT_EQ(read_byte, byte);

    EXPECT_TRUE(prop.Read("word1", 5, read_word));
    ASSERT_EQ(Microprop::Type::Word, Microprop::FieldType(prop.FieldExist("word1")));
    EXPECT_EQ(read_word, 2);
    EXPECT_TRUE(prop.Read("word1", read_ddword));
    EXPECT_EQ(read_ddword, 2);
    EXPECT_TRUE(prop.Read("word2", read_word));
    EXPECT_EQ(read_word, word);

    EXPECT_TRUE(prop.Read("dword1", 6, read_dword));
    ASSERT_EQ(Microprop::Type::DWord, Microprop::FieldType(prop.FieldExist("dword1")));
    EXPECT_EQ(read_dword, 3);
    EXPECT_TRUE(prop.Read("dword1", read_ddword));
    EXPECT_EQ(read_ddword, 3);
    EXPECT_TRUE(prop.Read("dword2", read_dword));
    EXPECT_EQ(read_dword, dword);

    EXPECT_TRUE(prop.Read("ddword1", read_ddword));
    ASSERT_EQ(Microprop::Type::DDWord, Microprop::FieldType(prop.FieldExist("ddword1")));
    EXPECT_EQ(read_ddword, 4);
    EXPECT_TRUE(prop.Read("ddword2", read_ddword));
    EXPECT_EQ(read_ddword, ddword);

    EXPECT_FALSE(prop.Read("ddword1_bad", read_ddword));
    EXPECT_EQ(read_ddword, ddword);
    EXPECT_FALSE(prop.Read("ddword2_bad", read_ddword));
    EXPECT_EQ(read_ddword, ddword);


    EXPECT_TRUE(prop.Read("float", read_f));
    ASSERT_EQ(Microprop::Type::Float, Microprop::FieldType(prop.FieldExist("float")));
    EXPECT_FLOAT_EQ(read_f, 0.123456f);
    EXPECT_TRUE(prop.Read("float2", read_f));
    EXPECT_FLOAT_EQ(read_f, f);

    EXPECT_TRUE(prop.Read("double", read_d));
    ASSERT_EQ(Microprop::Type::Double, Microprop::FieldType(prop.FieldExist("double")));
    EXPECT_DOUBLE_EQ(read_d, 0.121212);
    EXPECT_TRUE(prop.Read("double2", read_d));
    EXPECT_DOUBLE_EQ(read_d, d);


    EXPECT_TRUE(prop.Read(123456, read_d)); // 4 bytes in key
    EXPECT_DOUBLE_EQ(read_d, 0.121212);
    //    EXPECT_TRUE(prop.Read((uint16_t) 123456, read_d)); // 2 bytes in key
    //    EXPECT_DOUBLE_EQ(read_d, d);

}

TEST(Microprop, String) {

    uint8_t buffer[33];
    Microprop prop(buffer, sizeof (buffer));
    EXPECT_EQ(sizeof (buffer), prop.GetSize());

    const char *str = "string";
    EXPECT_TRUE(prop.AppendAsString("str1", str));
    EXPECT_EQ(6 + 4 + 1 + 2, prop.GetUsed());

    EXPECT_TRUE(prop.AppendAsString((uint32_t) 1234, str));
    EXPECT_EQ(2 * (6 + 4 + 1 + 2), prop.GetUsed());
    EXPECT_EQ(26, prop.GetUsed());

    const char *str2 = "str";
    EXPECT_FALSE(prop.AppendAsString("lg", str2));
    EXPECT_TRUE(prop.AppendAsString("short name", 1, str2));
    EXPECT_EQ(33, prop.GetUsed());


    size_t str_len;
    const char *res;

    EXPECT_TRUE(prop.FieldExist("str1"));
    EXPECT_EQ(buffer, prop.FieldExist("str1"));
    res = prop.ReadAsString("str1", &str_len);
    EXPECT_EQ(6, str_len);
    EXPECT_STREQ(res, str);


    EXPECT_TRUE(prop.FieldExist((uint32_t) 1234));
    EXPECT_EQ(&buffer[13], prop.FieldExist((uint32_t) 1234));
    res = prop.ReadAsString((uint32_t) 1234, &str_len);
    EXPECT_EQ(6, str_len);
    EXPECT_STREQ(res, str);

    EXPECT_TRUE(prop.FieldExist("ssss", 1));
    EXPECT_EQ(&buffer[26], prop.FieldExist("ssss", 1));
    res = prop.ReadAsString("s", 1, &str_len);
    EXPECT_EQ(3, str_len);
    EXPECT_STREQ(res, str2);
}

TEST(Microprop, Blob) {

    uint8_t buffer[1000];
    Microprop prop(buffer, sizeof (buffer));
    EXPECT_EQ(sizeof (buffer), prop.GetSize());

    uint8_t b5[5] = {5, 5, 5, 5, 5};
    uint8_t b10[10] = {1, 2, 3, 4, 5, 6};
    uint8_t b20[20] = {2, 2, 2, 2, 2, 2, 2, 2, 3, 4, 5, 6};
    uint8_t large[300];

    EXPECT_EQ(5, prop.Append("b5", b5, sizeof (b5)));
    EXPECT_EQ(9, prop.GetUsed());

    EXPECT_EQ(10, prop.Append((int16_t) 10, b10, sizeof (b10)));
    EXPECT_EQ(23, prop.GetUsed());

    EXPECT_EQ(20, prop.Append("b20", 3, b20, sizeof (b20)));
    EXPECT_EQ(48, prop.GetUsed());

    EXPECT_EQ(0, prop.Append("large", large, sizeof (large)));
    EXPECT_EQ(48, prop.GetUsed());

    EXPECT_EQ(5, prop.Read("b5", large, sizeof (large)));
    EXPECT_TRUE(memcmp(large, b5, sizeof (b5)) == 0);

    EXPECT_EQ(10, prop.Read((int16_t) 10, large, sizeof (b10)));
    EXPECT_TRUE(memcmp(large, b10, sizeof (b10)) == 0);

    EXPECT_EQ(20, prop.Read("b20", 3, large, sizeof (large)));
    EXPECT_TRUE(memcmp(large, b20, sizeof (b20)) == 0);

    EXPECT_EQ(0, prop.Read("fail", large, sizeof (large)));
}

TEST(Microprop, Array) {

    uint8_t buffer[1000];
    Microprop prop(buffer, sizeof (buffer));
    EXPECT_EQ(sizeof (buffer), prop.GetSize());

    uint8_t a8[5] = {1, 2, 3, 4, 5};
    uint16_t a16[5] = {10, 20, 30, 40, 50};
    uint32_t a32[5] = {100, 200, 300, 400, 500};
    uint64_t a64[5] = {1000, 2000, 3000, 4000, 5000};
    float f[5] = {1.1, 2.2, 3.3, 4.4, 5.5};
    double d[5] = {1.11, 2.22, 3.33, 4.44, 5.55};

    EXPECT_TRUE(prop.Append("a8", a8));
    EXPECT_EQ(5 + 2 + 1 + 1, prop.GetUsed());

    EXPECT_TRUE(prop.Append("a16", 3, a16));
    EXPECT_EQ(9 + 10 + 3 + 1 + 1, prop.GetUsed());

    EXPECT_TRUE(prop.Append("a32", a32));
    EXPECT_EQ(24 + 20 + 3 + 1 + 1, prop.GetUsed());

    EXPECT_TRUE(prop.Append("a64", a64));
    EXPECT_EQ(49 + 40 + 3 + 1 + 1, prop.GetUsed());

    EXPECT_TRUE(prop.Append("a8_2", a8, sizeof (a8)));
    EXPECT_EQ(94 + 5 + 4 + 1 + 1, prop.GetUsed());

    EXPECT_TRUE(prop.Append("f", f));
    EXPECT_TRUE(prop.Append("d", d));

    EXPECT_TRUE(prop.Append(123456, f));
    EXPECT_TRUE(prop.Append((uint16_t) 100, d));

    EXPECT_TRUE(prop.FieldExist("a8", 2));
    EXPECT_EQ(&buffer[0], prop.FieldExist("a8", 2));
    uint8_t a8_res[5];
    EXPECT_EQ(5, prop.Read("a8", 2, a8_res));
    EXPECT_TRUE(memcmp(a8, a8_res, sizeof (a8)) == 0);

    EXPECT_TRUE(prop.FieldExist("a16"));
    uint16_t a16_res[5];
    EXPECT_EQ(10, prop.Read("a16", a16_res));
    EXPECT_TRUE(memcmp(a16, a16_res, sizeof (a16)) == 0);

    EXPECT_TRUE(prop.FieldExist("a32"));
    uint32_t a32_res[10]; // Большем, чем исходный
    EXPECT_EQ(20, prop.Read("a32", a32_res));
    EXPECT_TRUE(memcmp(a32, a32_res, sizeof (a32)) == 0);

    EXPECT_EQ(0, prop.Read("a64", 3, a32_res)); // Другой тип массива

    EXPECT_TRUE(prop.FieldExist("a64"));
    uint64_t a64_res[20]; // Большем, чем исходный
    EXPECT_EQ(40, prop.Read("a64", 3, a64_res));
    EXPECT_TRUE(memcmp(a64, a64_res, sizeof (a64)) == 0);

    EXPECT_TRUE(prop.FieldExist("f"));
    float fres[20]; // Большем, чем исходный
    EXPECT_EQ(20, prop.Read("f", fres));
    EXPECT_TRUE(memcmp(f, fres, sizeof (f)) == 0);

    EXPECT_TRUE(prop.FieldExist("d"));
    double dres[20]; // Большем, чем исходный
    EXPECT_EQ(40, prop.Read("d", dres));
    EXPECT_TRUE(memcmp(d, dres, sizeof (d)) == 0);

    EXPECT_TRUE(prop.FieldExist(123456));
    EXPECT_EQ(20, prop.Read(123456, fres));
    EXPECT_TRUE(memcmp(f, fres, sizeof (f)) == 0);

    EXPECT_TRUE(prop.FieldExist((uint16_t) 100));
    EXPECT_EQ(40, prop.Read((uint16_t) 100, dres));
    EXPECT_TRUE(memcmp(d, dres, sizeof (d)) == 0);
}

TEST(Microprop, ReadOnly) {

    Microprop prop1;
    EXPECT_FALSE(prop1.IsReadOnly());

    uint8_t buffer[100];
    Microprop prop2(buffer, sizeof (buffer));
    EXPECT_FALSE(prop2.IsReadOnly());

    Microprop prop_ro((const uint8_t *) buffer, sizeof (buffer));
    EXPECT_TRUE(prop_ro.IsReadOnly());

    EXPECT_FALSE(prop1.Append("bool", true));
    EXPECT_FALSE(prop1.Append("byte", (int8_t) 25));
    EXPECT_FALSE(prop1.Append("dword", 123456));

    EXPECT_TRUE(prop2.Append("bool", true));
    EXPECT_TRUE(prop2.Append("byte", (int8_t) 25));
    EXPECT_TRUE(prop2.Append("dword", 123456));

    EXPECT_FALSE(prop_ro.Append("bool2", true));
    EXPECT_FALSE(prop_ro.Append("byte2", (int8_t) 25));
    EXPECT_FALSE(prop_ro.Append("dword2", 123456));

    EXPECT_TRUE(prop_ro.FieldExist("bool"));
    EXPECT_TRUE(prop_ro.FieldExist("byte"));
    EXPECT_TRUE(prop_ro.FieldExist("dword"));
    EXPECT_FALSE(prop_ro.FieldExist("error"));

    prop1.AssignBuffer((const uint8_t *) buffer, sizeof (buffer));
    EXPECT_TRUE(prop1.IsReadOnly());
    EXPECT_FALSE(prop1.Append("bool2", true));
    EXPECT_FALSE(prop1.Append("byte2", (int8_t) 25));
    EXPECT_FALSE(prop1.Append("dword2", 123456));

    EXPECT_TRUE(prop1.FieldExist("bool"));
    EXPECT_TRUE(prop1.FieldExist("byte"));
    EXPECT_TRUE(prop1.FieldExist("dword"));
    EXPECT_FALSE(prop1.FieldExist("error"));

    prop1.AssignBuffer(buffer, sizeof (buffer));
    EXPECT_FALSE(prop1.IsReadOnly());
    EXPECT_TRUE(prop1.Append("bool2", true));
    EXPECT_TRUE(prop1.Append("byte2", (int8_t) 25));
    EXPECT_TRUE(prop1.Append("dword2", 123456));
}

#include <cstdio>

GTEST_API_ int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#include <gtest/googletest/src/gtest-all.cc>
