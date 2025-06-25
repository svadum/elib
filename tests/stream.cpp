#include <catch2/catch_test_macros.hpp>
#include <elib/stream.h>

TEST_CASE("Data stream: constructors", "[elib::data::stream]") {
    { // OutputDataStream: std::array
        std::array<std::uint8_t, 0> data{};
        elib::data::OutputStream stream(data);

        REQUIRE(stream.pos() == 0);
        REQUIRE_FALSE(stream.overflow());

        // Since OutputDataStream does not support reading, we don't test `read` here
    }

    { // OutputDataStream: c array
        std::uint8_t data[1]{0xAA};
        elib::data::OutputStream stream(data);

        REQUIRE(stream.pos() == 0);
        REQUIRE_FALSE(stream.overflow());

        // Since OutputDataStream does not support reading, we don't test `read` here
    }

    { // OutputDataStream: pointer + size
        std::uint8_t data[1]{0xAA};
        elib::data::OutputStream stream(data, sizeof(data));

        REQUIRE(stream.pos() == 0);
        REQUIRE_FALSE(stream.overflow());

        // Since OutputDataStream does not support reading, we don't test `read` here
    }

    { // OutputDataStream: larger integer
        std::array<std::uint32_t, 1> data{0xABCDEFAB};
        elib::data::OutputStream stream(data);

        REQUIRE(stream.pos() == 0);
        REQUIRE_FALSE(stream.overflow());

        // Since OutputDataStream does not support reading, we don't test `read` here
    }

    { // InputDataStream: std::array
        std::array<std::uint8_t, 1> data{0xAA};
        elib::data::InputStream stream(data);

        REQUIRE(stream.pos() == 0);
        REQUIRE_FALSE(stream.overflow());
        REQUIRE(stream.read<std::uint8_t>() == std::uint8_t{0xAA});
        REQUIRE(stream.pos() == 1);
    }

    { // InputDataStream: c array
        std::uint8_t data[1]{0xAA};
        elib::data::InputStream stream(data);

        REQUIRE(stream.pos() == 0);
        REQUIRE_FALSE(stream.overflow());
        REQUIRE(stream.read<std::uint8_t>() == std::uint8_t{0xAA});
        REQUIRE(stream.pos() == 1);
    }

    { // InputDataStream: pointer + size
        std::uint8_t data[1]{0xAA};
        elib::data::InputStream stream(data, sizeof(data));

        REQUIRE(stream.pos() == 0);
        REQUIRE_FALSE(stream.overflow());
        REQUIRE(stream.read<std::uint8_t>() == std::uint8_t{0xAA});
        REQUIRE(stream.pos() == 1);
    }

    { // InputDataStream: larger integer (std::array)
        constexpr std::uint32_t expected{0xAABBCCDD};
        std::array<decltype(expected), 1> data{expected};
        elib::data::InputStream stream(data);

        REQUIRE(stream.pos() == 0);
        REQUIRE_FALSE(stream.overflow());
        REQUIRE(stream.read<std::decay_t<decltype(expected)>>() == expected);
        REQUIRE(stream.pos() == sizeof(expected));
    }

    { // InputDataStream: larger integer (C array)
        constexpr std::uint32_t expected{0xAABBCCDD};
        decltype(expected) data[1]{expected};
        elib::data::InputStream stream(data);

        REQUIRE(stream.pos() == 0);
        REQUIRE_FALSE(stream.overflow());
        REQUIRE(stream.read<std::decay_t<decltype(expected)>>() == expected);
        REQUIRE(stream.pos() == sizeof(expected));
    }
}

