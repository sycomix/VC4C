/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#include "Operators.h"

#include "../intermediate/Helper.h"
#include "../periphery/SFU.h"
#include "Comparisons.h"
#include "log.h"

#include <bitset>
#include <cmath>

using namespace vc4c;
using namespace vc4c::intermediate;

InstructionWalker intermediate::intrinsifySignedIntegerMultiplication(
    Method& method, InstructionWalker it, IntrinsicOperation& op)
{
    Value opDest = op.getOutput().value();

    const Value& arg0 = op.assertArgument(0);
    const Value& arg1 = op.assertArgument(1);

    // convert operands to positive
    Value op1Sign = UNDEFINED_VALUE;
    Value op2Sign = UNDEFINED_VALUE;
    Value op1Pos = method.addNewLocal(arg0.type, "%unsigned");
    Value op2Pos = method.addNewLocal(arg1.type, "%unsigned");

    it = insertMakePositive(it, method, arg0, op1Pos, op1Sign);
    it = insertMakePositive(it, method, arg1, op2Pos, op2Sign);

    op.setArgument(0, op1Pos);
    op.setArgument(1, op2Pos);

    // use new temporary result, so we can store the final result in the correct value
    const Value tmpDest = method.addNewLocal(opDest.type, "%result");
    op.setOutput(tmpDest);

    // do unsigned multiplication
    it = intrinsifyUnsignedIntegerMultiplication(method, it, op);
    // skip the original instruction
    it.nextInBlock();

    if(op1Sign.hasLiteral(INT_ZERO.literal()) && op2Sign.hasLiteral(INT_ZERO.literal()))
    {
        // if both operands are marked with (unsigned), we don't need to invert the result
        it.emplace(new MoveOperation(opDest, tmpDest));
        it.nextInBlock();
        return it;
    }
    else
    {
        // if exactly one operand was negative, invert sign of result
        const Value eitherSign = method.addNewLocal(TYPE_INT32.toVectorType(tmpDest.type.getVectorWidth()));
        it.emplace(new Operation(OP_XOR, eitherSign, op1Sign, op2Sign));
        it.nextInBlock();
        return insertRestoreSign(it, method, tmpDest, opDest, eitherSign);
    }
}

