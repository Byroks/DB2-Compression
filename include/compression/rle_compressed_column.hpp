#pragma once

#include <boost/serialization/utility.hpp>
#include <compression/compressed_column.hpp>

using namespace std;

namespace CoGaDB {

    template<class T>
    class RLECompressedColumn : public CompressedColumn<T> {
    public:
        /***************** constructors and destructor *****************/
        RLECompressedColumn(const std::string &name, AttributeType db_type);
        virtual ~RLECompressedColumn();

        virtual bool insert(const boost::any &new_Value);
        virtual bool insert(const T &new_value);
        template<typename InputIterator>
        bool insert(InputIterator first, InputIterator last);

        virtual bool update(TID tid, const boost::any &new_value);
        virtual bool update(PositionListPtr tid, const boost::any &new_value);

        virtual bool remove(TID tid);
        // assumes tid list is sorted ascending
        virtual bool remove(PositionListPtr tid);
        virtual bool clearContent();

        virtual const boost::any get(TID tid);
        // virtual const boost::any* const getRawData()=0;
        virtual void print() const throw();
        virtual size_t size() const throw();
        virtual unsigned int getSizeinBytes() const throw();

        virtual const ColumnPtr copy() const;

        virtual bool store(const std::string &path);
        virtual bool load(const std::string &path);

        virtual T &operator[](const int index);

    private:
        typedef pair<uint8_t, T> Item;

        vector<Item> values;

        void tid_to_idx(TID tid, size_t &idx_of_run, size_t &idx_in_run);
    };

    /***************** Start of Implementation Section ******************/

    template<class T>
    RLECompressedColumn<T>::RLECompressedColumn(
            const std::string &name, AttributeType db_type)
        : CompressedColumn<T>(name, dpe), values() {}

    template<class T>
    RLECompressedColumn<T>::~RLECompressedColumn() {}

    template<class T>
    void RLECompressedColumn<T>::tid_to_idx(TID tid, size_t &idx_of_run, size_t &idx_in_run) {
        TID tid_of_run_start = 0;
        idx_of_run = 0;
        idx_in_run = 0;
        size_t len = values.size();

        while (tid_of_run_start <= tid && idx_of_run < len) {
            Item &cur = values[idx_of_run];
            idx_in_run = tid - tid_of_run_start;

            if (idx_in_run < cur.first) break;
            else
                tid_of_run_start += values[idx_of_run++].first;// not in this run, skip
        }
    }

    template<class T>
    bool RLECompressedColumn<T>::insert(const boost::any &new_value) {
        if (new_value.empty()) return false;

        if (typeid(T) == new_value.type()) {
            return insert(boost::any_cast<T>(new_value));
        }

        return false;
    }

    template<class T>
    bool RLECompressedColumn<T>::insert(const T &new_value) {
        size_t length = values.size();

        if (length > 0) {
            Item &most_recent_value = values[length - 1];
            if (most_recent_value.second == new_value && most_recent_value.first < UINT8_MAX - 1) {
                most_recent_value.first++;
                return true;
            }
        }

        values.push_back(make_pair(1, new_value));
        return true;
    }

    template<typename T>
    template<typename InputIterator>
    bool RLECompressedColumn<T>::insert(InputIterator start, InputIterator end) {
        for (InputIterator i = start; i < end; ++i) insert(*i);
        return true;
    }

    template<class T>
    const boost::any RLECompressedColumn<T>::get(TID idx) {

        return boost::any(operator[](idx));
    }

    template<class T>
    void RLECompressedColumn<T>::print() const throw() {
        cout << this->name_ << "(" << size() << ")" << endl;
        for (auto const &pair: values) {
            for (size_t i = 0; i < pair.first; ++i) cout << pair.second << endl;
        }
    }

    template<class T>
    size_t RLECompressedColumn<T>::size() const throw() {
        uint64_t size = 0;
        for (auto const &pair: values) size += pair.first;
        return size;
    }

    template<class T>
    const ColumnPtr RLECompressedColumn<T>::copy() const {
        return ColumnPtr(new RLECompressedColumn(*this));
    }

    template<class T>
    bool RLECompressedColumn<T>::update(TID tid, const boost::any &new_value) {
        if (new_value.empty() || typeid(T) != new_value.type()) return false;

        size_t idx_of_run = 0, idx_in_run = 0;
        tid_to_idx(tid, idx_of_run, idx_in_run);

        auto &entry = values[idx_of_run];

        auto val = boost::any_cast<T>(new_value);
        if (entry.first == 1) {
            // is run
            entry.second = val;
        } else if (idx_in_run == 0) {
            // Updating part of a run decreases its length
            // leads, insert before run
            entry.first--;
            values.insert(values.begin() + idx_of_run, make_pair(1, val));
        } else if (idx_in_run == entry.first) {
            // trails, insert after run
            entry.first--;
            values.insert(values.begin() + idx_of_run, make_pair(1, val));
        } else {
            // inside, split run, insert inbetween
            // 1 1 1 1 1 1 1 1 1 [(9, 1)]
            // [5] = 2
            // 1 1 1 1 1 2 1 1 1 [(5, 1), (1, 2), (3, 1)]
            uint8_t len_a = idx_in_run, len_b = entry.first - len_a - 1;

            entry.first = len_a;
            vector<Item> v{make_pair(1, val), make_pair(len_b, entry.second)};
            values.insert(values.begin() + idx_of_run + 1, begin(v), end(v));
        }

        return true;
    }

    template<class T>
    bool RLECompressedColumn<T>::update(PositionListPtr,
                                        const boost::any &) {
        return false;
    }

    template<class T>
    bool RLECompressedColumn<T>::remove(TID tid) {
        size_t idx_of_run = 0, idx_in_run = 0;
        tid_to_idx(tid, idx_of_run, idx_in_run);

        // TODO: abort/false if tid not in values

        auto &entry = values[idx_of_run];
        if (entry.first == 1) {
            // is run
            values.erase(values.begin() + idx_of_run);
        } else {
            values[idx_of_run].first--;
        }

        return true;
    }

    template<class T>
    bool RLECompressedColumn<T>::remove(PositionListPtr) {
        return false;
    }

    template<class T>
    bool RLECompressedColumn<T>::clearContent() {
        values.clear();
        return true;
    }

    template<class T>
    bool RLECompressedColumn<T>::store(const string &dir) {
        string path(dir);
        path += "/";
        path += this->name_;
        ofstream outfile(path.c_str(), ios_base::binary | ios_base::out);
        boost::archive::binary_oarchive oa(outfile);

        oa << values;

        outfile.flush();
        outfile.close();
        return true;
    }

    template<class T>
    bool RLECompressedColumn<T>::load(const string &dir) {
        string path(dir);
        path += "/";
        path += this->name_;

        ifstream infile(path.c_str(), ios_base::binary | ios_base::in);
        boost::archive::binary_iarchive ia(infile);
        ia >> values;
        infile.close();

        return true;
    }


    template<class T>
    T &RLECompressedColumn<T>::operator[](int idx) {
        static T t;
        for (auto &pair: values) {
            if (pair.first > idx) return pair.second;
            else
                idx -= pair.first;
        }
        return t;
    }

    template<class T>
    unsigned int RLECompressedColumn<T>::getSizeinBytes() const throw() {
        return values.size() * sizeof(Item);
    }

    /***************** End of Implementation Section ******************/

};// namespace CoGaDB
