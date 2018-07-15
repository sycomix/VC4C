/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#include "TestIntegerFunctions.h"
#include "../src/helper.h"
#include "emulation_helper.h"

#include <bitset>
#include <utility>

static const std::string UNARY_FUNCTION = R"(
__kernel void test(__global OUT* out, __global IN* in) {
  size_t gid = get_global_id(0);
  out[gid] = FUNC(in[gid]);
}
)";

static const std::string BINARY_FUNCTION = R"(
__kernel void test(__global OUT* out, __global IN0* in0, __global IN1* in1) {
  size_t gid = get_global_id(0);
  out[gid] = FUNC(in0[gid], in1[gid]);
}
)";

static const std::string TERNARY_FUNCTION = R"(
__kernel void test(__global OUT* out, __global IN0* in0, __global IN1* in1, __global IN2* in2) {
  size_t gid = get_global_id(0);
  out[gid] = FUNC(in0[gid], in1[gid], in2[gid]);
}
)";

TestIntegerFunctions::TestIntegerFunctions(const vc4c::Configuration& config) : config(config)
{
    TEST_ADD(TestIntegerFunctions::testAbsSignedInt);
    TEST_ADD(TestIntegerFunctions::testAbsSignedShort);
    TEST_ADD(TestIntegerFunctions::testAbsSignedChar);
    TEST_ADD(TestIntegerFunctions::testAbsUnsignedInt);
    TEST_ADD(TestIntegerFunctions::testAbsUnsignedShort);
    TEST_ADD(TestIntegerFunctions::testAbsUnsignedChar);

    TEST_ADD(TestIntegerFunctions::testAbsDiffSignedInt);
    TEST_ADD(TestIntegerFunctions::testAbsDiffSignedShort);
    TEST_ADD(TestIntegerFunctions::testAbsDiffSignedChar);
    TEST_ADD(TestIntegerFunctions::testAbsDiffUnsignedInt);
    TEST_ADD(TestIntegerFunctions::testAbsDiffUnsignedShort);
    TEST_ADD(TestIntegerFunctions::testAbsDiffUnsignedChar);

    TEST_ADD(TestIntegerFunctions::testAddSatSignedInt);
    TEST_ADD(TestIntegerFunctions::testAddSatSignedShort);
    TEST_ADD(TestIntegerFunctions::testAddSatSignedChar);
    TEST_ADD(TestIntegerFunctions::testAddSatUnsignedInt);
    TEST_ADD(TestIntegerFunctions::testAddSatUnsignedShort);
    TEST_ADD(TestIntegerFunctions::testAddSatUnsignedChar);

    TEST_ADD(TestIntegerFunctions::testHAddSignedInt);
    TEST_ADD(TestIntegerFunctions::testHAddSignedShort);
    TEST_ADD(TestIntegerFunctions::testHAddSignedChar);
    TEST_ADD(TestIntegerFunctions::testHAddUnsignedInt);
    TEST_ADD(TestIntegerFunctions::testHAddUnsignedShort);
    TEST_ADD(TestIntegerFunctions::testHAddUnsignedChar);

    TEST_ADD(TestIntegerFunctions::testRHAddSignedInt);
    TEST_ADD(TestIntegerFunctions::testRHAddSignedShort);
    TEST_ADD(TestIntegerFunctions::testRHAddSignedChar);
    TEST_ADD(TestIntegerFunctions::testRHAddUnsignedInt);
    TEST_ADD(TestIntegerFunctions::testRHAddUnsignedShort);
    TEST_ADD(TestIntegerFunctions::testRHAddUnsignedChar);

    /* XXX
        TEST_ADD(TestIntegerFunctions::testClampSignedInt);
        TEST_ADD(TestIntegerFunctions::testClampSignedShort);
        TEST_ADD(TestIntegerFunctions::testClampSignedChar);
        TEST_ADD(TestIntegerFunctions::testClampUnsignedInt);
        TEST_ADD(TestIntegerFunctions::testClampUnsignedShort);
        TEST_ADD(TestIntegerFunctions::testClampUnsignedChar);
    */

    TEST_ADD(TestIntegerFunctions::testClzSignedInt);
    TEST_ADD(TestIntegerFunctions::testClzSignedShort);
    TEST_ADD(TestIntegerFunctions::testClzSignedChar);
    TEST_ADD(TestIntegerFunctions::testClzUnsignedInt);
    TEST_ADD(TestIntegerFunctions::testClzUnsignedShort);
    TEST_ADD(TestIntegerFunctions::testClzUnsignedChar);

    /* XXX
        TEST_ADD(TestIntegerFunctions::testMadHiSignedInt);
        TEST_ADD(TestIntegerFunctions::testMadHiSignedShort);
        TEST_ADD(TestIntegerFunctions::testMadHiSignedChar);
        TEST_ADD(TestIntegerFunctions::testMadHiUnsignedInt);
        TEST_ADD(TestIntegerFunctions::testMadHiUnsignedShort);
        TEST_ADD(TestIntegerFunctions::testMadHiUnsignedChar);

        TEST_ADD(TestIntegerFunctions::testMadSatSignedInt);
        TEST_ADD(TestIntegerFunctions::testMadSatSignedShort);
        TEST_ADD(TestIntegerFunctions::testMadSatSignedChar);
        TEST_ADD(TestIntegerFunctions::testMadSatUnsignedInt);
        TEST_ADD(TestIntegerFunctions::testMadSatUnsignedShort);
        TEST_ADD(TestIntegerFunctions::testMadSatUnsignedChar);
*/

    TEST_ADD(TestIntegerFunctions::testMaxSignedInt);
    TEST_ADD(TestIntegerFunctions::testMaxSignedShort);
    TEST_ADD(TestIntegerFunctions::testMaxSignedChar);
    TEST_ADD(TestIntegerFunctions::testMaxUnsignedInt);
    TEST_ADD(TestIntegerFunctions::testMaxUnsignedShort);
    TEST_ADD(TestIntegerFunctions::testMaxUnsignedChar);

    TEST_ADD(TestIntegerFunctions::testMinSignedInt);
    TEST_ADD(TestIntegerFunctions::testMinSignedShort);
    TEST_ADD(TestIntegerFunctions::testMinSignedChar);
    TEST_ADD(TestIntegerFunctions::testMinUnsignedInt);
    TEST_ADD(TestIntegerFunctions::testMinUnsignedShort);
    TEST_ADD(TestIntegerFunctions::testMinUnsignedChar);

    /*XXX
        TEST_ADD(TestIntegerFunctions::testMulHiSignedInt);
        TEST_ADD(TestIntegerFunctions::testMulHiSignedShort);
        TEST_ADD(TestIntegerFunctions::testMulHiSignedChar);
        TEST_ADD(TestIntegerFunctions::testMulHiUnsignedInt);
        TEST_ADD(TestIntegerFunctions::testMulHiUnsignedShort);
        TEST_ADD(TestIntegerFunctions::testMulHiUnsignedChar);
    */

    TEST_ADD(TestIntegerFunctions::testRotateSignedInt);
    TEST_ADD(TestIntegerFunctions::testRotateSignedShort);
    TEST_ADD(TestIntegerFunctions::testRotateSignedChar);
    TEST_ADD(TestIntegerFunctions::testRotateUnsignedInt);
    TEST_ADD(TestIntegerFunctions::testRotateUnsignedShort);
    TEST_ADD(TestIntegerFunctions::testRotateUnsignedChar);

    TEST_ADD(TestIntegerFunctions::testSubSatSignedInt);
    TEST_ADD(TestIntegerFunctions::testSubSatSignedShort);
    TEST_ADD(TestIntegerFunctions::testSubSatSignedChar);
    TEST_ADD(TestIntegerFunctions::testSubSatUnsignedInt);
    TEST_ADD(TestIntegerFunctions::testSubSatUnsignedShort);
    TEST_ADD(TestIntegerFunctions::testSubSatUnsignedChar);

    TEST_ADD(TestIntegerFunctions::testUpsampleSignedShortToInt);
    TEST_ADD(TestIntegerFunctions::testUpsampleSignedCharToShort);
    TEST_ADD(TestIntegerFunctions::testUpsampleUnsignedShortToInt);
    TEST_ADD(TestIntegerFunctions::testUpsampleUnsignedCharToShort);

    TEST_ADD(TestIntegerFunctions::testPopcountSignedInt);
    TEST_ADD(TestIntegerFunctions::testPopcountSignedShort);
    TEST_ADD(TestIntegerFunctions::testPopcountSignedChar);
    TEST_ADD(TestIntegerFunctions::testPopcountUnsignedInt);
    TEST_ADD(TestIntegerFunctions::testPopcountUnsignedShort);
    TEST_ADD(TestIntegerFunctions::testPopcountUnsignedChar);

    /* XXX
    TEST_ADD(TestIntegerFunctions::testMad24SignedInt);
    TEST_ADD(TestIntegerFunctions::testMad24UnsignedInt);

    TEST_ADD(TestIntegerFunctions::testMul24SignedInt);
    TEST_ADD(TestIntegerFunctions::testMul24UnsignedInt);
    */
}

