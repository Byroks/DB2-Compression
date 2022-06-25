
/*! \example dictionary_compressed_column.hpp
 * This is an example of how to implement a compression technique in our framework. One has to inherit from the abstract
 * base class CoGaDB::CompressedColumn and implement the pure virtual methods.
 */

#pragma once

#include "compressed_column.hpp"
#include "core/global_definitions.hpp"
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>
#include <iterator>

namespace CoGaDB {

    /*!
     *  \brief     This class represents a dictionary compressed column with type T, is the base class for all
     * compressed typed column classes.
     */
    template<class T>
    class RLECompressedColumn final : public CompressedColumn<T> {
    public:
        /***************** constructors and destructor *****************/
        explicit RLECompressedColumn(const std::string &name);

        ~RLECompressedColumn() final;

        void insert(const ColumnType &new_Value) final;
        void insert(const T &new_value) final;

        template<typename InputIterator>
        void insert(InputIterator first, InputIterator last);

        void update(TID tid, const ColumnType &new_value) final;

        void update(PositionList &tid, const ColumnType &new_value) final;

        void remove(TID tid) final;

        // assumes tid list is sorted ascending
        void remove(PositionList &tid) final;

        void clearContent() final;

        ColumnType get(TID tid) final;

        std::string print() const noexcept final;

        [[nodiscard]] size_t size() const noexcept final;

        [[nodiscard]] size_t getSizeInBytes() const noexcept final;

        [[nodiscard]] virtual std::unique_ptr<ColumnBase> copy() const;

        void store(const std::string &path) final;
        void load(const std::string &path) final;

        T operator[](int index) final;

        /**
         * @brief Serialization method called by Cereal. Implement this method in your compressed columns to get serialization working.
         */
        template<class Archive>
        void serialize(Archive &archive) {
            archive(values);// serialize things by passing them to the archive
        }

    private:
        typedef std::pair<uint8_t, T> Item;

        std::vector<Item> values;

        void tid_to_idx(TID tid, size_t &idx_of_run, size_t &idx_in_run);
    };

    /***************** Start of Implementation Section ******************/

    template<class T>
    RLECompressedColumn<T>::RLECompressedColumn(const std::string &name) : CompressedColumn<T>(name), values() {}

    template<class T>
    RLECompressedColumn<T>::~RLECompressedColumn() = default;

    template<class T>
    void RLECompressedColumn<T>::insert(const ColumnType &newRecord) {
        const T &new_value = std::get<T>(newRecord);
        insert(new_value);
    }

    template<class T>
    void RLECompressedColumn<T>::insert(const T &new_value) {
        size_t length = values.size();

        if (length > 0) {
            Item &most_recent_value = values[length - 1];
            if (most_recent_value.second == new_value && most_recent_value.first < UINT8_MAX - 1) {
                most_recent_value.first++;
                return;
            }
        }

        values.push_back(std::make_pair(1, new_value));
    }

    template<typename T>
    template<typename InputIterator>
    void RLECompressedColumn<T>::insert(InputIterator start, InputIterator end) {
        for (InputIterator i = start; i < end; ++i) {
            insert(*i);
        }
    }

    template<class T>
    ColumnType RLECompressedColumn<T>::get(TID id) {
        return {operator[](id)};
    }

    template<class T>
    std::string RLECompressedColumn<T>::print() const noexcept {
        std::stringstream output;

        output << this->name_ << "(" << size() << ")" << std::endl;
        for (auto const &pair: values) {
            for (size_t i = 0; i < pair.first; ++i) output << pair.second << std::endl;
        }

        return output.str();
    }

    template<class T>
    size_t RLECompressedColumn<T>::size() const noexcept {
        uint64_t size = 0;
        for (auto const &pair: values) size += pair.first;
        return size;
    }

    template<class T>
    std::unique_ptr<ColumnBase> RLECompressedColumn<T>::copy() const {
        return std::make_unique<RLECompressedColumn<T>>(*this);
    }

