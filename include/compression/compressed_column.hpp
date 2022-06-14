#pragma once

#include <core/column_base_typed.hpp>

namespace CoGaDB
{

    /*!
     *
     *
     *  \brief This class represents a compressed column with type T, is the base class for all compressed typed
     * column classes and allows a uniform handling of compressed columns of a certain type T. \details   This class is
     * intended to be a base class, so it has a virtual destructor and pure virtual methods, which need to be
     * implemented in a derived class. \author    Sebastian Breß \version   0.2 \date      2013 \copyright GNU LESSER
     * GENERAL PUBLIC LICENSE - Version 3, http://www.gnu.org/licenses/lgpl-3.0.txt
     */
    template<class T>
    class CompressedColumn : public ColumnBaseTyped<T>
    {
      public:
        /***************** constructors and destructor *****************/
        //inherit constructor
        using ColumnBaseTyped<T>::ColumnBaseTyped;

        ~CompressedColumn() override = default;

        [[nodiscard]] virtual bool isMaterialized() const noexcept final;

        [[nodiscard]] virtual bool isCompressed() const noexcept final;
    };

    /***************** Start of Implementation Section ******************/

    template<class T>
    bool CompressedColumn<T>::isMaterialized() const noexcept
    {
        return false;
    }

    template<class T>
    bool CompressedColumn<T>::isCompressed() const noexcept
    {
        return true;
    }

    /***************** End of Implementation Section ******************/

} // namespace CoGaDB