InstructionWalker intermediate::intrinsifyUnsignedIntegerMultiplication(
    Method& method, InstructionWalker it, IntrinsicOperation& op)
{
    const Value& arg0 = op.getFirstArg();
    const Value& arg1 = op.getSecondArg().value_or(UNDEFINED_VALUE);

    // mul24 can multiply 24-bits * 24-bits into 32-bits
    // default case, full multiplication
    // NOTE: the instructions are ordered in a way, that the insertion of NOPs to split read-after-write is minimal
    logging::debug() << "Intrinsifying unsigned multiplication of integers" << logging::endl;

    const Value a0 = method.addNewLocal(op.getOutput()->type, "%mul.a0");
    const Value a1 = method.addNewLocal(op.getOutput()->type, "%mul.a1");
    const Value b0 = method.addNewLocal(op.getOutput()->type, "%mul.b0");
    const Value b1 = method.addNewLocal(op.getOutput()->type, "%mul.b1");
    const Value out0 = method.addNewLocal(op.getOutput()->type, "%mul.out0");
    const Value out1 = method.addNewLocal(op.getOutput()->type, "%mul.out1");
    const Value out2 = method.addNewLocal(op.getOutput()->type, "%mul.out2");
    /*
     *                             |     a[0]    .    a[1]     |
     *  *                          |     b[0]    .    b[1]     |
     * ---------------------------------------------------------
     * |xxxxxx.xxxxxx.xxxxxx.xxxxxx|      .      .      .      |
     *
     *                             |        a[1] * b[1]        |
     *   +           |        a[1] * b[0]        |
     *   +           |        a[0] * b[1]        |
     *
     */
    // split arguments into parts
    bool hasA0Part, hasA1Part, hasB0Part, hasB1Part;
    if(arg0.getLiteralValue())
    {
        hasA1Part = (arg0.getLiteralValue()->unsignedInt() & 0xFFFF) != 0;
        it.emplace(new MoveOperation(a1, Value(Literal(arg0.getLiteralValue()->unsignedInt() & 0xFFFF), TYPE_INT16)));
        it.nextInBlock();
        hasA0Part = (arg0.getLiteralValue()->unsignedInt() >> 16) != 0;
        it.emplace(new MoveOperation(a0, Value(Literal(arg0.getLiteralValue()->unsignedInt() >> 16), TYPE_INT16)));
        it.nextInBlock();
    }
    else
    {
        hasA0Part = true; // not known
        hasA1Part = true;
        it.emplace(new Operation(OP_AND, a1, arg0, Value(Literal(0xFFFFu), TYPE_INT16)));
        it.nextInBlock();
        it.emplace(new Operation(OP_SHR, a0, arg0, Value(Literal(16u), TYPE_INT16)));
        it.nextInBlock();
    }
    if(arg1.getLiteralValue())
    {
        hasB1Part = (arg1.getLiteralValue()->unsignedInt() & 0xFFFF) != 0;
        it.emplace(new MoveOperation(b1, Value(Literal(arg1.getLiteralValue()->unsignedInt() & 0xFFFF), TYPE_INT8)));
        it.nextInBlock();
        hasB0Part = (arg1.getLiteralValue()->unsignedInt() >> 16) != 0;
        it.emplace(new MoveOperation(b0, Value(Literal(arg1.getLiteralValue()->unsignedInt() >> 16), TYPE_INT8)));
        it.nextInBlock();
    }
    else
    {
        // not known
        hasB0Part = true;
        hasB1Part = true;
        it.emplace(new Operation(OP_AND, b1, arg1, Value(Literal(0xFFFFu), TYPE_INT8)));
        it.nextInBlock();
        it.emplace(new Operation(OP_SHR, b0, arg1, Value(Literal(16u), TYPE_INT8)));
        it.nextInBlock();
    }
    if(hasA1Part && hasB1Part)
    {
        it.emplace(new Operation(OP_MUL24, out0, a1, b1));
        it.nextInBlock();
    }
    else
    {
        it.emplace(new MoveOperation(out0, INT_ZERO));
        it.nextInBlock();
    }
    if(hasA1Part && hasB0Part)
    {
        const Value tmp = method.addNewLocal(op.getOutput()->type);
        it.emplace(new Operation(OP_MUL24, tmp, a1, b0));
        it.nextInBlock();
        it.emplace(new Operation(OP_SHL, tmp, tmp, Value(Literal(16u), TYPE_INT8)));
        it.nextInBlock();
        it.emplace(new Operation(OP_ADD, out1, out0, tmp));
        it.nextInBlock();
    }
    else
    {
        it.emplace(new MoveOperation(out1, out0));
        it.nextInBlock();
    }
    if(hasA0Part && hasB1Part)
    {
        const Value tmp = method.addNewLocal(op.getOutput()->type);
        it.emplace(new Operation(OP_MUL24, tmp, a0, b1));
        it.nextInBlock();
        it.emplace(new Operation(OP_SHL, out2, tmp, Value(Literal(16u), TYPE_INT8)));
        it.nextInBlock();
    }
    else
    {
        it.emplace(new MoveOperation(out2, INT_ZERO));
        it.nextInBlock();
    }
    it.reset(new Operation(OP_ADD, op.getOutput().value(), out1, out2));
    it->addDecorations(InstructionDecorations::UNSIGNED_RESULT);

    return it;
}

/*
 * "The number of elementary operations q is the number of 1’s in the binary expansion, minus 1"
 * note: q is a shift and an addition
 *
 * - https://www.clear.rice.edu/comp512/Lectures/Papers/Lefevre-Multiplication.pdf, page 5
 *
 * Since we use a minimum of 8 operations for unsigned multiplication (without any optimization applied),
 * we set this as the threshold for applying this optimization.
 */
static constexpr int BINARY_METHOD_OPERATIONS_THRESHOLD = 8;