void TestIntegerFunctions::onMismatch(const std::string& expected, const std::string& result)
{
    TEST_ASSERT_EQUALS(expected, result);
}

template <typename T, typename R = T>
static void testUnaryFunction(vc4c::Configuration& config, const std::string& options, const std::function<R(T)>& op,
    const std::function<void(const std::string&, const std::string&)>& onError)
{
    std::stringstream code;
    compileBuffer(config, code, UNARY_FUNCTION, options);

    auto in = generateInput<T, 16 * 12>(true);

    auto out = runEmulation<T, R, 16, 12>(code, {in});
    auto pos = options.find("-DFUNC=") + std::string("-DFUNC=").size();
    checkUnaryResults<R, T>(in, out, op, options.substr(pos, options.find(' ', pos) - pos), onError);
}

template <typename T, typename R = T>
static void testBinaryFunction(vc4c::Configuration& config, const std::string& options,
    const std::function<R(T, T)>& op, const std::function<void(const std::string&, const std::string&)>& onError)
{
    std::stringstream code;
    compileBuffer(config, code, BINARY_FUNCTION, options);

    auto in0 = generateInput<T, 16 * 12>(true);
    auto in1 = generateInput<T, 16 * 12>(true);

    auto out = runEmulation<T, R, 16, 12>(code, {in0, in1});
    auto pos = options.find("-DFUNC=") + std::string("-DFUNC=").size();
    checkBinaryResults<R, T>(in0, in1, out, op, options.substr(pos, options.find(' ', pos) - pos), onError);
}

