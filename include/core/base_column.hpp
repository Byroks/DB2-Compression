#pragma once
// CoGaDB includes
#include <core/global_definitions.hpp>
#include <memory>
#include <vector>

namespace CoGaDB {
    /* \brief a PositionList is an STL vector of TID values*/
    using PositionList = std::vector<TID>;
    /* \brief a PositionListPair is an STL pair consisting of two PositionList objects
     *  \details This type is returned by binary operators, e.g., joins*/
    using PositionListPair = std::pair<PositionList, PositionList>;

    /*!
     *
     *
     *  \brief     This class represents a generic column, is the base class for all column classes and allows a uniform
     * handling of columns. \details   This class is intended to be a base class, so it has a virtual destructor and
     * pure virtual methods, which need to be implemented in a derived class. \author    Sebastian Breß \version   0.2
     *  \date      2013
     *  \copyright GNU LESSER GENERAL PUBLIC LICENSE - Version 3, http://www.gnu.org/licenses/lgpl-3.0.txt
     */

    class ColumnBase {
    public:
        /***************** constructors and destructor *****************/
        explicit ColumnBase(std::string name);

        virtual ~ColumnBase();
        /***************** methods *****************/
        /*! \brief appends a value new_Value to end of column throws an error if not successful*/
        virtual void insert(const ColumnType &new_Value) = 0;

        /*! \brief updates the value on position tid with a value new_Value, throws if an error occurs */
        virtual void update(TID tid, const ColumnType &new_Value) = 0;

        /*! \brief updates the values specified by the position list with a value new_Value , throws if an error occurs*/
        virtual void update(PositionList &tids, const ColumnType &new_value) = 0;

        /*! \brief deletes the value on position tid, throws if an error occurs*/
        virtual void remove(TID tid) = 0;

        /*! \brief deletes the values defined in the position list
         *  \details assumes tid list is sorted ascending, throws if an error occurs*/
        virtual void remove(PositionList &tid) = 0;

        /*! \brief deletes all values stored in the column throws am exception if an error occurs */
        virtual void clearContent() = 0;

        /*! \brief generic function for fetching a value form a column (slow)
         *  \details check whether the object is valid (e.g., when a tid is not valid, then the returned object is
         * invalid as well) \return object of type ColumnType containing the value on position tid. If tid is not valid, throws exception. */
        virtual ColumnType get(TID tid) = 0; // not const, because operator [] does not provide const return type
        // and the child classes rely on []
        /*! \brief creates a textual representation of the content of the column */
        [[nodiscard]] virtual std::string print() const noexcept = 0;

        /*! \brief returns the number of values (rows) in a column*/
        [[nodiscard]] virtual size_t size() const noexcept = 0;

        /*! \brief returns the size in bytes the column consumes in main memory*/
        [[nodiscard]] virtual size_t getSizeInBytes() const noexcept = 0;

        /*! \brief virtual copy constructor
         * \return a ColumnPtr to an exact copy of the current column*/
        [[nodiscard]] virtual std::unique_ptr<ColumnBase> copy() const = 0;
        /************ relational operations on Columns which return a PositionListPtr/PositionListPairPtr *************/
        /*! \brief sorts a column w.r.t. a SortOrder
         * \return PositionListPtr to a PositionList, which represents the result*/
        virtual PositionList sort(SortOrder order = ASCENDING) = 0;

        /*! \brief filters the values of a column according to a filter condition consisting of a comparison value and a
         * ValueComparator (=,<,>) \return PositionListPtr to a PositionList, which represents the result*/
        virtual PositionList selection(const ColumnType &value_for_comparison, ValueComparator comp) = 0;

        /*! \brief filters the values of a column in parallel according to a filter condition consisting of a comparison
         * value and a ValueComparator (=,<,>) \details the additional parameter specifies the number of threads that
         * may be used to perform the operation \return PositionListPtr to a PositionList, which represents the result*/
        virtual PositionList parallel_selection(const ColumnType &value_for_comparison,
                                                ValueComparator comp,
                                                unsigned int number_of_threads) = 0;