bool intermediate::canOptimizeMultiplicationWithBinaryMethod(const IntrinsicOperation& op)
{
    return std::any_of(op.getArguments().begin(), op.getArguments().end(), [](const Value& arg) -> bool {
        if(arg.getLiteralValue() && arg.getLiteralValue()->signedInt() > 0)
        {
            std::bitset<32> tmp(arg.getLiteralValue()->unsignedInt());
            return tmp.count() <= BINARY_METHOD_OPERATIONS_THRESHOLD;
        }
        return false;
    });
}

/*
 * Optimization of integer multiplication with binary method
 *
 * See: https://www.clear.rice.edu/comp512/Lectures/Papers/Lefevre-Multiplication.pdf, chapter 3
 */
InstructionWalker intermediate::intrinsifyIntegerMultiplicationViaBinaryMethod(
    Method& method, InstructionWalker it, IntrinsicOperation& op)
{
    auto factor = op.getFirstArg().getLiteralValue() ? op.getFirstArg().getLiteralValue()->signedInt() :
                                                       op.getSecondArg()->getLiteralValue()->signedInt();
    const auto& src = op.getFirstArg().getLiteralValue() ? op.getSecondArg().value() : op.getFirstArg();

    if(factor <= 0)
        throw CompilationError(
            CompilationStep::OPTIMIZER, "Invalid factor for this multiplication optimization", op.to_string());

    // tracks the deconstruction of the factor into its parts
    std::bitset<32> deconstruct(static_cast<unsigned>(factor));
    auto highBit = [](auto u) -> unsigned {
        for(unsigned i = 0; i < 32; ++i)
        {
            if((u >> i) == 0)
                return i - 1;
        }
        return 32;
    };
    Value intermediateResult = src;
    unsigned lastHigh = highBit(deconstruct.to_ulong());
    deconstruct.reset(lastHigh);
    /*
     * 113 = 1110001 (2)
     * x * 113:
     * 3x = x << 1 + x
     * 7x = 3x << 1 + 1
     * 113x = 7x << 4 + 1
     */
    while(deconstruct.any())
    {
        unsigned nextHigh = highBit(deconstruct.to_ulong());

        // shift by the difference between the high bits
        auto tmp = method.addNewLocal(src.type, "%mul_shift");
        it.emplace(new Operation(OP_SHL, tmp, intermediateResult, Value(Literal(lastHigh - nextHigh), TYPE_INT8)));
        it.nextInBlock();

        // add the value for this high bit
        auto tmp2 = method.addNewLocal(src.type, "%mul_add");
        it.emplace(new Operation(OP_ADD, tmp2, tmp, src));
        it.nextInBlock();

        intermediateResult = tmp2;
        deconstruct.reset(nextHigh);
        lastHigh = nextHigh;
    }

    if(lastHigh != 0)
    {
        // insert last shift
        it.reset(
            (new Operation(OP_SHL, op.getOutput().value(), intermediateResult, Value(Literal(lastHigh), TYPE_INT8)))
                ->copyExtrasFrom(&op));
    }
    else
    {
        it.reset((new MoveOperation(op.getOutput().value(), intermediateResult))->copyExtrasFrom(&op));
    }

    return it;
}

/*
 * Sources/Info:
 * - http://ipa.ece.illinois.edu/mif/pubs/web-only/Frank-RawMemo12-1999.html
 * - http://flounder.com/multiplicative_inverse.htm
 */

