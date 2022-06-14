#define CATCH_CONFIG_MAIN
#include "core/column.hpp"

// TODO: include your compressed column implementations here
#include "compression/dictionary_compressed_column.hpp"
// #include "compression/rle_compressed_column.hpp"

#include "config.hpp"
#include "tests/utils.hpp"

#include <catch2/catch.hpp>

template<typename T>
struct Column_Test_Fixture {
    using ValueType = typename T::value_type;
    Column_Test_Fixture() : col_one(getAttributeString<ValueType>()),
                            col_two(getAttributeString<ValueType>()),
                            reference_data(100){};

    T col_one;
    T col_two;
    std::vector<ValueType> reference_data;
};

using namespace CoGaDB;

TEMPLATE_PRODUCT_TEST_CASE_METHOD(Column_Test_Fixture,
                                  "Template test case method with test types specified inside std::tuple",
                                  "[class][template]",
                                  (Column, DictionaryCompressedColumn),
                                  (int, float, std::string)) {
    using ValueType = typename Column_Test_Fixture<TestType>::ValueType;
    auto &col_one = Column_Test_Fixture<TestType>::col_one;
    auto &col_two = Column_Test_Fixture<TestType>::col_two;
    auto &reference_data = Column_Test_Fixture<TestType>::reference_data;

    /****** INSERT TEST ******/
    REQUIRE_NOTHROW(fill_column<ValueType>(col_one, reference_data));

    REQUIRE(reference_data.size() == col_one.size());
    REQUIRE_THAT(col_one, isEqual<TestType>(reference_data));

    /****** VIRTUAL COPY CONSTRUCTOR TEST ******/
    std::unique_ptr<ColumnBase> copy = col_one.copy();
    REQUIRE(copy);
    auto &cpy = dynamic_cast<TestType &>(*copy);
    REQUIRE(cpy == col_one);

    /****** UPDATE TEST ******/
    std::uniform_int_distribution dist(0, 100);
    TID tid = dist(gen);
    auto new_value = get_rand_value<ValueType>();

    reference_data[tid] = new_value;

    REQUIRE_NOTHROW(col_one.update(tid, new_value));
    REQUIRE_THAT(col_one, isEqual<TestType>(reference_data));

    /****** DELETE TEST ******/
    tid = dist(gen);

    reference_data.erase(reference_data.begin() + tid);

    REQUIRE_NOTHROW(col_one.remove(tid));
    REQUIRE_THAT(col_one, isEqual<TestType>(reference_data));

    /****** STORE AND LOAD TEST ******/
    REQUIRE_NOTHROW(col_one.store(DATA_PATH));
    col_one.clearContent();
    REQUIRE(col_one.size() == 0);

    REQUIRE_NOTHROW(col_two.load(DATA_PATH));
    REQUIRE_THAT(col_two, isEqual<TestType>(reference_data));
}
