/***************************************************************************************************

  Zyan Core Library (Zycore-C)

  Original Author : Joel Hoener

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.

***************************************************************************************************/

/**
 * @file
 * @brief   Tests the the arg parse implementation.
 */

#include <string_view>

#include <gtest/gtest.h>
#include <Zycore/ArgParse.h>
#include <Zycore/LibC.h>

/* ============================================================================================== */
/* Helpers                                                                                        */
/* ============================================================================================== */

auto cvt_string_view(const ZyanStringView *sv)
{
    const char* buf;
    if (ZYAN_FAILED(ZyanStringViewGetData(sv, &buf))) throw std::exception{};
    ZyanUSize len;
    if (ZYAN_FAILED(ZyanStringViewGetSize(sv, &len))) throw std::exception{};

    return std::string_view{buf, len};
}

/* ============================================================================================== */
/* Tests                                                                                          */
/* ============================================================================================== */

/* ---------------------------------------------------------------------------------------------- */
/* Unnamed args                                                                                   */
/* ---------------------------------------------------------------------------------------------- */

static auto UnnamedArgTest(ZyanU64 min, ZyanU64 max)
{
    const char* argv[] = {
        "test", "a", "xxx"
    };

    ZyanArgParseConfig cfg = {
        argv,   // argv
        3,      // argc
        min,    // min_unnamed_args
        max,    // max_unnamed_args
        nullptr // args
    };

    ZyanVector parsed;
    ZYAN_MEMSET(&parsed, 0, sizeof(parsed));
    auto status = ZyanArgParse(&cfg, &parsed);
    return std::make_tuple(status, parsed);
}

TEST(UnnamedArgs, TooFew)
{
    auto [status, parsed] = UnnamedArgTest(5, 5);
    ASSERT_FALSE(ZYAN_SUCCESS(status));
}

TEST(UnnamedArgs, TooMany)
{
    auto [status, parsed] = UnnamedArgTest(1, 1);
    ASSERT_FALSE(ZYAN_SUCCESS(status));
}

TEST(UnnamedArgs, PerfectFit)
{
    auto [status, parsed] = UnnamedArgTest(2, 2);
    ASSERT_TRUE(ZYAN_SUCCESS(status));

    ZyanUSize size;
    ASSERT_TRUE(ZYAN_SUCCESS(ZyanVectorGetSize(&parsed, &size)));
    ASSERT_EQ(size, 2);

    auto arg = (const ZyanArgParseArg*)ZyanVectorGet(&parsed, 0);
    ASSERT_NE(arg, nullptr);
    ASSERT_TRUE(arg->has_value);
    ASSERT_STREQ((const char*)arg->value.string.vector.data /* hax! */, "a");

    arg = (const ZyanArgParseArg*)ZyanVectorGet(&parsed, 1);
    ASSERT_NE(arg, nullptr);
    ASSERT_TRUE(arg->has_value);
    ASSERT_STREQ((const char*)arg->value.string.vector.data /* hax! */, "xxx");
}

/* ---------------------------------------------------------------------------------------------- */
/* Dash args                                                                                      */
/* ---------------------------------------------------------------------------------------------- */

TEST(DashArg, MixedBoolAndValueArgs)
{
    const char* argv[] = {
        "test", "-aio42", "-n", "xxx"
    };

    ZyanArgParseDefinition args[] = {
        {"-o", ZYAN_FALSE},
        {"-a", ZYAN_TRUE},
        {"-n", ZYAN_FALSE},
        {"-i", ZYAN_TRUE},
        {nullptr, ZYAN_FALSE}
    };

    ZyanArgParseConfig cfg = {
        argv, // argv
        4,    // argc
        0,    // min_unnamed_args
        0,    // max_unnamed_args
        args  // args
    };

    ZyanVector parsed;
    ZYAN_MEMSET(&parsed, 0, sizeof(parsed));
    auto status = ZyanArgParse(&cfg, &parsed);
    ASSERT_TRUE(ZYAN_SUCCESS(status));

    ZyanUSize size;
    ASSERT_TRUE(ZYAN_SUCCESS(ZyanVectorGetSize(&parsed, &size)));
    ASSERT_EQ(size, 4);

    const ZyanArgParseArg* arg;
    ASSERT_TRUE(ZYAN_SUCCESS(ZyanVectorGetPointer(&parsed, 0, (const void**)&arg)));
    ASSERT_STREQ(arg->arg->name, "-a");
    ASSERT_FALSE(arg->has_value);

    ASSERT_TRUE(ZYAN_SUCCESS(ZyanVectorGetPointer(&parsed, 1, (const void**)&arg)));
    ASSERT_STREQ(arg->arg->name, "-i");
    ASSERT_FALSE(arg->has_value);

    ASSERT_TRUE(ZYAN_SUCCESS(ZyanVectorGetPointer(&parsed, 2, (const void**)&arg)));
    ASSERT_STREQ(arg->arg->name, "-o");
    ASSERT_TRUE(arg->has_value);
    ASSERT_EQ(cvt_string_view(&arg->value), "42");

    ASSERT_TRUE(ZYAN_SUCCESS(ZyanVectorGetPointer(&parsed, 3, (const void**)&arg)));
    ASSERT_STREQ(arg->arg->name, "-n");
    ASSERT_TRUE(arg->has_value);
    ASSERT_EQ(cvt_string_view(&arg->value), "xxx");
}