InstructionWalker intermediate::intrinsifySignedIntegerDivision(
    Method& method, InstructionWalker it, IntrinsicOperation& op, const bool useRemainder)
{
    Value opDest = op.getOutput().value();
    // check any operand is negative
    Value op1Sign = UNDEFINED_VALUE;
    Value op2Sign = UNDEFINED_VALUE;

    // convert operands to positive
    Value op1Pos = method.addNewLocal(op.assertArgument(0).type, "%unsigned");
    Value op2Pos = method.addNewLocal(op.assertArgument(0).type, "%unsigned");

    it = insertMakePositive(it, method, op.assertArgument(0), op1Pos, op1Sign);
    it = insertMakePositive(it, method, op.assertArgument(1), op2Pos, op2Sign);

    op.setArgument(0, op1Pos);
    op.setArgument(1, op2Pos);

    // use new temporary result, so we can store the final result in the correct value
    const Value tmpDest = method.addNewLocal(opDest.type, "%result");
    op.setOutput(tmpDest);

    // calculate unsigned division
    it = intrinsifyUnsignedIntegerDivision(method, it, op, useRemainder);
    it.nextInBlock();

    if(op1Sign.hasLiteral(INT_ZERO.literal()) && op2Sign.hasLiteral(INT_ZERO.literal()))
    {
        // if both operands are marked with (unsigned), we don't need to invert the result
        it.emplace(new MoveOperation(opDest, tmpDest));
        it.nextInBlock();
    }
    else if(useRemainder)
    {
        // For signed remainder (srem), the results sign only depends on the sign of the dividend!
        return insertRestoreSign(it, method, tmpDest, opDest, op1Sign);
    }
    else
    {
        // if exactly one operand was negative, invert sign of result
        Value eitherSign(TYPE_UNKNOWN);
        it = insertOperation(OP_XOR, it, method, eitherSign, op1Sign, op2Sign);
        return insertRestoreSign(it, method, tmpDest, opDest, eitherSign);
    }
    return it;
}

InstructionWalker intermediate::intrinsifyUnsignedIntegerDivision(
    Method& method, InstructionWalker it, IntrinsicOperation& op, const bool useRemainder)
{
    // https://en.wikipedia.org/wiki/Division_algorithm#Integer_division_.28unsigned.29_with_remainder
    // see also: https://www.microsoft.com/en-us/research/wp-content/uploads/2008/08/tr-2008-141.pdf
    // TODO for |type| < 24, use floating-point division??
    // NOTE: the instructions are ordered in a way, that the insertion of NOPs to split read-after-write is minimal
    const Value& numerator = op.getFirstArg();
    const Value& divisor = op.getSecondArg().value_or(UNDEFINED_VALUE);

    logging::debug() << "Intrinsifying division of unsigned integers" << logging::endl;

    // TODO divisor = 0 handling!

    // Q := 0                 -- initialize quotient and remainder to zero
    // R := 0
    Value quotient = method.addNewLocal(op.getOutput()->type, "%udiv.quotient");
    Value remainder = method.addNewLocal(op.getOutput()->type, "%udiv.remainder");
    // set explicitly to zero
    it.emplace(new MoveOperation(remainder, INT_ZERO));
    it.nextInBlock();
    it.emplace(new MoveOperation(quotient, INT_ZERO));
    it.nextInBlock();

    // for i := n ? 1 ... 0 do     -- where n is number of bits in N
    // "number of bits in N" as in in the numerator, not in the type!!
    // but since the leading bits in the numerator are all zero, including them does not modify the result
    for(int i = numerator.type.getScalarBitCount() - 1; i >= 0; --i)
    {
        // R := R << 1          -- left-shift R by 1 bit
        {
            const Value newRemainder = method.addNewLocal(op.getOutput()->type, "%udiv.remainder");
            it.emplace(new Operation(OP_SHL, newRemainder, remainder, INT_ONE));
            it.nextInBlock();
            remainder = newRemainder;
        }
        // R(0) := N(i)         -- set the least-significant bit of R equal to bit i of the numerator
        // R = R | ((N >> i) & 1) <=> R = R | (N & (1 << i) == 1 ? 1 : 0) <=> R = R | 1, if N & (1 << i) != 0
        {
            const Value newRemainder = method.addNewLocal(op.getOutput()->type, "%udiv.remainder");
            const Value tmp = method.addNewLocal(numerator.type, "%udiv.tmp");
            it.emplace(new Operation(OP_SHR, tmp, numerator, Value(Literal(i), TYPE_INT32)));
            it.nextInBlock();
            it.emplace(new Operation(OP_AND, tmp, tmp, INT_ONE));
            it.nextInBlock();
            // XXX actually OP_OR, but it gets somehow removed/optimized away
            it.emplace(new Operation(OP_ADD, newRemainder, remainder, tmp));
            it.nextInBlock();
            remainder = newRemainder;
        }
        // if R >= D then
        // R = R >= D ? R - D : R <=> R = R - D >= 0 ? R - D : R <=> R = R - D < 0 ? R : R - D
        {
            const Value tmp = method.addNewLocal(op.getOutput()->type, "%udiv.tmp");
            it.emplace(new Operation(OP_SUB, tmp, remainder, divisor, COND_ALWAYS, SetFlag::SET_FLAGS));
            it.nextInBlock();
            const Value newRemainder = method.addNewLocal(op.getOutput()->type, "%udiv.remainder");
            it.emplace(new MoveOperation(newRemainder, tmp, COND_NEGATIVE_CLEAR));
            it.nextInBlock();
            it.emplace(new MoveOperation(newRemainder, remainder, COND_NEGATIVE_SET));
            it.nextInBlock();
            remainder = newRemainder;
        }
        // Q(i) := 1
        {
            const Value newQuotient = method.addNewLocal(op.getOutput()->type, "%udiv.quotient");
            it.emplace(new Operation(OP_OR, newQuotient, quotient,
                Value(Literal(static_cast<int32_t>(1) << i), TYPE_INT32), COND_NEGATIVE_CLEAR));
            it.nextInBlock();
            // else Q(new) := Q(old)
            it.emplace(new MoveOperation(newQuotient, quotient, COND_NEGATIVE_SET));
            it.nextInBlock();
            quotient = newQuotient;
        }
    }

    // make move from original instruction

    if(useRemainder)
        it.reset(new MoveOperation(op.getOutput().value(), remainder));
    else
        it.reset(new MoveOperation(op.getOutput().value(), quotient));
    it->addDecorations(InstructionDecorations::UNSIGNED_RESULT);

    return it;
}

