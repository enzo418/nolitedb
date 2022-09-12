#include "gtest/gtest.h"
#include "yaml-cpp/exceptions.h"
#include "yaml-cpp/yaml.h"  // IWYU pragma: keep

#define EXPECT_THROW_EXCEPTION(exception_type, statement, message) \
    ASSERT_THROW(statement, exception_type);                       \
    try {                                                          \
        statement;                                                 \
    } catch (const exception_type& e) {                            \
        EXPECT_EQ(e.msg, message);                                 \
    }

#define EXPECT_THROW_EXCEPTION_KEY(exception_type, statement, message, skey) \
    ASSERT_THROW(statement, exception_type);                                 \
    try {                                                                    \
        statement;                                                           \
    } catch (const exception_type& e) {                                      \
        EXPECT_EQ(e.msg, message);                                           \
        EXPECT_EQ(e.key, skey);                                              \
    }

#define EXPECT_THROW_MARK(statement, epos, eline, ecolumn) \
    try {                                                  \
        statement;                                         \
    } catch (const YAML::Exception& ex) {                  \
        ASSERT_EQ(ex.mark.pos, epos);                      \
        ASSERT_EQ(ex.mark.line, eline);                    \
        ASSERT_EQ(ex.mark.column, ecolumn);                \
    }

#define EXPECT_THROW_KEY(statement, skey)  \
    try {                                  \
        statement;                         \
    } catch (const YAML::KeyNotFound& e) { \
        EXPECT_EQ(e.key, skey);            \
    }

namespace YAML {
    namespace {

        TEST(ErrorMessageTest, BadSubscriptErrorMessage) {
            const char* example_yaml =
                "first:\n"
                "   second: 1\n"
                "   third: 2\n";

            Node doc = Load(example_yaml);

            // Test that printable key is part of error message
            EXPECT_THROW_EXCEPTION(
                YAML::BadSubscript, doc["first"]["second"]["fourth"],
                "operator[] call on a scalar (key: \"fourth\")");

            EXPECT_THROW_EXCEPTION(YAML::BadSubscript,
                                   doc["first"]["second"][37],
                                   "operator[] call on a scalar (key: \"37\")");

            // Non-printable key is not included in error message
            EXPECT_THROW_EXCEPTION(YAML::BadSubscript,
                                   doc["first"]["second"][std::vector<int>()],
                                   "operator[] call on a scalar");

            EXPECT_THROW_EXCEPTION(YAML::BadSubscript,
                                   doc["first"]["second"][Node()],
                                   "operator[] call on a scalar");
        }

        TEST(ErrorMessageTest, Ex9_1_TypedBadConversionErrorMessage) {
            const char* example_yaml =
                "first:\n"
                "   second: 1\n"
                "   third: 2\n";

            const Node doc = Load(example_yaml);

            // Non-printable key is not included in error message
            EXPECT_THROW_EXCEPTION(YAML::TypedBadConversion<int>,
                                   doc["first"].as<int>(), "bad conversion");

            // mark should be right
            try {
                doc["first"].as<int>();
            } catch (YAML::TypedBadConversion<int> ex) {
                ASSERT_EQ(ex.mark.pos, 10);  // 10 = parsed is before "second"
                ASSERT_EQ(ex.mark.line, 1);
                ASSERT_EQ(ex.mark.column, 3);  // 3rd space in 2nd line
            }
        }

        TEST(ErrorMessageTest, Ex9_2_KeyNotFoundErrorMessage) {
            const char* example_yaml =
                "first:\n"
                "   second: 1\n"
                "   third: 2\n"
                "main:\n"
                "   main2:\n"
                "      one: 1";

            const Node doc = Load(example_yaml);

            // Test that printable key is part of error message
            EXPECT_THROW_EXCEPTION_KEY(YAML::KeyNotFound,
                                       doc["first"]["fourth"],
                                       "key not found: \"fourth\"", "fourth");

            EXPECT_THROW_EXCEPTION_KEY(YAML::KeyNotFound,
                                       doc["first"][37].as<int>(),
                                       "key not found: \"37\"", "37");

            EXPECT_THROW_EXCEPTION_KEY(
                YAML::KeyNotFound, doc["first"][std::vector<int>()].as<int>(),
                "invalid node; this may result from using a map "
                "iterator as a sequence iterator, or vice-versa",
                "");
        }

        TEST(ErrorMessageTest, Ex9_3_KeyNotFoundMarks) {
            const char* example_yaml =
                "first:\n"
                "   second: 1\n"
                "   third: 2\n"
                "main:\n"
                "   main2:\n"
                "      one: 1";

            const Node doc = Load(example_yaml);

            // check marks on "first"
            EXPECT_THROW_MARK(doc["first"]["fourth"], 10, 1, 3);

            // mark should be (0,0,0) since doc has no "main2"
            EXPECT_THROW_MARK(doc["main2"]["two"], 0, 0, 0);

            // check marks on "main2" from "main"
            EXPECT_THROW_MARK(doc["main"]["main2"]["two"], 54, 5, 6);
        }
    }  // namespace
}  // namespace YAML