/* ---------------------------------------------------------------------------------------------- */
/* Double dash args                                                                               */
/* ---------------------------------------------------------------------------------------------- */

TEST(DoubleDashArg, PerfectFit)
{
    const char* argv[] = {
        "test", "--help", "--stuff", "1337"
    };

    ZyanArgParseDefinition args[] = {
        {"--help", ZYAN_TRUE},
        {"--stuff", ZYAN_FALSE},
        {nullptr, ZYAN_FALSE}
    };

    ZyanArgParseConfig cfg = {
        argv, // argv
        4,    // argc
        0,    // min_unnamed_args
        0,    // max_unnamed_args
        args  // args
    };

    ZyanVector parsed;
    ZYAN_MEMSET(&parsed, 0, sizeof(parsed));
    auto status = ZyanArgParse(&cfg, &parsed);
    ASSERT_TRUE(ZYAN_SUCCESS(status));

    ZyanUSize size;
    ASSERT_TRUE(ZYAN_SUCCESS(ZyanVectorGetSize(&parsed, &size)));
    ASSERT_EQ(size, 2);

    const ZyanArgParseArg* arg;
    ASSERT_TRUE(ZYAN_SUCCESS(ZyanVectorGetPointer(&parsed, 0, (const void**)&arg)));
    ASSERT_STREQ(arg->arg->name, "--help");
    ASSERT_FALSE(arg->has_value);

    ASSERT_TRUE(ZYAN_SUCCESS(ZyanVectorGetPointer(&parsed, 1, (const void**)&arg)));
    ASSERT_STREQ(arg->arg->name, "--stuff");
    ASSERT_TRUE(arg->has_value);
    ASSERT_EQ(cvt_string_view(&arg->value), "1337");
}

/* ---------------------------------------------------------------------------------------------- */
/* Mixed                                                                                          */
/* ---------------------------------------------------------------------------------------------- */

TEST(MixedArgs, Stuff)
{
    const char* argv[] = {
        "test", "--feature-xyz", "-n5", "blah.c", "woof.moo"
    };

    ZyanArgParseDefinition args[] = {
        {"--feature-xyz", ZYAN_TRUE},
        {"-n", ZYAN_FALSE},
        {nullptr, ZYAN_FALSE}
    };

    ZyanArgParseConfig cfg = {
        argv, // argv
        5,    // argc4
        0,    // min_unnamed_args
        100,  // max_unnamed_args
        args  // args
    };

    ZyanVector parsed;
    ZYAN_MEMSET(&parsed, 0, sizeof(parsed));
    auto status = ZyanArgParse(&cfg, &parsed);
    ASSERT_TRUE(ZYAN_SUCCESS(status));

    ZyanUSize size;
    ASSERT_TRUE(ZYAN_SUCCESS(ZyanVectorGetSize(&parsed, &size)));
    ASSERT_EQ(size, 4);

    const ZyanArgParseArg* arg;
    ASSERT_TRUE(ZYAN_SUCCESS(ZyanVectorGetPointer(&parsed, 0, (const void**)&arg)));
    ASSERT_STREQ(arg->arg->name, "--feature-xyz");
    ASSERT_FALSE(arg->has_value);

    ASSERT_TRUE(ZYAN_SUCCESS(ZyanVectorGetPointer(&parsed, 1, (const void**)&arg)));
    ASSERT_STREQ(arg->arg->name, "-n");
    ASSERT_TRUE(arg->has_value);
    ASSERT_EQ(cvt_string_view(&arg->value), "5");

    ASSERT_TRUE(ZYAN_SUCCESS(ZyanVectorGetPointer(&parsed, 2, (const void**)&arg)));
    ASSERT_EQ(arg->arg, nullptr);
    ASSERT_TRUE(arg->has_value);
    ASSERT_EQ(cvt_string_view(&arg->value), "blah.c");

    ASSERT_TRUE(ZYAN_SUCCESS(ZyanVectorGetPointer(&parsed, 3, (const void**)&arg)));
    ASSERT_EQ(arg->arg, nullptr);
    ASSERT_TRUE(arg->has_value);
    ASSERT_EQ(cvt_string_view(&arg->value), "woof.moo");
}

/* ============================================================================================== */
/* Entry point                                                                                    */
/* ============================================================================================== */

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

/* ============================================================================================== */