/*
 * File:   unittest.hpp
 * Author: ameister
 *
 * Created on 3. Juni 2016, 15:59
 */

#pragma once

#include "catch2/catch.hpp"
#include <compression/compressed_column.hpp>
#include <core/base_column.hpp>
#include <core/column.hpp>
#include <core/column_base_typed.hpp>
#include <core/global_definitions.hpp>
#include <random>
#include <string>

using namespace CoGaDB;

constexpr static auto SEED = 0;
static auto gen = std::mt19937(SEED);

template<typename T>
T get_rand_value() {
    return gen();
}

template<>
int get_rand_value() {
    auto dist = std::uniform_int_distribution(0, 100);
    return dist(gen);
}

template<>
float get_rand_value() {
    auto dist = std::uniform_real_distribution<float>(0, 100);
    return dist(gen);
}

template<>
std::string get_rand_value() {
    std::string s;
    auto characters = std::uniform_int_distribution('a', 'z');

    std::generate_n(std::back_inserter(s), 10, [&characters]() { return characters(gen); });

    return s;
}

template<class T>
void fill_column(ColumnBaseTyped<T> &col, std::vector<T> &reference_data) {
    for (unsigned int i = 0; i < reference_data.size(); i++) {
        reference_data[i] = get_rand_value<T>();
        col.insert(reference_data[i]);
    }
}

template<class Column>
class ColumnComparator : public Catch::MatcherBase<Column> {
    std::vector<typename Column::value_type> ref_data;

public:
    explicit ColumnComparator(decltype(ref_data) &reference_data) : ref_data(reference_data) {}

    // Performs the test for this matcher
    bool match(Column const &col) const override {
        if (ref_data.size() != col.size()) {
            std::cerr << "Size mismatch!" << std::endl;
            return false;
        }
        for (unsigned int i = 0; i < ref_data.size(); i++) {
            typename Column::value_type col_value = const_cast<Column &>(col)[i];
            if (ref_data[i] != col_value) {
                std::cerr << "Fatal Error! In Unittest: read invalid data" << std::endl;
                std::cerr << "Column: '" << col.getName() << "' TID: '" << i << "' Expected Value: '" << ref_data[i]
                          << "' Actual Value: '" << col_value << "'" << std::endl;
                return false;
            }
        }
        return true;
    }

    // Produces a string describing what this matcher does. It should
    // include any provided data (the begin/ end in this case) and
    // be written as if it were stating a fact (in the output it will be
    // preceded by the value under test).
    [[nodiscard]] std::string describe() const override {
        std::ostringstream ss;
        ss << "column equals reference data";
        return ss.str();
    }
};

// The builder function
template<class Column>
inline ColumnComparator<Column> isEqual(std::vector<typename Column::value_type> &ref_data) {
    return ColumnComparator<Column>(ref_data);
}

template<typename ValueType>
AttributeType getAttributeType() {
    return INT;
}

template<>
AttributeType getAttributeType<int>() {
    return INT;
}
template<>
AttributeType getAttributeType<float>() {
    return FLOAT;
}

template<>
AttributeType getAttributeType<std::string>() {
    return VARCHAR;
}

template<typename ValueType>
std::string getAttributeString() {
    return "unknown column";
}

template<>
std::string getAttributeString<int>() {
    return "int column";
}
template<>
std::string getAttributeString<float>() {
    return "float column";
}

template<>
std::string getAttributeString<std::string>() {
    return "string column";
}