InstructionWalker intermediate::intrinsifySignedIntegerDivisionByConstant(
    Method& method, InstructionWalker it, IntrinsicOperation& op, bool useRemainder)
{
    /*
     * Conversion between signedness is taken from:
     * https://llvm.org/doxygen/IntegerDivision_8cpp_source.html
     */

    Value opDest = op.getOutput().value();
    // check any operand is negative
    Value op1Sign = UNDEFINED_VALUE;
    Value op2Sign = UNDEFINED_VALUE;

    // convert operands to positive
    Value op1Pos = method.addNewLocal(op.assertArgument(0).type, "%unsigned");
    Value op2Pos = method.addNewLocal(op.assertArgument(0).type, "%unsigned");

    it = insertMakePositive(it, method, op.assertArgument(0), op1Pos, op1Sign);
    it = insertMakePositive(it, method, op.assertArgument(1), op2Pos, op2Sign);

    op.setArgument(0, op1Pos);
    op.setArgument(1, op2Pos);

    // use new temporary result, so we can store the final result in the correct value
    const Value tmpDest = method.addNewLocal(opDest.type, "%result");
    op.setOutput(tmpDest);

    // calculate unsigned division
    it = intrinsifyUnsignedIntegerDivisionByConstant(method, it, op, useRemainder);
    it.nextInBlock();

    if(op1Sign.hasLiteral(INT_ZERO.literal()) && op2Sign.hasLiteral(INT_ZERO.literal()))
    {
        // if both operands are marked with (unsigned), we don't need to invert the result
        it.emplace(new MoveOperation(opDest, tmpDest));
        it.nextInBlock();
    }
    else if(useRemainder)
    {
        // For signed remainder (srem), the results sign only depends on the sign of the dividend!
        return insertRestoreSign(it, method, tmpDest, opDest, op1Sign);
    }
    else
    {
        // if exactly one operand was negative, invert sign of result
        Value eitherSign(TYPE_UNKNOWN);
        it = insertOperation(OP_XOR, it, method, eitherSign, op1Sign, op2Sign);
        return insertRestoreSign(it, method, tmpDest, opDest, eitherSign);
    }
    return it;
}

static std::pair<Literal, Literal> calculateConstant(Literal divisor, unsigned accuracy)
{
    uint32_t shift = static_cast<uint32_t>(std::log2(divisor.unsignedInt() * accuracy)) + 2;
    uint32_t factor =
        static_cast<uint32_t>(std::round(std::pow(2.0f, shift) / static_cast<float>(divisor.unsignedInt())));
    if(shift > 31)
        throw CompilationError(CompilationStep::OPTIMIZER,
            "Unsigned division by constant generated invalid shift offset", std::to_string(shift));
    if(factor >= std::numeric_limits<uint16_t>::max())
        throw CompilationError(CompilationStep::OPTIMIZER,
            "Unsigned division by constant generated invalid multiplication factor", std::to_string(factor));
    return std::make_pair(Literal(factor), Literal(shift));
}

