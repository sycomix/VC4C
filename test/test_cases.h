/*
 * Contains test-cases for emulator (and execution) test
 *
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#ifndef VC4C_TEST_CASES_H
#define VC4C_TEST_CASES_H

#ifdef VC4C_TOOLS_HEADER
#include VC4C_TOOLS_HEADER
#else
#include "tools.h"
#include "helper.h"
#endif

#include <algorithm>
#include <map>
#include <vector>

#ifndef VC4C_ROOT_PATH
#define VC4C_ROOT_PATH "./"
#endif

namespace vc4c
{
	namespace test
	{
		using namespace vc4c::tools;

		constexpr uint32_t maxExecutionCycles{1 << 16};

		template<typename T>
		static std::map<uint32_t, std::vector<uint32_t>> addVector(std::map<uint32_t, std::vector<uint32_t>> input, uint32_t index, std::vector<T> values)
		{
			std::vector<uint32_t> buffer;
			buffer.reserve(values.size());
			for(T val : values)
				buffer.push_back(bit_cast<T, uint32_t>(val));
			input.emplace(index, buffer);
			return input;
		}

		template<>
		std::map<uint32_t, std::vector<uint32_t>> addVector(std::map<uint32_t, std::vector<uint32_t>> input, uint32_t index, std::vector<uint32_t> values)
		{
			input.emplace(index, values);
			return input;
		}

		template<typename T>
		static std::pair<uint32_t, Optional<std::vector<uint32_t>>> toParameter(const std::vector<T>& values)
		{
			std::vector<uint32_t> buffer;
			buffer.reserve(values.size());
			for(T val : values)
				buffer.push_back(bit_cast<T, uint32_t>(val));

			return std::make_pair(0, buffer);
		}

		template<typename T>
		static std::pair<uint32_t, Optional<std::vector<uint32_t>>> toScalarParameter(T val)
		{
			return std::make_pair(bit_cast<T, uint32_t>(val), Optional<std::vector<uint32_t>>{});
		}

		template<typename T>
		static std::vector<T> toRange(T start, T end, T step = 1)
		{
			std::vector<T> out;
			for(T val = start; val != end; val += step)
				out.push_back(val);
			return out;
		}
		
		template<typename T, typename R>
		static std::vector<R> transfer(std::vector<T> in, const std::function<R(T)>& func)
		{
			std::vector<R> out;
			out.resize(in.size());
			std::transform(in.begin(), in.end(), out.begin(), func);
			return out;
		}

		static WorkGroupConfig toConfig(uint32_t localSizeX, uint32_t localSizeY = 1, uint32_t localSizeZ = 1, uint32_t numGroupsX = 1, uint32_t numGroupsY = 1, uint32_t numGroupsZ = 1)
		{
			WorkGroupConfig config;
			config.dimensions = 3;
			config.globalOffsets.fill(0);
			config.localSizes[0] = localSizeX;
			config.localSizes[1] = localSizeY;
			config.localSizes[2] = localSizeZ;
			config.numGroups[0] = numGroupsX;
			config.numGroups[1] = numGroupsY;
			config.numGroups[2] = numGroupsZ;

			return config;
		}

		static std::vector<std::pair<EmulationData, std::map<uint32_t, std::vector<uint32_t>>>> integerTests = {
				std::make_pair(
					EmulationData(VC4C_ROOT_PATH "example/fibonacci.cl", "fibonacci",
					{toScalarParameter(1u), toScalarParameter(1u), toParameter(std::vector<uint32_t>(10))},
					{}, maxExecutionCycles),
					addVector({}, 2, std::vector<uint32_t>{2, 3, 5, 8, 13, 21, 34, 55, 89, 144})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "example/test.cl", "test_llvm_ir",
					{toParameter(std::vector<uint32_t>(1))}, {}, maxExecutionCycles),
					addVector({}, 0, std::vector<uint32_t>{142})
				),
				//TODO requires v8muld support
//				std::make_pair(EmulationData(VC4C_ROOT_PATH "example/test_instructions.cl", "test_instructions",
//					{toScalarParameter(2u), toScalarParameter(4u), toScalarParameter(2.0f), toScalarParameter(4.0f), toParameter(std::vector<uint32_t>(32)), toParameter(std::vector<uint32_t>(32))}, {}, maxExecutionCycles),
//					addVector({}, 4, std::vector<uint32_t>{6, bit_cast<int32_t, uint32_t>(-2), 8, 0, 2, 4, 2, 2, 32, 0, 0, 6, 6, bit_cast<int32_t, uint32_t>(-3), 30, 3, 3, 1, 1, 0, 0, 0, 1, 1, 1, 0, 4, 8})
//				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/test_struct.cl", "test_struct",
					{toParameter(std::vector<uint32_t>(20)), toParameter(std::vector<uint32_t>(20))}, {}, maxExecutionCycles),
					addVector({}, 1, std::vector<uint32_t>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 42, 0})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/test_vector.cl", "test_copy",
					{toParameter(toRange(1, 17)), std::make_pair(0, std::vector<uint32_t>(32))}, {}, maxExecutionCycles),
					addVector({}, 1, std::vector<uint32_t>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/test_other.cl", "test_atomics",
					{toParameter(toRange<int32_t>(1, 12)), toParameter(toRange<int32_t>(1, 12))}, {}, maxExecutionCycles),
					addVector({}, 1, std::vector<int32_t>{2, 0, 3, 5, 4, 6, 7, 8, 9, 10, 0})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/test_other.cl", "test_f2i",
					{toScalarParameter(1.0f), toScalarParameter(1.1f), toScalarParameter(1.5f), toScalarParameter(1.9f), toParameter(std::vector<int>(32))}, {}, maxExecutionCycles),
					addVector({}, 4, std::vector<int>{1, 1, 1, 1, -1, -1, -1, -1, 1, -1, 2, -1, 1, -1, 1, -2, 1, -1, 2, -2, 1, 1, 2, 2, -1, -1, -2, -2, 1, -1})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/test_other.cl", "test_global_data",
					{toScalarParameter(1), toParameter(std::vector<int32_t>(2))}, {}, maxExecutionCycles),
					addVector({}, 1, std::vector<int32_t>{2, 21})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/test_vectorization.cl", "test4",
					{toParameter(toRange<int>(0, 1024)), toParameter(std::vector<int>(1))}, {}, maxExecutionCycles * 2),
					addVector({}, 1, std::vector<int>{528896})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/test_vectorization.cl", "test8",
					{toParameter(toRange<int>(0, 1024)), toParameter(toRange<int>(0, 4096))}, {}, maxExecutionCycles * 2),
					addVector({}, 0, std::vector<int>{0, 5, 10, 15, 20, 25, 30, 35, 40})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/test_vectorization.cl", "test9",
					{toParameter(toRange<int>(0, 4096)), toParameter(toRange<int>(0, 4096))}, {}, maxExecutionCycles * 2),
					addVector({}, 0, std::vector<int>{0, 1, 2, 3, 5, 5, 6, 7, 10, 9, 10, 11, 15, 13, 14, 15, 20})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/test_vectorization.cl", "test10",
					{toParameter(toRange<int>(0, 1024)), toParameter(toRange<int>(0, 1024))}, {}, maxExecutionCycles * 2),
					addVector({}, 0, std::vector<int>{0, 5, 10, 15, 20, 25, 30, 35, 40})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/test_vectorization.cl", "test11",
					{toParameter(toRange<int>(0, 256))}, {}, maxExecutionCycles),
					addVector({}, 0, std::vector<int>{100,100,100,100,100,100,100,100,100})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/OpenCL-CTS/pointer_cast.cl", "test_pointer_cast",
					{toParameter(std::vector<unsigned>{0x01020304}), toParameter(std::vector<unsigned>(1))}, {}, maxExecutionCycles),
					addVector({}, 1, std::vector<unsigned>{0x01020304})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/OpenCL-CTS/sub_sat.cl", "test_sub_sat_int",
					{toParameter(std::vector<int>{std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), std::numeric_limits<int>::min(), std::numeric_limits<int>::max()}),
							toParameter(std::vector<int>{1, -1, std::numeric_limits<int>::max(), std::numeric_limits<int>::min()}), toParameter(std::vector<int>(4))
					}, toConfig(4, 1, 1, 1, 1, 1), maxExecutionCycles),
					addVector({}, 2, std::vector<int>{std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), std::numeric_limits<int>::min(), std::numeric_limits<int>::max()})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/OpenCL-CTS/uchar_compare.cl", "test_select",
					{toParameter(std::vector<unsigned>{0x01020304}), toParameter(std::vector<unsigned>{0x04020301}), toParameter(std::vector<unsigned>(1))}, {}, maxExecutionCycles),
					addVector({}, 2, std::vector<unsigned>{0x04020301})
				)
		};

		//TODO NVIDIA/matrixMul, NVIDIA/transpose, OpenCLIPP/Arithmetic, OpenCLIPP/Logic, OpenCLIPP/Thresholding, test_signedness, test_shuffle, local_private_storage

		static std::vector<std::pair<EmulationData, std::map<uint32_t, std::vector<uint32_t>>>> floatTests = {
//				std::make_pair(EmulationData(VC4C_ROOT_PATH "example/test_instructions.cl", "test_instructions",
//					{std::make_pair(2, Optional<std::vector<uint32_t>>{}), std::make_pair(4, Optional<std::vector<uint32_t>>{}), std::make_pair(bit_cast<float, uint32_t>(2.0f), Optional<std::vector<uint32_t>>{}), std::make_pair(bit_cast<float, uint32_t>(4.0f), Optional<std::vector<uint32_t>>{}), std::make_pair(0, std::vector<uint32_t>(32)), std::make_pair(0, std::vector<uint32_t>(32))}, {}, maxExecutionCycles),
//					addVector({}, 5, std::vector<float>{6.0f, -2.0f, 8.0f, 0.5f, 4.0f, 2.0f, 2.0f})
//				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/bugs/30_local_memory.cl", "dot3",
					{toParameter(toRange<float>(0.0f, 20.0f)), toParameter(std::vector<float>{0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f}), toParameter(std::vector<float>(24)), toParameter(std::vector<float>(16))},
					toConfig(10, 1, 1, 2, 1, 1), maxExecutionCycles),
					addVector({}, 2, std::vector<float>{0.1f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 2.1f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.8f, 1.9f})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/bugs/30_local_memory.cl", "dot3_local",
					{toParameter(toRange<float>(0.0f, 20.0f)), toParameter(std::vector<float>{0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f}), toParameter(std::vector<float>(24))},
					toConfig(10, 1, 1, 2, 1, 1), maxExecutionCycles),
					addVector({}, 2, std::vector<float>{0.1f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 2.1f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.8f, 1.9f})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/bugs/33_floating_point_folding.cl", "add_redundancy",
					{toParameter(std::vector<float>{5.0f})}, {}, maxExecutionCycles),
					addVector({}, 0, std::vector<float>{5.0f})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/bugs/33_floating_point_folding.cl", "mul_redundancy",
					{toParameter(std::vector<float>{5.0f})}, {}, maxExecutionCycles),
					addVector({}, 0, std::vector<float>{0.0f})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/NVIDIA/VectorAdd.cl", "VectorAdd",
					{toParameter(toRange<float>(0.0f, 18.0f)), toParameter(toRange<float>(0.0f, 18.0f)), toParameter(std::vector<uint32_t>(20)), toScalarParameter(16)},
					toConfig(12, 1, 1, 2, 1, 1), maxExecutionCycles),
					addVector({}, 2, std::vector<float>{0.0f, 2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f, 14.0f, 16.0f, 18.0f, 20.0f, 22.0f, 24.0f, 26.0f, 28.0f, 30.0f, 0.0f})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/test_vector.cl", "test_arithm",
					{toScalarParameter(2.0f), toParameter(toRange(1.0f, 17.0f)), toParameter(std::vector<float>(16))}, {}, maxExecutionCycles),
					addVector({}, 2, toRange(3.0f, 19.0f))
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/test_vectorization.cl", "test1",
					{toParameter(std::vector<float>(1000)), toParameter(std::vector<float>(1000))}, {}, maxExecutionCycles),
					addVector(addVector({}, 0, toRange(-0.0f, -14.0f, -1.0f)), 1, toRange(-0.0f, -14.0f, -1.0f))
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/test_vectorization.cl", "test2",
					{toParameter(toRange<float>(1.0f, 10.0f)), toParameter(toRange<float>(1.0f, 10.0f)), toScalarParameter(7.0f), toScalarParameter(1u), toScalarParameter(6u)}, {}, maxExecutionCycles),
					addVector({}, 0, std::vector<float>{1.0f, 18.0f, 30.0f, 44.0f, 60.0f, 78.0f, 7.0f, 8.0f, 9.0f})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/test_vectorization.cl", "test3",
					{toParameter(toRange<float>(1.0f, 801.0f)), toParameter(toRange<float>(1.0f, 801.0f)), toScalarParameter(7.0f)}, {}, maxExecutionCycles),
					addVector({}, 0, std::vector<float>{8.0f, 18.0f, 30.0f, 44.0f, 60.0f, 78.0f, 98.0f, 120.0f, 144.0f})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/test_vectorization.cl", "test5",
					{toParameter(std::vector<float>(1024))}, {}, maxExecutionCycles * 2),
					addVector({}, 0, toRange<float>(0.0f, 1024.0f))
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/OpenCL-CTS/clamp.cl", "test_clamp",
					{toParameter(std::vector<float>{17.0f, 0.0f, 3.0f}), toParameter(std::vector<float>{1.0f, 1.0f, 1.0f}), toParameter(std::vector<float>{5.0f, 5.0f, 5.0f}), toParameter(std::vector<float>(3))},
					toConfig(3, 1, 1, 1, 1, 1), maxExecutionCycles),
					addVector({}, 3, std::vector<float>{5.0f, 1.0f, 3.0f})
				),
				std::make_pair(EmulationData(VC4C_ROOT_PATH "testing/OpenCL-CTS/cross_product.cl", "test_cross",
					{toParameter(std::vector<float>{1.0f, 2.0f, 3.0f}), toParameter(std::vector<float>{3.0f, 4.0f, 5.0f}), toParameter(std::vector<float>(3))}, {}, maxExecutionCycles),
					addVector({}, 2, std::vector<float>{-2.0f, 4.0f, -2.0f})
				)
		};
	} /* namespace test */
} /* namespace vc4c */

#endif /* VC4C_TEST_CASES_H */