    template<class T>
    void RLECompressedColumn<T>::tid_to_idx(TID tid, size_t &idx_pair, size_t &idx_str) {
        TID tid_of_run_start = 0;
        idx_pair = 0;
        idx_str = 0;
        size_t len = values.size();

        while (tid_of_run_start <= tid && idx_pair < len) {
            Item &cur = values[idx_pair];
            idx_str = tid - tid_of_run_start;

            if (idx_str < cur.first) break;
            else
                tid_of_run_start += values[idx_pair++].first;// not in this run, skip
        }
    }

    template<class T>
    void RLECompressedColumn<T>::update(TID tid, const ColumnType &new_value) {
        size_t idx_pair = 0, idx_str = 0;
        tid_to_idx(tid, idx_pair, idx_str);

        auto &entry = values[idx_pair];

        auto val = std::get<T>(new_value);
        if (entry.first == 1) {
            // value only occurs once, can be replaced without problems
            entry.second = val;
        } else if (idx_str == 0) {
            // Updating part of a run decreases its length AAAABBBCCCC
            // leading value needs to be replaced
            // insert before pair {4, A}; {3, B}; {4, C}
            entry.first--;
            values.insert(values.begin() + idx_pair, std::make_pair(1, val));
        } else if (idx_str == entry.first) {
            // trailing value needs to be replaced
            // insert after pair
            entry.first--;
            values.insert(values.begin() + idx_pair, std::make_pair(1, val));
        } else {
            // value inside of string needs to be replaced
            // split pair, insert inbetween
            uint8_t len_a = idx_str, len_b = entry.first - len_a - 1;

            entry.first = len_a;
            std::vector<Item> v{std::make_pair(1, val), std::make_pair(len_b, entry.second)};
            values.insert(values.begin() + idx_pair + 1, begin(v), end(v));
        }
    }

    template<class T>
    void RLECompressedColumn<T>::update(PositionList &positions, const ColumnType &new_value) {
        for (auto &tid: positions) {
            update(tid, new_value);
        }
    }

    template<class T>
    void RLECompressedColumn<T>::remove(TID tid) {
        size_t idx_of_run = 0, idx_in_run = 0;
        tid_to_idx(tid, idx_of_run, idx_in_run);

        auto &entry = values[idx_of_run];
        if (entry.first == 1) {
            // is run
            values.erase(values.begin() + idx_of_run);
        } else {
            values[idx_of_run].first--;
        }
    }

    template<class T>
    void RLECompressedColumn<T>::remove(PositionList &positions) {
        for (auto &tid: positions) {
            remove(tid);
        }
    }

    template<class T>
    void RLECompressedColumn<T>::clearContent() {
        values.clear();
    }

    template<class T>
    void RLECompressedColumn<T>::store(const std::string &path_) {
        std::string path(path_);
        path += this->name_;

        std::ofstream outfile(path.c_str(), std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);
        assert(outfile.is_open());
        cereal::PortableBinaryOutputArchive oarchive(outfile);// Create an output archive
        oarchive(*this);
    }

    template<class T>
    void RLECompressedColumn<T>::load(const std::string &path_) {
        std::string path(path_);
        path += this->name_;

        std::ifstream infile(path.c_str(), std::ifstream::binary | std::ifstream::in);
        cereal::PortableBinaryInputArchive ia(infile);
        ia(*this);
    }


    template<class T>
    T RLECompressedColumn<T>::operator[](int idx) {
        static T t;
        for (auto &pair: values) {
            if (pair.first > idx) return pair.second;
            else
                idx -= pair.first;
        }
        return t;
    }

    template<class T>
    size_t RLECompressedColumn<T>::getSizeInBytes() const noexcept {
        return values.size() * sizeof(Item);
    }

    /***************** End of Implementation Section ******************/

}// namespace CoGaDB