TEST_CASE("Data stream: read operations", "[elib::data::stream]") {
    { // byte by byte
        std::array<std::uint8_t, 5> data{0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
        elib::data::InputStream stream{data};

        for (const auto byte : data)
            REQUIRE(stream.read<std::uint8_t>() == byte);

        // trigger overflow
        REQUIRE(stream.read<std::uint8_t>() == std::uint8_t{});
        REQUIRE(stream.overflow());

        // reset overflow
        REQUIRE(stream.seek(0));
        REQUIRE_FALSE(stream.overflow());

        std::uint8_t readByte{};
        for (const auto byte : data)
        {
            stream >> readByte;
            REQUIRE(readByte == byte);
        }

        REQUIRE_FALSE(stream.overflow());
    }

    {
        auto test_read_for = [](auto&& data)
        {
            using DataType = std::remove_reference_t<std::remove_const_t<decltype(data)>>;
            constexpr auto dataSize = sizeof(data);

            elib::data::InputStream stream(reinterpret_cast<std::uint8_t*>(&data), sizeof(data));

            REQUIRE(stream.read<DataType>() == data);
            REQUIRE(stream.pos() == dataSize);
            REQUIRE_FALSE(stream.overflow());

            REQUIRE(stream.seek(0));

            DataType readData{};
            stream >> readData;

            REQUIRE(readData == data);
            REQUIRE(stream.pos() == dataSize);
            REQUIRE_FALSE(stream.overflow());
        };

        test_read_for(std::uint8_t(0xAA));
        test_read_for(std::uint16_t(0xAABB));
        test_read_for(std::uint32_t(0xAABBCCDD));
        test_read_for(std::uint64_t(0xAABBCCDDDDCCBBAA));

        test_read_for(std::int8_t(0xAA));
        test_read_for(std::int16_t(0xAABB));
        test_read_for(std::int32_t(0xAABBCCDD));
        test_read_for(std::int64_t(0xAABBCCDDDDCCBBAA));

        test_read_for(float(12345.54321f));
        test_read_for(double(12345.54321));
    }

    {
        std::array<std::uint8_t, 5> data{0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
        elib::data::InputStream stream(data);

        decltype(data) readData{};

        stream >> readData;

        REQUIRE(data == readData);
        REQUIRE(stream.pos() == data.size());
        REQUIRE_FALSE(stream.overflow());
    }

    {
        std::uint8_t data[5]{0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
        elib::data::InputStream stream(data);

        decltype(data) readData{};

        stream >> readData;

        REQUIRE(std::memcmp(data, readData, std::size(data)) == 0);
        REQUIRE(stream.pos() == std::size(data));
        REQUIRE_FALSE(stream.overflow());
    }
}

TEST_CASE("Data stream: write operations", "[elib::data::stream]") {
    { // write in empty
        std::array<std::uint8_t, 0> data{};
        elib::data::OutputStream stream(data);

        stream << std::uint8_t{};
        REQUIRE(stream.overflow());
        REQUIRE(stream.pos() == 0);

        stream.seek(0);
        REQUIRE(stream.pos() == 0);
        REQUIRE_FALSE(stream.overflow());

        stream << std::array<std::uint8_t, 1>{};
        REQUIRE(stream.overflow());
        REQUIRE(stream.pos() == 0);

        stream.seek(0);
        REQUIRE(stream.pos() == 0);
        REQUIRE_FALSE(stream.overflow());

        std::uint8_t carray[1]{};
        stream << carray;

        REQUIRE(stream.overflow());
        REQUIRE(stream.pos() == 0);
    }

    { // byte by byte
        std::array<std::uint8_t, 5> data{};
        std::array<std::uint8_t, 5> expectedData{0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
        elib::data::OutputStream stream{data};

        for (const auto byte : expectedData)
            stream << byte;

        REQUIRE(data == expectedData);
        REQUIRE(stream.pos() == expectedData.size());
        REQUIRE_FALSE(stream.overflow());

        // trigger overflow
        stream << std::uint8_t{};
        REQUIRE(data == expectedData); // NOTE: data must remain unchanged
        REQUIRE(stream.overflow());    // and overflow flag set
    }

    {
        auto test_write_for = [](auto&& expectedData)
        {
            using DataType = std::remove_reference_t<std::remove_const_t<decltype(expectedData)>>;
            constexpr auto dataSize = sizeof(expectedData);

            DataType data{};
            elib::data::OutputStream stream(reinterpret_cast<std::uint8_t*>(&data), sizeof(data));

            stream << expectedData;

            REQUIRE(data == expectedData);
            REQUIRE(stream.pos() == dataSize);
            REQUIRE_FALSE(stream.overflow());

            stream << DataType{};

            REQUIRE(data == expectedData);
            REQUIRE(stream.overflow());
        };

        test_write_for(std::uint8_t(0xAA));
        test_write_for(std::uint16_t(0xAABB));
        test_write_for(std::uint32_t(0xAABBCCDD));
        test_write_for(std::uint64_t(0xAABBCCDDDDCCBBAA));

        test_write_for(std::int8_t(0xAA));
        test_write_for(std::int16_t(0xAABB));
        test_write_for(std::int32_t(0xAABBCCDD));
        test_write_for(std::int64_t(0xAABBCCDDDDCCBBAA));

        test_write_for(float(12345.54321f));
        test_write_for(double(12345.54321));
    }

    {
        std::array<std::uint8_t, 5> expectedData{0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
        decltype(expectedData) data{};
        elib::data::OutputStream stream{data};

        stream << expectedData;

        REQUIRE(data == expectedData);
        REQUIRE(stream.pos() == expectedData.size());
        REQUIRE_FALSE(stream.overflow());

        // trigger overflow
        stream << std::uint8_t{};
        REQUIRE(data == expectedData); // NOTE: data must remain unchanged
        REQUIRE(stream.overflow());    // and overflow flag set
    }

    {
        std::uint8_t expectedData[5]{0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
        decltype(expectedData) data{};
        elib::data::OutputStream stream{data};

        stream << expectedData;

        REQUIRE(std::memcmp(data, expectedData, std::size(expectedData)) == 0);
        REQUIRE(stream.pos() == std::size(expectedData));
        REQUIRE_FALSE(stream.overflow());

        // trigger overflow
        stream << std::uint8_t{};
        REQUIRE(std::memcmp(data, expectedData, std::size(expectedData)) == 0); // NOTE: data must remain unchanged
        REQUIRE(stream.overflow());    // and overflow flag set
    }
}