static std::pair<Value, Value> calculateConstant(const Value& divisor, unsigned accuracy)
{
    if(divisor.hasContainer())
    {
        Value factors(ContainerValue(divisor.container().elements.size()), divisor.type);
        Value shifts(ContainerValue(divisor.container().elements.size()), divisor.type);
        for(const auto& element : divisor.container().elements)
        {
            auto tmp = calculateConstant(element.literal(), accuracy);
            factors.container().elements.push_back(Value(tmp.first, factors.type.toVectorType(1)));
            shifts.container().elements.push_back(Value(tmp.second, shifts.type.toVectorType(1)));
        }
        return std::make_pair(factors, shifts);
    }

    const Literal div = divisor.getLiteralValue().value();
    auto tmp = calculateConstant(div, accuracy);
    return std::make_pair(Value(tmp.first, divisor.type), Value(tmp.second, divisor.type));
}

InstructionWalker intermediate::intrinsifyUnsignedIntegerDivisionByConstant(
    Method& method, InstructionWalker it, IntrinsicOperation& op, bool useRemainder)
{
    /*
     * Taken from here:
     * http://forums.parallax.com/discussion/114807/fast-faster-fastest-code-integer-division
     *
     * If we accept unsigned char and short values, the maximum values for the numerator/denominator are USHORT_MAX
     * (65536). Thus, for the multiplication not to overflow for any numerator, the maximum value for the factor can be
     * USHORT_MAX - 1.
     */

    if(op.getFirstArg().type.getScalarBitCount() > 16)
        throw CompilationError(CompilationStep::OPTIMIZER, "Division by constant may overflow for argument type",
            op.getFirstArg().type.to_string());
    if(!op.getSecondArg().ifPresent(toFunction(&Value::isLiteralValue)) &&
        !(op.getSecondArg() && op.assertArgument(1).hasContainer()))
        throw CompilationError(CompilationStep::OPTIMIZER, "Can only optimize division by constant", op.to_string());

    /*
     * Relative accuracy, the value is determined by experiment:
     * - values <= 16000 trigger value mismatch to "exact" division
     * - values >= 16500 trigger overflow in multiplication with factor or shifts of >= 32 positions
     */
    static const unsigned accuracy = 16100;
    auto constants = calculateConstant(op.assertArgument(1), accuracy);
    logging::debug() << "Intrinsifying unsigned division by " << op.assertArgument(1).to_string(false, true)
                     << " by multiplication with " << constants.first.to_string(false, true) << " and right-shift by "
                     << constants.second.to_string(false, true) << logging::endl;

    const Value tmp = method.addNewLocal(op.getFirstArg().type, "%udiv");
    it.emplace(new Operation(OP_MUL24, tmp, op.getFirstArg(), constants.first));
    it.nextInBlock();
    const Value divOut = method.addNewLocal(op.getFirstArg().type, "%udiv");
    it.emplace(new Operation(OP_SHR, divOut, tmp, constants.second));
    it->copyExtrasFrom(&op);
    it->addDecorations(InstructionDecorations::UNSIGNED_RESULT);
    it.nextInBlock();
    // the original version has an error, which returns a too small value for exact multiples of the denominator, the
    // next lines fix this error
    const Value tmpFix0 = method.addNewLocal(op.getFirstArg().type, "%udiv.fix");
    const Value tmpFix1 = method.addNewLocal(op.getFirstArg().type, "%udiv.fix");
    it.emplace(new Operation(OP_MUL24, tmpFix0, divOut, op.assertArgument(1)));
    it.nextInBlock();
    it.emplace(new Operation(OP_SUB, tmpFix1, op.getFirstArg(), tmpFix0));
    it.nextInBlock();
    it.emplace(new Operation(OP_SUB, NOP_REGISTER, op.assertArgument(1), tmpFix1, COND_ALWAYS, SetFlag::SET_FLAGS));
    it.nextInBlock();
    const Value finalResult =
        useRemainder ? method.addNewLocal(op.getFirstArg().type, "%udiv.result") : op.getOutput().value();
    it.emplace(new MoveOperation(finalResult, divOut));
    it->addDecorations(InstructionDecorations::UNSIGNED_RESULT);
    it.nextInBlock();
    it.emplace(new Operation(OP_ADD, finalResult, divOut, INT_ONE, COND_NEGATIVE_SET));
    it->addDecorations(InstructionDecorations::UNSIGNED_RESULT);
    it.nextInBlock();
    it.emplace(new Operation(OP_ADD, finalResult, divOut, INT_ONE, COND_ZERO_SET));
    it->addDecorations(InstructionDecorations::UNSIGNED_RESULT);
    it.nextInBlock();

    if(useRemainder)
    {
        // x mod y = x - (x/y) * y;
        const Value tmpMul = method.addNewLocal(op.getFirstArg().type, "%udiv.remainder");
        it.emplace(new Operation(OP_MUL24, tmpMul, finalResult, op.assertArgument(1)));
        it.nextInBlock();
        // replace original division
        it.reset(new Operation(OP_SUB, op.getOutput().value(), op.getFirstArg(), tmpMul));
        it->addDecorations(InstructionDecorations::UNSIGNED_RESULT);
    }
    else
    {
        // erase original division
        it.erase();
        // so next instruction is not skipped
        it.previousInBlock();
    }
    return it;
}