        /*! \brief joins two columns using the hash join algorithm
         * \return PositionListPairPtr to a PositionListPair, which represents the result*/
        virtual PositionListPair hash_join(ColumnBase &join_column) = 0;

        /*! \brief joins two columns using the sort merge join algorithm
         * \return PositionListPairPtr to a PositionListPair, which represents the result*/
        virtual PositionListPair sort_merge_join(ColumnBase &join_column) = 0;

        /*! \brief joins two columns using the nested loop join algorithm
         * \return PositionListPairPtr to a PositionListPair, which represents the result*/
        virtual PositionListPair nested_loop_join(ColumnBase &join_column) = 0;
        /***************** column algebra operations *****************/
        /*! \brief adds constant to column
         *  \details for all indices i holds the following property: B[i]=A[i]+new_Value*/
        virtual bool add(const ColumnType &new_Value) = 0;

        /*! \brief vector addition of two columns
         *  \details for all indices i holds the following property: C[i]=A[i]+B[i]*/
        virtual bool add(ColumnBase &column) = 0;

        /*! \brief subtracts constant from column
         *  \details for all indices i holds the following property: B[i]=A[i]-new_Value*/
        virtual bool minus(const ColumnType &new_Value) = 0;

        /*! \brief vector subtraction of two columns
         *  \details for all indices i holds the following property: C[i]=A[i]-B[i]*/
        virtual bool minus(ColumnBase &column) = 0;

        /*! \brief multiply constant with column
         *  \details for all indices i holds the following property: B[i]=A[i]*new_Value*/
        virtual bool multiply(const ColumnType &new_Value) = 0;

        /*! \brief multiply two columns A and B
         *  \details for all indices i holds the following property: C[i]=A[i]*B[i]*/
        virtual bool multiply(ColumnBase &column) = 0;

        /*! \brief divide values in column by a constant
         *  \details for all indices i holds the following property: B[i]=A[i]/new_Value*/
        virtual bool division(const ColumnType &new_Value) = 0;

        /*! \brief divide column A with column B
         *  \details for all indices i holds the following property: C[i]=A[i]/B[i]*/
        virtual bool division(ColumnBase &column) = 0;
        /***************** persistence operations *****************/
        /*! \brief store a column on the disc, throws if an error occurred*/
        virtual void store(const std::string &path) = 0;

        /*! \brief load column from disc
         *  \details calling load on a column that is not empty yields undefined behaviour, throws if an error occurred*/
        virtual void load(const std::string &path) = 0;

        /*! \brief use this method to determine whether the column is materialized or a Lookup Column
         * \return true in case the column is storing the plain values (without compression) and false in case the
         * column is a LookupColumn.*/

        /***************** misc operations *****************/
        [[nodiscard]] virtual bool isMaterialized() const noexcept = 0;

        /*! \brief use this method to determine whether the column is materialized or a Lookup Column
         * \return true in case the column is storing the compressed values and false otherwise.*/
        [[nodiscard]] virtual bool isCompressed() const noexcept = 0;

        /*! \brief returns attribute name of column
            \return attribute name of column*/
        [[nodiscard]] std::string getName() const noexcept;

        [[nodiscard]] virtual AttributeType getType() const = 0;

        /**
         * @brief output the column to the output stream, for example when printing the column to console
         * @param output output stream
         * @param base the column to print
         * @return a reference to the output stream for chaining
         */
        friend std::ostream &operator<<(std::ostream &output, const ColumnBase &base) {
            output << base.print();
            return output;
        }

    protected:
        /*! \brief attribute name of the column*/
        std::string name_;
    };

    /*! \brief Column factory function, creates an empty materialized column*/
    std::unique_ptr<ColumnBase> createColumn(AttributeType type, const std::string &name);
} // namespace CoGaDB