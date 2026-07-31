#include <catch2/catch_test_macros.hpp>
#include <elib/stream.h>
#include <elib/data/stream_ext.h>
#include <string>

TEST_CASE("elib::data: extension write operations", "[data][stream][extensions]") {

    SECTION("std::string_view") {
        std::array<std::uint8_t, 10> buffer{};
        elib::data::output_stream stream(buffer);

        std::string_view text = "TEST";
        stream << text;

        REQUIRE(stream.pos() == 4);
        REQUIRE_FALSE(stream.overflow());
        REQUIRE(buffer[0] == 'T');
        REQUIRE(buffer[1] == 'E');
        REQUIRE(buffer[2] == 'S');
        REQUIRE(buffer[3] == 'T');

        // Overflow test
        stream << std::string_view("12345678");
        REQUIRE(stream.overflow());
    }

    SECTION("elib::array") {
        std::array<std::uint8_t, 10> buffer{};
        elib::data::output_stream stream(buffer);

        elib::array<std::uint16_t, 5> custom_arr; // Capacity 5[cite: 2]
        custom_arr.push_back(0xAABB);
        custom_arr.push_back(0xCCDD);

        stream << custom_arr;

        // 2 elements * 2 bytes each = 4 bytes written
        REQUIRE(stream.pos() == 4);
        REQUIRE_FALSE(stream.overflow());
    }

    SECTION("elib::span") {
        std::array<std::uint8_t, 10> buffer{};
        elib::data::output_stream stream(buffer);

        std::array<std::uint32_t, 2> source_data{0x11223344, 0x55667788};
        elib::span<std::uint32_t> sp(source_data); // Uses span-lite mapping[cite: 3]

        stream << sp;

        // 2 elements * 4 bytes each = 8 bytes written
        REQUIRE(stream.pos() == 8);
        REQUIRE_FALSE(stream.overflow());
    }
}