InstructionWalker intermediate::intrinsifyFloatingDivision(Method& method, InstructionWalker it, IntrinsicOperation& op)
{
    // TODO correct??

    /*
     * https://dspace.mit.edu/bitstream/handle/1721.1/80133/43609668-MIT.pdf
     * https://en.wikipedia.org/wiki/Division_algorithm#Newton.E2.80.93Raphson_division
     * http://www.rfwireless-world.com/Tutorials/floating-point-tutorial.html
     */
    logging::debug() << "Intrinsifying floating-point division" << logging::endl;

    const Value nominator = op.getFirstArg();
    const Value& divisor = op.assertArgument(1);

    ////
    // Newton-Raphson
    ////
    // TODO: "The Newton-Raphson  algorithm [...] is commonly used if the result does not require proper rounding"
    //-> use Goldschmidt??

    // 1. initialization step: P0 = SFU_RECIP(D)
    /*
     * The GLSL shader uses the SFU_RECIP with a Newton-Raphson step "to improve our approximation",
     * see http://anholt.livejournal.com/49474.html
     */
    const Value P0 = method.addNewLocal(op.getOutput()->type, "%fdiv_recip");
    periphery::insertSFUCall(REG_SFU_RECIP, it, divisor);
    it.emplace(new MoveOperation(P0, Value(REG_SFU_OUT, TYPE_FLOAT)));
    it.nextInBlock();
    const Value const2(Literal(2.0f), TYPE_FLOAT);

    // 2. iteration step: Pi+1 = Pi(2 - D * Pi)
    // run 5 iterations
    const Value P1 = method.addNewLocal(op.getOutput()->type, "%fdiv_p1");
    const Value P1_1 = method.addNewLocal(op.getOutput()->type, "%fdiv_p1");
    const Value P1_2 = method.addNewLocal(op.getOutput()->type, "%fdiv_p1");
    it.emplace(new Operation(OP_FMUL, P1, divisor, P0));
    it.nextInBlock();
    it.emplace(new Operation(OP_FSUB, P1_1, const2, P1));
    it.nextInBlock();
    it.emplace(new Operation(OP_FMUL, P1_2, P0, P1_1));
    it.nextInBlock();

    const Value P2 = method.addNewLocal(op.getOutput()->type, "%fdiv_p2");
    const Value P2_1 = method.addNewLocal(op.getOutput()->type, "%fdiv_p2");
    const Value P2_2 = method.addNewLocal(op.getOutput()->type, "%fdiv_p2");
    it.emplace(new Operation(OP_FMUL, P2, divisor, P1_2));
    it.nextInBlock();
    it.emplace(new Operation(OP_FSUB, P2_1, const2, P2));
    it.nextInBlock();
    it.emplace(new Operation(OP_FMUL, P2_2, P1_2, P2_1));
    it.nextInBlock();

    const Value P3 = method.addNewLocal(op.getOutput()->type, "%fdiv_p3");
    const Value P3_1 = method.addNewLocal(op.getOutput()->type, "%fdiv_p3");
    const Value P3_2 = method.addNewLocal(op.getOutput()->type, "%fdiv_p3");
    it.emplace(new Operation(OP_FMUL, P3, divisor, P2_2));
    it.nextInBlock();
    it.emplace(new Operation(OP_FSUB, P3_1, const2, P3));
    it.nextInBlock();
    it.emplace(new Operation(OP_FMUL, P3_2, P2_2, P3_1));
    it.nextInBlock();

    const Value P4 = method.addNewLocal(op.getOutput()->type, "%fdiv_p4");
    const Value P4_1 = method.addNewLocal(op.getOutput()->type, "%fdiv_p4");
    const Value P4_2 = method.addNewLocal(op.getOutput()->type, "%fdiv_p4");
    it.emplace(new Operation(OP_FMUL, P4, divisor, P3_2));
    it.nextInBlock();
    it.emplace(new Operation(OP_FSUB, P4_1, const2, P4));
    it.nextInBlock();
    it.emplace(new Operation(OP_FMUL, P4_2, P3_2, P4_1));
    it.nextInBlock();

    const Value P5 = method.addNewLocal(op.getOutput()->type, "%fdiv_p5");
    const Value P5_1 = method.addNewLocal(op.getOutput()->type, "%fdiv_p5");
    const Value P5_2 = method.addNewLocal(op.getOutput()->type, "%fdiv_p5");
    it.emplace(new Operation(OP_FMUL, P5, divisor, P4_2));
    it.nextInBlock();
    it.emplace(new Operation(OP_FSUB, P5_1, const2, P5));
    it.nextInBlock();
    it.emplace(new Operation(OP_FMUL, P5_2, P4_2, P5_1));
    it.nextInBlock();

    // TODO add a 6th step? Sometimes the float-division is too inaccurate

    // 3. final step: Q = Pn * N
    it.reset(new Operation(OP_FMUL, op.getOutput().value(), nominator, P5_2));

    return it;
}

