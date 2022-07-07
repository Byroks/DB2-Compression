
/*! \example dictionary_compressed_column.hpp
 * This is an example of how to implement a compression technique in our framework. One has to inherit from the abstract
 * base class CoGaDB::CompressedColumn and implement the pure virtual methods.
 */

#pragma once

#include "compressed_column.hpp"
#include "core/global_definitions.hpp"
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <iterator>
#include <map>

namespace CoGaDB
{

    /*!
     *  \brief     This class represents a dictionary compressed column with type T, is the base class for all
     * compressed typed column classes.
     */
    template <class T>
    class DictionaryCompressedColumn final : public CompressedColumn<T>
    {
    public:
        /***************** constructors and destructor *****************/
        explicit DictionaryCompressedColumn(const std::string &name);

        ~DictionaryCompressedColumn() final;

        void insert(const ColumnType &new_Value) final;

        void insert(const T &new_value) final;

        template <typename InputIterator>
        void insert(InputIterator first, InputIterator last);

        void update(TID tid, const ColumnType &new_value) final;

        void update(PositionList &tid, const ColumnType &new_value) final;

        void remove(TID tid) final;

        // assumes tid list is sorted ascending
        void remove(PositionList &tid) final;

        void clearContent() final;

        ColumnType get(TID tid) final;

        // virtual const std::any* const getRawData()=0;
        std::string print() const noexcept final;

        [[nodiscard]] size_t size() const noexcept final;

        [[nodiscard]] size_t getSizeInBytes() const noexcept final;

        [[nodiscard]] virtual std::unique_ptr<ColumnBase> copy() const;

        void store(const std::string &path) final;
        void load(const std::string &path) final;

        T operator[](int idx) final;

        /**
         * @brief Serialization method called by Cereal. Implement this method in your compressed columns to get serialization working.
         */
        template <class Archive>
        void serialize(Archive &archive)
        {
            archive(dictionary, table); // serialize things by passing them to the archive
        }

    private:
        std::vector<T> dictionary;
        std::vector<size_t> table;
    };

    /***************** Start of Implementation Section ******************/

    template <class T>
    DictionaryCompressedColumn<T>::DictionaryCompressedColumn(const std::string &name) : CompressedColumn<T>(name), dictionary(), table() {}

    template <class T>
    DictionaryCompressedColumn<T>::~DictionaryCompressedColumn() = default;

    template <class T>
    void DictionaryCompressedColumn<T>::insert(const ColumnType &newRecord)
    {
        const T &newRecordValue = std::get<T>(newRecord);
        insert(newRecordValue);
    }

    template <class T>
    void DictionaryCompressedColumn<T>::insert(const T &newRecord)
    {

        // search dictionary for existing record
        for (size_t i = 0; i < dictionary.size(); i++)
        {
            auto &record = dictionary[i];
            if (newRecord == record)
            {
                // if record exists, insert dictonary index into table
                table.push_back(i);
                return;
            }
        }

        // if new record, insert into dictonary and then insert index into table
        size_t newIdx = dictionary.size();
        dictionary.push_back(newRecord);
        table.push_back(newIdx);
    }

    template <typename T>
    template <typename InputIterator>
    void DictionaryCompressedColumn<T>::insert(InputIterator start, InputIterator end)
    {
        for (InputIterator i = start; i < end; ++i)
        {
            insert(*i);
        }
    }

    template <class T>
    ColumnType DictionaryCompressedColumn<T>::get(TID tid)
    {
        return {operator[](tid)};
    }

    template <class T>
    std::string DictionaryCompressedColumn<T>::print() const noexcept
    {
        std::stringstream output;

        output << this->name_ << "(" << size() << ")" << std::endl;
        for (auto const &key : table)
        {
            output << "\t" << key << ": " << dictionary[key] << std::endl;
        }

        return output.str();
    }

    template <class T>
    size_t DictionaryCompressedColumn<T>::size() const noexcept
    {
        return table.size();
    }

    template <class T>
    std::unique_ptr<ColumnBase> DictionaryCompressedColumn<T>::copy() const
    {
        return std::make_unique<DictionaryCompressedColumn<T>>(*this);
    }

    template <class T>
    void DictionaryCompressedColumn<T>::update(TID tid, const ColumnType &newRecord)
    {
        const T &newRecordValue = std::get<T>(newRecord);

        // search dictionary for existing record
        for (size_t i = 0; i < dictionary.size(); i++)
        {
            auto &record = dictionary[i];
            // if record exists, change reference in table (dictionary index)
            if (newRecordValue == record)
            {
                table[tid] = i;
                return;
            }
        }

        // if not, insert new reference
        size_t newIdx = dictionary.size();
        dictionary.push_back(newRecordValue);
        table[tid] = newIdx;
    }

    template <class T>
    void DictionaryCompressedColumn<T>::update(PositionList &positions, const ColumnType &data)
    {
        for (auto &tid : positions)
        {
            update(tid, data);
        }
    }

    template <class T>
    void DictionaryCompressedColumn<T>::remove(TID tid)
    {
        // remove reference from table with offset tid
        // we don't remove record from dictionary because if so we would have to shift the table indices
        // furthermore it's possible that the record is used at another index in table and if not
        // it's possible it will be used again in the future
        table.erase(table.begin() + tid);
    }

    template <class T>
    void DictionaryCompressedColumn<T>::remove(PositionList &positions)
    {
        for (auto &tid : positions)
        {
            remove(tid);
        }
    }

    template <class T>
    void DictionaryCompressedColumn<T>::clearContent()
    {
        table.clear();
        dictionary.clear();
    }

    template <class T>
    void DictionaryCompressedColumn<T>::store(const std::string &path_)
    {
        std::string path(path_);
        path += this->name_;

        std::ofstream outfile(path.c_str(), std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);
        assert(outfile.is_open());
        cereal::PortableBinaryOutputArchive oarchive(outfile); // Create an output archive
        oarchive(*this);
    }

    template <class T>
    void DictionaryCompressedColumn<T>::load(const std::string &path_)
    {
        std::string path(path_);
        path += this->name_;

        std::ifstream infile(path.c_str(), std::ifstream::binary | std::ifstream::in);
        cereal::PortableBinaryInputArchive ia(infile);
        ia(*this);
    }

    template <class T>
    T DictionaryCompressedColumn<T>::operator[](const int idx)
    {
        size_t dictIdx = table[idx];
        return dictionary[dictIdx];
    }

    template <class T>
    size_t DictionaryCompressedColumn<T>::getSizeInBytes() const noexcept
    {
        return table.size() * sizeof(size_t) + dictionary.size() * sizeof(T);
    }

    /***************** End of Implementation Section ******************/

} // namespace CoGaDB
