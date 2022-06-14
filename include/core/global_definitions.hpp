#pragma once

#include <string>
#include <variant>
#include <utility>

namespace CoGaDB
{
    using ColumnType = std::variant<std::monostate, int, float, std::string, bool>;

    /**
     * @brief Possible attribute types supported by the system
     */
    enum AttributeType
    {
        INT = 1,
        FLOAT,
        VARCHAR,
        BOOLEAN
    };

    enum ValueComparator
    {
        LESSER,
        GREATER,
        EQUAL
    };

    enum SortOrder
    {
        ASCENDING,
        DESCENDING
    };

    enum DebugMode
    {
        quiet = 1,
        verbose = 0,
        debug = 0,
        print_time_measurement = 0
    };

    /**
     * @brief The Tuple IDentifier (TID) is the unique,numeric identifier of a tuple in a relation
     */
    using TID = unsigned int;

} // namespace CoGaDB