template <typename T>
static typename std::make_unsigned<T>::type checkAbs(T in)
{
    return std::abs(in);
}

template <typename T>
static T checkIdentity(T in)
{
    return in;
}

template <typename T>
static typename std::make_unsigned<T>::type checkAbsDiff(T in1, T in2)
{
    return std::abs((in1 > in2) ? (static_cast<long>(in1) - static_cast<long>(in2)) :
                                  (static_cast<long>(in2) - static_cast<long>(in1)));
}

template <typename T>
static T checkDiff(T in1, T in2)
{
    return in1 > in2 ? (in1 - in2) : (in2 - in1);
}

template <typename T>
static T checkAddSat(T in1, T in2)
{
    return std::max(
        std::min(static_cast<long>(in1) + static_cast<long>(in2), static_cast<long>(std::numeric_limits<T>::max())),
        static_cast<long>(std::numeric_limits<T>::min()));
}

template <typename T>
static T checkHAdd(T in1, T in2)
{
    return (static_cast<long>(in1) + static_cast<long>(in2)) >> 1;
}

template <typename T>
static T checkRHAdd(T in1, T in2)
{
    return (static_cast<long>(in1) + static_cast<long>(in2) + 1) >> 1;
}

template <typename T>
static T checkClz(T in)
{
    for(int i = sizeof(T) * 8 - 1; i >= 0; --i)
    {
        if(((static_cast<unsigned long>(in) >> i) & 0x1) == 0x1)
            return sizeof(T) * 8 - 1 - i;
    }
    return sizeof(T) * 8;
}

template <typename T>
static T checkMulHi(T in1, T in2)
{
    return (static_cast<long>(in1) * static_cast<long>(in2)) >> sizeof(T) * 8;
}

// taken from: https://stackoverflow.com/questions/25799215/bitwise-rotation-circular-shift
template <typename T>
static T checkRotate(T v, T shift)
{
    // auto s = shift >= 0 ? shift % (sizeof(T) * 8) : -((-shift) % (sizeof(T) * 8));
    auto s = (static_cast<long>(shift) + (1l << 32)) % (sizeof(T) * 8);
    auto tmp = vc4c::bit_cast<T, typename std::make_unsigned<T>::type>(v);
    tmp = (tmp << s) | (tmp >> ((sizeof(T) * 8) - s));
    return vc4c::bit_cast<typename std::make_unsigned<T>::type, T>(tmp);
}

