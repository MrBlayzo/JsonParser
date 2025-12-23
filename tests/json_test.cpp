#include <gtest/gtest.h>
#include "../src/json.h"

using namespace std;

class TestableJsonReader : public JsonReader
{
public:
    using JsonReader::_read;
    using JsonReader::_write;
};

class JsonTest : public ::testing::Test
{
protected:
    TestableJsonReader parser;
    void SetUp() override
    {
        // Выполняется ПЕРЕД каждым тестом
    }

    void TearDown() override
    {
    }
};

TEST_F(JsonTest, ReadRawEmptyFile)
{

    EXPECT_EQ(parser._read("res/TestEmpty.json"), "");
}

TEST_F(JsonTest, ReadRawEmptyList)
{
    EXPECT_EQ(parser._read("res/TestEmptyList.json"), "[]");
}

TEST_F(JsonTest, ReadRawEmptyDict)
{
    EXPECT_EQ(parser._read("res/TestEmptyDict.json"), "{}");
}

TEST_F(JsonTest, ReadRawNotExistFile)
{
    EXPECT_THROW(parser._read("res/gsdfgsd.json"), FileNotFoundError);
}

TEST_F(JsonTest, WriteRaw)
{
    const string output =
        R"({
    "args":["1", "2", "3"],
    "kwargs":{"1":1, "2": 2.1, "3":null}
})";

    EXPECT_NO_THROW(parser._write("res/WriteTest.json", output));
}

TEST_F(JsonTest, ReadEmptyList)
{
    JsonValue output =
        JsonValue(vector<JsonValue>());
    EXPECT_EQ(parser.read("res/TestEmptyList.json"), output);
}

TEST_F(JsonTest, Read)
{
    JsonValue output =
        JsonValue(
            {{"args", JsonValue({"1", "2", "3", "4"})},
             {"rwargs", JsonValue({{"1", JsonValue(1)},
                                   {"2", JsonValue(1.2)},
                                   {"3", JsonValue()},
                                   {"4", JsonValue("test_string")},
                                   {"5", JsonValue(false)}})}});
    EXPECT_EQ(parser.read("res/Test1.json"), output);
}

TEST_F(JsonTest, Read2)
{
    JsonValue output =
        JsonValue(
            {{"message", JsonValue("Hello, \"World\"!")},
             {"nested", JsonValue(
                            {
                                {"a", JsonValue(1)},
                                {"b", JsonValue(
                                          vector<JsonValue>{JsonValue(true), JsonValue()})},
                            })}});
    EXPECT_EQ(parser.read("res/Test2.json"), output);
}

TEST_F(JsonTest, ReadError)
{
    JsonValue output = JsonValue();
    EXPECT_THROW(parser.read("res/TestError.json"), JsonParseError);
}

TEST_F(JsonTest, Write)
{
    JsonValue output =
        JsonValue(
            {{"args", JsonValue({"1", "2", "3", "4"})},
             {"rwargs", JsonValue({{"1", JsonValue(1)},
                                   {"2", JsonValue(1.2)},
                                   {"3", JsonValue()},
                                   {"4", JsonValue("test_string")},
                                   {"5", JsonValue(false)}})}});
    EXPECT_NO_THROW(parser.write("res/TestWriteReal.json", output));

    string reference = parser._read("res/Test1.json");
    string writed = parser._read("res/TestWriteReal.json");
    EXPECT_EQ(reference, writed);
}

TEST_F(JsonTest, Read3)
{
    JsonValue output =
        JsonValue(
            {
                map<string, JsonValue>{{"\"World\"", JsonValue("Hello, \"World\"!")}},
            });
    EXPECT_EQ(parser.read("res/Test3.json"), output);
}

TEST_F(JsonTest, ReadError2)
{
    EXPECT_THROW(parser.read("res/TestError2.json"), JsonParseError);
}

TEST_F(JsonTest, ReadError3)
{
    EXPECT_THROW(parser.read("res/TestError3.json"), JsonParseError);
}

TEST_F(JsonTest, ReadError4)
{
    EXPECT_THROW(parser.read("res/TestError4.json"), JsonParseError);
}

TEST_F(JsonTest, ReadUnicode)
{
    JsonValue output =
        JsonValue(
            {
                map<string, JsonValue>{{"1", JsonValue("$")}},
            });
    EXPECT_EQ(parser.read("res/TestUnicode.json"), output);
}

TEST_F(JsonTest, ReadErrorPlusNumber)
{
    EXPECT_THROW(parser.read("res/TestErrorNumber1.json"), JsonParseError);
}

TEST_F(JsonTest, ReadErrorLeadingZero)
{
    EXPECT_THROW(parser.read("res/TestErrorNumber2.json"), JsonParseError);
}

TEST_F(JsonTest, ReadUnicodeCyrillic)
{
    JsonValue expected("П");
    JsonValue actual = parser.read("res/TestUnicode1.json");
    EXPECT_EQ(actual, expected);  // ← упадёт: actual = "\x1F", expected = "\xD0\x9F"
}

TEST_F(JsonTest, ReadUnicodeEmoji)
{
    JsonValue expected("❤");
    JsonValue actual = parser.read("res/TestUnicode2.json");
    EXPECT_EQ(actual, expected);  // ← упадёт
}