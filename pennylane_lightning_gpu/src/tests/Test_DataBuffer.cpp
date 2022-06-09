
#include <algorithm>
#include <complex>
#include <iostream>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch.hpp>

#include "DevTag.hpp"
#include "DataBuffer.hpp"

#include <cuComplex.h> // cuDoubleComplex
#include <cuda.h>

#include "TestHelpers.hpp"

using namespace Pennylane;
using namespace CUDA;

namespace {
namespace cuUtil = Pennylane::CUDA::Util;
} // namespace


/**
 * @brief Tests the constructability of the DataBuffer class.
 *
 */
TEMPLATE_TEST_CASE("DataBuffer::DataBuffer",
                   "[DataBuffer]", char, int, unsigned int, long, float, double, float2, double2) {
    SECTION("DataBuffer<GPUDataT>{std::size_t, int, cudaStream_t, bool}"){
        REQUIRE(std::is_constructible<DataBuffer<TestType, int>,
                                      std::size_t, int, cudaStream_t, bool>::value);
    }
    SECTION("DataBuffer<GPUDataT>{std::size_t, const DevTag<int>&, bool}"){
        REQUIRE(std::is_constructible<DataBuffer<TestType, int>,
                                      std::size_t, const DevTag<int>&, bool>::value);
    }
    SECTION("DataBuffer<GPUDataT>{std::size_t, DevTag<int>&&, bool}"){
        REQUIRE(std::is_constructible<DataBuffer<TestType, int>,
                                      std::size_t, DevTag<int>&&, bool>::value);
    }
    SECTION("DataBuffer<GPUDataT>=default non-constructable"){
        REQUIRE_FALSE(std::is_default_constructible<DataBuffer<TestType, int>>::value);
    }
}

TEMPLATE_TEST_CASE("DataBuffer::memory allocation",
                   "[DataBuffer]", char, int, unsigned int, long, float, double, float2, double2) {
    SECTION("Allocate buffer memory = true"){
        DataBuffer<TestType, int> data_buffer1{8, 0, 0, true};
        CHECK(data_buffer1.getData() != nullptr);
        CHECK(data_buffer1.getLength() == 8);
        CHECK(data_buffer1.getStream() == 0);
        CHECK(data_buffer1.getDevice() == 0);
    }
    SECTION("Allocate buffer memory = false"){
        DataBuffer<TestType, int> data_buffer1{7, 0, 0, false};
        CHECK(data_buffer1.getData() == nullptr);
        CHECK(data_buffer1.getLength() == 7);
        CHECK(data_buffer1.getStream() == 0);
        CHECK(data_buffer1.getDevice() == 0);
    }
}

TEMPLATE_TEST_CASE("DataBuffer::&operator=",
                   "[DataBuffer]", char, int, unsigned int, long, float, double, float2, double2) {
    SECTION("Copy assignment operator"){
        DataBuffer<TestType, int> data_buffer1{6, 0, 0, true};
        std::vector<TestType> host_data_in(6, {1});
        std::vector<TestType> host_data_out(6, {0});
        data_buffer1.CopyHostDataToGpu(host_data.data(), host_data.size(), false);
        DataBuffer<TestType, int> data_buffer2 = data_buffer1;
        data_buffer2.CopyGpuDataToHost(host_data_out.data(), 6, false);
        CHECK(host_data_in == host_data_out);
        CHECK(data_buffer1.getLength() == data_buffer2.getLength());
        CHECK(data_buffer1.getData() != data_buffer2.getData()); // Ptrs should not refer to same block
    }
    SECTION("Move assignment operator"){
        DataBuffer<TestType, int> data_buffer1{6, 0, 0, true};
        std::vector<TestType> host_data_in(6, {1});
        std::vector<TestType> host_data_out(6, {0});
        data_buffer1.CopyHostDataToGpu(host_data.data(), host_data.size(), false);
        DataBuffer<TestType, int> data_buffer2 = std::move(data_buffer1);
        data_buffer2.CopyGpuDataToHost(host_data_out.data(), 6, false);
        CHECK(host_data_in == host_data_out);
        CHECK(data_buffer1.getLength() == 0);
        CHECK(data_buffer1.getData() == nullptr); // Ptrs should not refer to same block
        CHECK(data_buffer1.getLength() != data_buffer2.getLength());
        CHECK(data_buffer1.getData() != data_buffer2.getData()); // Ptrs should not refer to same block
    }
}