static constexpr unsigned MSB = 31;

Literal intermediate::asr(const DataType& type, const Literal& left, const Literal& right)
{
    std::bitset<sizeof(int32_t) * 8> tmp(left.signedInt());
    if(right.signedInt() < 0)
        throw CompilationError(CompilationStep::GENERAL, "ASR with negative numbers is not implemented");
    for(auto i = 0; i < right.signedInt(); ++i)
    {
        bool MSBSet = tmp.test(MSB);
        tmp >>= 1;
        tmp.set(MSB, MSBSet);
    }
    return Literal(static_cast<uint32_t>(tmp.to_ulong()));
}

Literal intermediate::clz(const DataType& type, const Literal& val)
{
    for(int i = MSB; i >= 0; --i)
    {
        if(((val.unsignedInt() >> i) & 0x1) == 0x1)
            return Literal(static_cast<int32_t>(MSB - i));
    }
    // FIXME is this correct? What does clz(0) return on VC4?
    return Literal(32);
}

Literal intermediate::smod(const DataType& type, const Literal& numerator, const Literal& denominator)
{
    throw CompilationError(CompilationStep::GENERAL, "SMOD is currently not implemented!");
}

Literal intermediate::srem(const DataType& type, const Literal& numerator, const Literal& denominator)
{
    throw CompilationError(CompilationStep::GENERAL, "SREM is currently not implemented!");
}

Literal intermediate::fmod(const DataType& type, const Literal& numerator, const Literal& denominator)
{
    throw CompilationError(CompilationStep::GENERAL, "FMOD is currently not implemented!");
}

Literal intermediate::frem(const DataType& type, const Literal& numerator, const Literal& denominator)
{
    throw CompilationError(CompilationStep::GENERAL, "FREM is currently not implemented!");
}