TEST_CASE("elib::data: extension read operations", "[data][stream][extensions]") {

    SECTION("elib::array") {
        std::array<std::uint8_t, 4> buffer{0xAA, 0xBB, 0xCC, 0xDD};
        elib::data::input_stream stream(buffer);

        elib::array<std::uint8_t, 5> read_arr; // Capacity 5[cite: 2]
        // Pre-fill to set size[cite: 2], as our implementation reads into existing elements
        read_arr.push_back(0x00);
        read_arr.push_back(0x00);
        read_arr.push_back(0x00);
        read_arr.push_back(0x00);

        stream >> read_arr;

        REQUIRE(stream.pos() == 4);
        REQUIRE_FALSE(stream.overflow());
        REQUIRE(read_arr[0] == 0xAA);
        REQUIRE(read_arr[3] == 0xDD);
    }

    SECTION("elib::span") {
        std::array<std::uint8_t, 8> buffer{0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
        elib::data::input_stream stream(buffer);

        std::array<std::uint16_t, 4> dest_data{};
        elib::span<std::uint16_t> sp(dest_data); // Dynamic extent[cite: 3]

        stream >> sp;

        REQUIRE(stream.pos() == 8);
        REQUIRE_FALSE(stream.overflow());
        // Validation relies on endianness; assuming direct copy matches memory footprint layout
        // consistent with stream_base's memcpy behavior[cite: 1].
    }
}

TEST_CASE("elib::data: extension remaining data", "[data][stream][extensions]") {
    std::array<int, 3> integers{};

    SECTION("output")
    {
        elib::data::output_stream stream{integers};

        int idx = 0;
        int pos = 1;
        for (; idx < static_cast<int>(integers.size()); idx++, pos++)
        {
            stream << pos;

            REQUIRE(integers[idx] == static_cast<int>(pos));

            auto remaining  = elib::data::remaining(stream);
            REQUIRE(static_cast<void*>(remaining.data()) == static_cast<void*>(integers.data() + pos));
            REQUIRE(remaining.size() / sizeof(int) == integers.size() - pos);
        }
    }

    SECTION("input")
    {
        elib::data::input_stream stream{integers};

        int idx = 0;
        int pos = 1;
        for (; idx < static_cast<int>(integers.size()); idx++, pos++)
        {
            int read = 0;
            stream >> read;

            REQUIRE(read == integers[idx]);

            auto remaining  = elib::data::remaining(stream);
            REQUIRE(static_cast<const void*>(remaining.data()) == static_cast<const void*>(integers.data() + pos));
            REQUIRE(remaining.size() / sizeof(int) == integers.size() - pos);
        }
    }
}

TEST_CASE("elib::data: extension serialized data", "[data][stream][extensions]") {
    std::array<int, 3> integers{};

    SECTION("output")
    {
        elib::data::output_stream stream{integers};

        int idx = 0;
        int pos = 1;
        for (; idx < static_cast<int>(integers.size()); idx++, pos++)
        {
            stream << pos;

            REQUIRE(integers[idx] == static_cast<int>(pos));

            auto serialized = elib::data::serialized(stream);
            REQUIRE(static_cast<void*>(serialized.data()) == static_cast<void*>(integers.data()));
            REQUIRE(serialized.size() / sizeof(int) == static_cast<std::size_t>(pos));
        }
    }

    SECTION("input")
    {
        elib::data::input_stream stream{integers};

        int idx = 0;
        int pos = 1;
        for (; idx < static_cast<int>(integers.size()); idx++, pos++)
        {
            int read = 0;
            stream >> read;

            REQUIRE(read == integers[idx]);

            auto serialized = elib::data::serialized(stream);
            REQUIRE(static_cast<const void*>(serialized.data()) == static_cast<const void*>(integers.data()));
            REQUIRE(serialized.size() / sizeof(int) == static_cast<std::size_t>(pos));
        }
    }

}

TEST_CASE("elib::data: make_output_stream from span", "[data][stream][extensions]") {
    std::array<std::uint32_t, 2> buffer{0, 0};
    elib::span<std::uint32_t> sp(buffer); // Uses span-lite mapping

    // Create stream using the factory function
    auto stream = elib::data::make_output_stream(sp);

    // 2 elements * 4 bytes each = 8 bytes capacity
    REQUIRE(stream.pos() == 0);
    REQUIRE(stream.capacity() == 8);
    REQUIRE_FALSE(stream.overflow());

    // Verify write goes to the underlying span memory
    stream << std::uint32_t{0xAABBCCDD};
    REQUIRE(stream.pos() == 4);
    REQUIRE_FALSE(stream.overflow());
    REQUIRE(buffer[0] == 0xAABBCCDD);
}

TEST_CASE("elib::data: make_input_stream from span", "[data][stream][extensions]") {
    std::array<std::uint16_t, 3> buffer{0x1122, 0x3344, 0x5566};

    // Note: Creating a read-only span
    elib::span<const std::uint16_t> sp(buffer);

    // Create stream using the factory function
    auto stream = elib::data::make_input_stream(sp);

    // 3 elements * 2 bytes each = 6 bytes capacity
    REQUIRE(stream.pos() == 0);
    REQUIRE(stream.capacity() == 6);
    REQUIRE_FALSE(stream.overflow());

    // Verify read extracts from the underlying span memory
    std::uint16_t val{};
    stream >> val;

    REQUIRE(val == 0x1122);
    REQUIRE(stream.pos() == 2);
    REQUIRE_FALSE(stream.overflow());
}

TEST_CASE("elib::data: state inspection helpers", "[data][stream][extensions]") {
    std::array<std::uint8_t, 10> buffer{};
    auto stream = elib::data::output_stream(buffer);


    stream << std::uint32_t{0x12345678};

    SECTION("serialized() helper") {
        auto ser_span = elib::data::serialized(stream);

        REQUIRE(ser_span.size() == 4);
        REQUIRE(ser_span.data() == buffer.data());
    }

    SECTION("remaining() helper") {
        auto rem_span = elib::data::remaining(stream);

        REQUIRE(rem_span.size() == 6);
        REQUIRE(rem_span.data() == buffer.data() + 4);
    }
}