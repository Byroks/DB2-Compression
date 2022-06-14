#pragma once

#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <core/column_base_typed.hpp>
#include <fstream>
#include <iostream>
#include <numeric>

namespace CoGaDB {

    template<typename T>
    class Column final : public ColumnBaseTyped<T> {
    public:
        /***************** constructors and destructor *****************/
        explicit Column(const std::string &name);

        ~Column() override = default;

        void insert(const ColumnType &new_value) final;

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

        [[nodiscard]] std::string print() const noexcept final;

        [[nodiscard]] size_t size() const noexcept final;

        [[nodiscard]] size_t getSizeInBytes() const noexcept final;

        [[nodiscard]] std::unique_ptr<ColumnBase> copy() const final;

        void store(const std::string &path) final;

        void load(const std::string &path) final;

        [[nodiscard]] bool isMaterialized() const noexcept final;

        [[nodiscard]] bool isCompressed() const noexcept final;

        template<class Archive>
        /**
         * @brief Serialization method called by Cereal. Implement this method in your compressed columns to get serialization working.
         */
        void serialize(Archive &archive) {
            archive(values_); // serialize things by passing them to the archive
        }

        T operator[](int index) final;

        [[maybe_unused]] std::vector<T> &getContent();

    private:
        struct Type_TID_Comparator {
            inline bool operator()(std::pair<T, TID> i, std::pair<T, TID> j) {
                return (i.first < j.first);
            }
        } type_tid_comparator;

        /*! values*/
        std::vector<T> values_;
    };

    /***************** Start of Implementation Section ******************/

    template<class T>
    [[maybe_unused]] std::vector<T> &Column<T>::getContent() {
        return values_;
    }

    template<class T>
    void Column<T>::insert(const ColumnType &new_value) {
        //will throw if types do not match
        T value = std::get<T>(new_value);
        values_.push_back(value);
    }

    template<class T>
    void Column<T>::insert(const T &new_value) {
        values_.push_back(new_value);
    }

    template<typename T>
    template<typename InputIterator>
    void Column<T>::insert(InputIterator first, InputIterator last) {
        this->values_.insert(this->values_.end(), first, last);
    }

    template<class T>
    void Column<T>::update(TID tid, const ColumnType &new_value) {
        //will throw if new_value doesn't hold type T
        T value = std::get<T>(new_value);
        values_[tid] = value;
    }

    template<class T>
    void Column<T>::update(PositionList &tids, const ColumnType &new_value) {

//will throw if new_value doesn't hold type T
        T value = std::get<T>(new_value);
        for (unsigned int tid: tids) {
            values_[tid] = value;
        }
    }

    template<class T>
    void Column<T>::remove(TID tid) {
        values_.erase(values_.begin() + tid);
    }

    template<class T>
    void Column<T>::remove(PositionList &tids) {
        for (auto rit = tids.rbegin(); rit != tids.rend(); ++rit)
            values_.erase(values_.begin() + (*rit));
    }

    template<class T>
    void Column<T>::clearContent() {
        values_.clear();
    }

    template<class T>
    ColumnType Column<T>::get(TID tid) {
        return values_.at(tid);
    }

    template<class T>
    std::string Column<T>::print() const noexcept {
        return std::accumulate(values_.cbegin(), values_.cend(), "| " + this->name_ + " |\n________________________\n",
                               [](std::string acc, const T &cur) {
                                   if constexpr(std::is_same_v<std::string, T>)
                                       return std::move(acc) + "| " + cur + " |\n";
                                   else
                                       return std::move(acc) + "| " + std::to_string(cur) + " |\n";
                               });
    }

    template<class T>
    size_t Column<T>::size() const noexcept {
        return values_.size();
    }

    template<class T>
    std::unique_ptr<ColumnBase> Column<T>::copy() const {
        return std::make_unique<Column<T>>(*this);
    }

    /***************** relational operations on Columns which return lookup tables *****************/
    template<class T>
    bool Column<T>::isMaterialized() const noexcept {
        return true;
    }

    template<class T>
    bool Column<T>::isCompressed() const noexcept {
        return false;
    }

    template<class T>
    T Column<T>::operator[](const int index) {
        return values_[index];
    }

    template<class T>
    size_t Column<T>::getSizeInBytes() const noexcept {
        return values_.capacity() * sizeof(T);
    }

    // total template specialization
    template<>
    inline size_t Column<std::string>::getSizeInBytes() const noexcept {
        return std::accumulate(values_.cbegin(), values_.cend(), 0,
                               [](size_t acc, const std::string &val) { return acc + val.size(); });
    }

    template<typename T>
    Column<T>::Column(const std::string &name) : ColumnBaseTyped<T>(name), type_tid_comparator(), values_() {

    }

    template<typename T>
    void Column<T>::load(const std::string &path_) {
        std::string path(path_);
        path += this->name_;

        std::ifstream infile(path.c_str(), std::ifstream::binary | std::ifstream::in);
        cereal::PortableBinaryInputArchive ia(infile);
        ia(*this);
    }

    template<typename T>
    void Column<T>::store(const std::string &path_) {
        std::string path(path_);
        path += this->name_;

        std::ofstream outfile(path.c_str(), std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);
        assert(outfile.is_open());
        cereal::PortableBinaryOutputArchive oarchive(outfile); // Create an output archive
        oarchive(*this);
    }

    /***************** End of Implementation Section ******************/

} // namespace CoGaDB
