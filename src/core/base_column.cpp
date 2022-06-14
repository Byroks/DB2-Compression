#include <core/base_column.hpp>
#include <core/column.hpp>
#include <iostream>
#include <utility>

namespace CoGaDB
{

    ColumnBase::ColumnBase(std::string name) : name_(std::move(name)) {}

    ColumnBase::~ColumnBase() = default;

    std::string ColumnBase::getName() const noexcept
    {
        return name_;
    }
} // namespace CoGaDB