template <typename T>
static T checkSubSat(T in1, T in2)
{
    return std::max(
        std::min(static_cast<long>(in1) - static_cast<long>(in2), static_cast<long>(std::numeric_limits<T>::max())),
        static_cast<long>(std::numeric_limits<T>::min()));
}

template <typename R, typename T, typename U = typename std::make_unsigned<T>::type>
static R checkUpsample(T in1, U in2)
{
    return static_cast<R>(in1) << (sizeof(T) * 8) | static_cast<R>(in2);
}

template <typename T>
static T checkPopcount(T val)
{
    std::bitset<sizeof(T) * 8> set(val);
    return set.count();
}

void TestIntegerFunctions::testAbsSignedInt()
{
    testUnaryFunction<int, unsigned int>(config, "-DOUT=uint16 -DIN=int16 -DFUNC=abs", checkAbs<int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testAbsSignedShort()
{
    testUnaryFunction<short, unsigned short>(config, "-DOUT=ushort16 -DIN=short16 -DFUNC=abs", checkAbs<short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testAbsSignedChar()
{
    testUnaryFunction<char, unsigned char>(config, "-DOUT=uchar16 -DIN=char16 -DFUNC=abs", checkAbs<char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testAbsUnsignedInt()
{
    testUnaryFunction<unsigned int, unsigned int>(config, "-DOUT=uint16 -DIN=uint16 -DFUNC=abs",
        checkIdentity<unsigned int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testAbsUnsignedShort()
{
    testUnaryFunction<unsigned short, unsigned short>(config, "-DOUT=ushort16 -DIN=ushort16 -DFUNC=abs",
        checkIdentity<unsigned short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testAbsUnsignedChar()
{
    testUnaryFunction<unsigned char, unsigned char>(config, "-DOUT=uchar16 -DIN=uchar16 -DFUNC=abs",
        checkIdentity<unsigned char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testAbsDiffSignedInt()
{
    testBinaryFunction<int, unsigned int>(config, "-DOUT=uint16 -DIN0=int16 -DIN1=int16 -DFUNC=abs_diff",
        checkAbsDiff<int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testAbsDiffSignedShort()
{
    testBinaryFunction<short, unsigned short>(config, "-DOUT=ushort16 -DIN0=short16 -DIN1=short16 -DFUNC=abs_diff",
        checkAbsDiff<short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testAbsDiffSignedChar()
{
    testBinaryFunction<char, unsigned char>(config, "-DOUT=uchar16 -DIN0=char16 -DIN1=char16 -DFUNC=abs_diff",
        checkAbsDiff<char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testAbsDiffUnsignedInt()
{
    testBinaryFunction<unsigned int, unsigned int>(config, "-DOUT=uint16 -DIN0=uint16 -DIN1=uint16 -DFUNC=abs_diff",
        checkDiff<unsigned int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testAbsDiffUnsignedShort()
{
    testBinaryFunction<unsigned short, unsigned short>(config,
        "-DOUT=ushort16 -DIN0=ushort16 -DIN1=ushort16 -DFUNC=abs_diff", checkDiff<unsigned short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testAbsDiffUnsignedChar()
{
    testBinaryFunction<unsigned char, unsigned char>(config,
        "-DOUT=uchar16 -DIN0=uchar16 -DIN1=uchar16 -DFUNC=abs_diff", checkDiff<unsigned char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testAddSatSignedInt()
{
    testBinaryFunction<int, int>(config, "-DOUT=int16 -DIN0=int16 -DIN1=int16 -DFUNC=add_sat", checkAddSat<int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testAddSatSignedShort()
{
    testBinaryFunction<short, short>(config, "-DOUT=short16 -DIN0=short16 -DIN1=short16 -DFUNC=add_sat",
        checkAddSat<short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testAddSatSignedChar()
{
    testBinaryFunction<char, char>(config, "-DOUT=char16 -DIN0=char16 -DIN1=char16 -DFUNC=add_sat", checkAddSat<char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testAddSatUnsignedInt()
{
    testBinaryFunction<unsigned int, unsigned int>(config, "-DOUT=uint16 -DIN0=uint16 -DIN1=uint16 -DFUNC=add_sat",
        checkAddSat<unsigned int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testAddSatUnsignedShort()
{
    testBinaryFunction<unsigned short, unsigned short>(config,
        "-DOUT=ushort16 -DIN0=ushort16 -DIN1=ushort16 -DFUNC=add_sat", checkAddSat<unsigned short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testAddSatUnsignedChar()
{
    testBinaryFunction<unsigned char, unsigned char>(config, "-DOUT=uchar16 -DIN0=uchar16 -DIN1=uchar16 -DFUNC=add_sat",
        checkAddSat<unsigned char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testHAddSignedInt()
{
    testBinaryFunction<int, int>(config, "-DOUT=int16 -DIN0=int16 -DIN1=int16 -DFUNC=hadd", checkHAdd<int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testHAddSignedShort()
{
    testBinaryFunction<short, short>(config, "-DOUT=short16 -DIN0=short16 -DIN1=short16 -DFUNC=hadd", checkHAdd<short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testHAddSignedChar()
{
    testBinaryFunction<char, char>(config, "-DOUT=char16 -DIN0=char16 -DIN1=char16 -DFUNC=hadd", checkHAdd<char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testHAddUnsignedInt()
{
    testBinaryFunction<unsigned int, unsigned int>(config, "-DOUT=uint16 -DIN0=uint16 -DIN1=uint16 -DFUNC=hadd",
        checkHAdd<unsigned int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testHAddUnsignedShort()
{
    testBinaryFunction<unsigned short, unsigned short>(config,
        "-DOUT=ushort16 -DIN0=ushort16 -DIN1=ushort16 -DFUNC=hadd", checkHAdd<unsigned short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testHAddUnsignedChar()
{
    testBinaryFunction<unsigned char, unsigned char>(config, "-DOUT=uchar16 -DIN0=uchar16 -DIN1=uchar16 -DFUNC=hadd",
        checkHAdd<unsigned char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testRHAddSignedInt()
{
    testBinaryFunction<int, int>(config, "-DOUT=int16 -DIN0=int16 -DIN1=int16 -DFUNC=rhadd", checkRHAdd<int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testRHAddSignedShort()
{
    testBinaryFunction<short, short>(config, "-DOUT=short16 -DIN0=short16 -DIN1=short16 -DFUNC=rhadd",
        checkRHAdd<short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testRHAddSignedChar()
{
    testBinaryFunction<char, char>(config, "-DOUT=char16 -DIN0=char16 -DIN1=char16 -DFUNC=rhadd", checkRHAdd<char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testRHAddUnsignedInt()
{
    testBinaryFunction<unsigned int, unsigned int>(config, "-DOUT=uint16 -DIN0=uint16 -DIN1=uint16 -DFUNC=rhadd",
        checkRHAdd<unsigned int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testRHAddUnsignedShort()
{
    testBinaryFunction<unsigned short, unsigned short>(config,
        "-DOUT=ushort16 -DIN0=ushort16 -DIN1=ushort16 -DFUNC=rhadd", checkRHAdd<unsigned short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testRHAddUnsignedChar()
{
    testBinaryFunction<unsigned char, unsigned char>(config, "-DOUT=uchar16 -DIN0=uchar16 -DIN1=uchar16 -DFUNC=rhadd",
        checkRHAdd<unsigned char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testClzSignedInt()
{
    testUnaryFunction<int, int>(config, "-DOUT=int16 -DIN=int16 -DFUNC=clz", checkClz<int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testClzSignedShort()
{
    testUnaryFunction<short, short>(config, "-DOUT=short16 -DIN=short16 -DFUNC=clz", checkClz<short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testClzSignedChar()
{
    testUnaryFunction<char, char>(config, "-DOUT=char16 -DIN=char16 -DFUNC=clz", checkClz<char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testClzUnsignedInt()
{
    testUnaryFunction<unsigned int, unsigned int>(config, "-DOUT=uint16 -DIN=uint16 -DFUNC=clz", checkClz<unsigned int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testClzUnsignedShort()
{
    testUnaryFunction<unsigned short, unsigned short>(config, "-DOUT=ushort16 -DIN=ushort16 -DFUNC=clz",
        checkClz<unsigned short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testClzUnsignedChar()
{
    testUnaryFunction<unsigned char, unsigned char>(config, "-DOUT=uchar16 -DIN=uchar16 -DFUNC=clz",
        checkClz<unsigned char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testMaxSignedInt()
{
    testBinaryFunction<int, int>(config, "-DOUT=int16 -DIN0=int16 -DIN1=int16 -DFUNC=max", std::max<int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testMaxSignedShort()
{
    testBinaryFunction<short, short>(config, "-DOUT=short16 -DIN0=short16 -DIN1=short16 -DFUNC=max", std::max<short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testMaxSignedChar()
{
    testBinaryFunction<char, char>(config, "-DOUT=char16 -DIN0=char16 -DIN1=char16 -DFUNC=max", std::max<char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testMaxUnsignedInt()
{
    testBinaryFunction<unsigned int, unsigned int>(config, "-DOUT=uint16 -DIN0=uint16 -DIN1=uint16 -DFUNC=max",
        std::max<unsigned int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testMaxUnsignedShort()
{
    testBinaryFunction<unsigned short, unsigned short>(config,
        "-DOUT=ushort16 -DIN0=ushort16 -DIN1=ushort16 -DFUNC=max", std::max<unsigned short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testMaxUnsignedChar()
{
    testBinaryFunction<unsigned char, unsigned char>(config, "-DOUT=uchar16 -DIN0=uchar16 -DIN1=uchar16 -DFUNC=max",
        std::max<unsigned char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testMinSignedInt()
{
    testBinaryFunction<int, int>(config, "-DOUT=int16 -DIN0=int16 -DIN1=int16 -DFUNC=min", std::min<int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testMinSignedShort()
{
    testBinaryFunction<short, short>(config, "-DOUT=short16 -DIN0=short16 -DIN1=short16 -DFUNC=min", std::min<short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testMinSignedChar()
{
    testBinaryFunction<char, char>(config, "-DOUT=char16 -DIN0=char16 -DIN1=char16 -DFUNC=min", std::min<char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testMinUnsignedInt()
{
    testBinaryFunction<unsigned int, unsigned int>(config, "-DOUT=uint16 -DIN0=uint16 -DIN1=uint16 -DFUNC=min",
        std::min<unsigned int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testMinUnsignedShort()
{
    testBinaryFunction<unsigned short, unsigned short>(config,
        "-DOUT=ushort16 -DIN0=ushort16 -DIN1=ushort16 -DFUNC=min", std::min<unsigned short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testMinUnsignedChar()
{
    testBinaryFunction<unsigned char, unsigned char>(config, "-DOUT=uchar16 -DIN0=uchar16 -DIN1=uchar16 -DFUNC=min",
        std::min<unsigned char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testMulHiSignedInt()
{
    testBinaryFunction<int, int>(config, "-DOUT=int16 -DIN0=int16 -DIN1=int16 -DFUNC=mul_hi", checkMulHi<int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testMulHiSignedShort()
{
    testBinaryFunction<short, short>(config, "-DOUT=short16 -DIN0=short16 -DIN1=short16 -DFUNC=mul_hi",
        checkMulHi<short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testMulHiSignedChar()
{
    testBinaryFunction<char, char>(config, "-DOUT=char16 -DIN0=char16 -DIN1=char16 -DFUNC=mul_hi", checkMulHi<char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testMulHiUnsignedInt()
{
    testBinaryFunction<unsigned int, unsigned int>(config, "-DOUT=uint16 -DIN0=uint16 -DIN1=uint16 -DFUNC=mul_hi",
        checkMulHi<unsigned int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testMulHiUnsignedShort()
{
    testBinaryFunction<unsigned short, unsigned short>(config,
        "-DOUT=ushort16 -DIN0=ushort16 -DIN1=ushort16 -DFUNC=mul_hi", checkMulHi<unsigned short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testMulHiUnsignedChar()
{
    testBinaryFunction<unsigned char, unsigned char>(config, "-DOUT=uchar16 -DIN0=uchar16 -DIN1=uchar16 -DFUNC=mul_hi",
        checkMulHi<unsigned char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testRotateSignedInt()
{
    testBinaryFunction<int, int>(config, "-DOUT=int16 -DIN0=int16 -DIN1=int16 -DFUNC=rotate", checkRotate<int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testRotateSignedShort()
{
    testBinaryFunction<short, short>(config, "-DOUT=short16 -DIN0=short16 -DIN1=short16 -DFUNC=rotate",
        checkRotate<short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testRotateSignedChar()
{
    testBinaryFunction<char, char>(config, "-DOUT=char16 -DIN0=char16 -DIN1=char16 -DFUNC=rotate", checkRotate<char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testRotateUnsignedInt()
{
    testBinaryFunction<unsigned int, unsigned int>(config, "-DOUT=uint16 -DIN0=uint16 -DIN1=uint16 -DFUNC=rotate",
        checkRotate<unsigned int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testRotateUnsignedShort()
{
    testBinaryFunction<unsigned short, unsigned short>(config,
        "-DOUT=ushort16 -DIN0=ushort16 -DIN1=ushort16 -DFUNC=rotate", checkRotate<unsigned short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testRotateUnsignedChar()
{
    testBinaryFunction<unsigned char, unsigned char>(config, "-DOUT=uchar16 -DIN0=uchar16 -DIN1=uchar16 -DFUNC=rotate",
        checkRotate<unsigned char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testSubSatSignedInt()
{
    testBinaryFunction<int, int>(config, "-DOUT=int16 -DIN0=int16 -DIN1=int16 -DFUNC=sub_sat", checkSubSat<int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testSubSatSignedShort()
{
    testBinaryFunction<short, short>(config, "-DOUT=short16 -DIN0=short16 -DIN1=short16 -DFUNC=sub_sat",
        checkSubSat<short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testSubSatSignedChar()
{
    testBinaryFunction<char, char>(config, "-DOUT=char16 -DIN0=char16 -DIN1=char16 -DFUNC=sub_sat", checkSubSat<char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testSubSatUnsignedInt()
{
    testBinaryFunction<unsigned int, unsigned int>(config, "-DOUT=uint16 -DIN0=uint16 -DIN1=uint16 -DFUNC=sub_sat",
        checkSubSat<unsigned int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testSubSatUnsignedShort()
{
    testBinaryFunction<unsigned short, unsigned short>(config,
        "-DOUT=ushort16 -DIN0=ushort16 -DIN1=ushort16 -DFUNC=sub_sat", checkSubSat<unsigned short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testSubSatUnsignedChar()
{
    testBinaryFunction<unsigned char, unsigned char>(config, "-DOUT=uchar16 -DIN0=uchar16 -DIN1=uchar16 -DFUNC=sub_sat",
        checkSubSat<unsigned char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testUpsampleSignedShortToInt()
{
    testBinaryFunction<short, int>(config, "-DOUT=int16 -DIN0=short16 -DIN1=ushort16 -DFUNC=upsample",
        checkUpsample<int, short, unsigned short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testUpsampleSignedCharToShort()
{
    testBinaryFunction<char, short>(config, "-DOUT=short16 -DIN0=char16 -DIN1=uchar16 -DFUNC=upsample",
        checkUpsample<short, char, unsigned char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testUpsampleUnsignedShortToInt()
{
    testBinaryFunction<unsigned short, unsigned int>(config,
        "-DOUT=uint16 -DIN0=ushort16 -DIN1=ushort16 -DFUNC=upsample",
        checkUpsample<unsigned int, unsigned short, unsigned short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testUpsampleUnsignedCharToShort()
{
    testBinaryFunction<unsigned char, unsigned short>(config,
        "-DOUT=ushort16 -DIN0=uchar16 -DIN1=uchar16 -DFUNC=upsample",
        checkUpsample<unsigned short, unsigned char, unsigned char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testPopcountSignedInt()
{
    testUnaryFunction<int, int>(config, "-DOUT=int16 -DIN=int16 -DFUNC=popcount", checkPopcount<int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testPopcountSignedShort()
{
    testUnaryFunction<short, short>(config, "-DOUT=short16 -DIN=short16 -DFUNC=popcount", checkPopcount<short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testPopcountSignedChar()
{
    testUnaryFunction<char, char>(config, "-DOUT=char16 -DIN=char16 -DFUNC=popcount", checkPopcount<char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testPopcountUnsignedInt()
{
    testUnaryFunction<unsigned int, unsigned int>(config, "-DOUT=uint16 -DIN=uint16 -DFUNC=popcount",
        checkPopcount<unsigned int>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testPopcountUnsignedShort()
{
    testUnaryFunction<unsigned short, unsigned short>(config, "-DOUT=ushort16 -DIN=ushort16 -DFUNC=popcount",
        checkPopcount<unsigned short>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}

void TestIntegerFunctions::testPopcountUnsignedChar()
{
    testUnaryFunction<unsigned char, unsigned char>(config, "-DOUT=uchar16 -DIN=uchar16 -DFUNC=popcount",
        checkPopcount<unsigned char>,
        std::bind(&TestIntegerFunctions::onMismatch, this, std::placeholders::_1, std::placeholders::_2));
}