//
// Created by mrplotva on 14.01.23.
//
#include <functional>
#include <stdexcept>

#ifndef HASHMAP_HASHMAP_H
#define HASHMAP_HASHMAP_H

#endif //HASHMAP_HASHMAP_H


template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
public:

    struct node {
        std::pair<KeyType, ValueType> el;
        int dist = -1;
        bool deleted = false;

        node() = default;

        ~node() = default;

        explicit node(std::pair<KeyType, ValueType> e, int d, bool del) : el(e) {
            dist = d;
            deleted = del;
        }
    };

    std::vector<node> table_;

    HashMap(Hash hasher = Hash()) : hasher_(hasher) {
        table_.resize(10);
    }

    template<typename Iterator>
    HashMap(Iterator f, Iterator s, Hash hasher = Hash()) : hasher_(hasher) {
        size_t cnt = 0;
        Iterator fcnt = f;
        while (fcnt != s) {
            ++fcnt;
            ++cnt;
        }
        table_.resize(10 * cnt);
        while (f != s) {
            insert(*f);
            ++f;
        }
    }

    ~HashMap() {}

    HashMap(std::initializer_list<std::pair<const KeyType, ValueType>> list, Hash hasher = Hash()) : hasher_(hasher) {
        table_.resize(10 * (list.size() + 1));
        for (auto &x: list) {
            insert(x);
        }
    }

    HashMap(const HashMap &other) : hasher_(other.hasher_) {
        size_ = other.size_;
        table_.resize(other.table_.size(), node());
        for (size_t i = 0; i < table_.size(); ++i) {
            if (other.table_[i].dist != -1) {
                table_[i] = node(other.table_[i].el, other.table_[i].dist, other.table_[i].deleted);
            }
        }
    }

    HashMap &operator=(const HashMap &other) {
        hasher_ = other.hasher_;
        size_ = other.size_;
        table_.shrink_to_fit();
        table_.resize(other.table_.size(), node());
        for (size_t i = 0; i < table_.size(); ++i) {
            if (other.table_[i].dist != -1) {
                table_[i] = node(other.table_[i].el, other.table_[i].dist, other.table_[i].deleted);
            }
        }
        return *this;
    }


    class iterator {
    public:
        explicit iterator(node *pointer, node *last) {
            pointer_ = pointer;
            last_ = last;
        }

        iterator() {
            pointer_ = nullptr;
            last_ = nullptr;
        }

        std::pair<const KeyType, ValueType> operator*() {
            return *(operator->());
        }

        std::pair<const KeyType, ValueType> *operator->() const {
            return reinterpret_cast<std::pair<const KeyType, ValueType> *>(&(*pointer_).el);
        }

        iterator &operator=(iterator other) {
            if (*this == other) return *this;
            pointer_ = other.pointer_;
            last_ = other.last_;
            return *this;
        }

        node &get_node() {
            return *pointer_;
        }

        iterator &operator++() {
            ++pointer_;
            while ((pointer_->dist == -1 || pointer_->deleted) && pointer_ != last_) {
                ++pointer_;
            }
            return *this;
        }

        iterator operator++(int) {
            iterator el = iterator(pointer_, last_);
            ++pointer_;
            while ((pointer_->dist == -1 || pointer_->deleted) && pointer_ != last_) {
                ++pointer_;
            }
            return el;
        }

        bool operator==(const iterator &other) const {
            return (pointer_ == other.pointer_);
        }

        bool operator!=(const iterator &other) const {
            return (pointer_ != other.pointer_);
        }

    private:
        node *pointer_ = nullptr;
        node *last_ = nullptr;
    };

    class const_iterator {
    public:
        explicit const_iterator(const node *pointer, const node *last) {
            pointer_ = pointer;
            last_ = last;
        }

        const_iterator() {
            pointer_ = nullptr;
            last_ = nullptr;
        }

        const std::pair<const KeyType, ValueType> operator*() {
            return *(operator->());
        }

        const std::pair<const KeyType, ValueType> *operator->() const {
            return reinterpret_cast<const std::pair<const KeyType, ValueType> *>(&(*pointer_).el);
        }


        node get_node() {
            return *pointer_;
        }

        const_iterator &operator++() {
            ++pointer_;
            while ((pointer_->dist == -1 || pointer_->deleted) && pointer_ != last_) {
                ++pointer_;
            }
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator el = const_iterator(pointer_, last_);
            ++pointer_;
            while ((pointer_->dist == -1 || pointer_->deleted) && pointer_ != last_) {
                ++pointer_;
            }
            return el;
        }

        bool operator==(const const_iterator &other) const {
            return (pointer_ == other.pointer_);
        }

        bool operator!=(const const_iterator &other) const {
            return (pointer_ != other.pointer_);
        }

    private:
        const node *pointer_ = nullptr;
        const node *last_ = nullptr;
    };


    iterator begin() {
        if (size_ == 0) {
            return end();
        }
        size_t i = 0;
        while (table_[i].dist == -1 || table_[i].deleted) {
            ++i;
        }
        return iterator(&table_[i], &table_.back());
    }

    const_iterator begin() const {
        if (size_ == 0) {
            return end();
        }
        size_t i = 0;
        while (table_[i].dist == -1 || table_[i].deleted) {
            ++i;
        }
        return const_iterator(&table_[i], &table_.back());
    }

    iterator end() {
        return iterator(&table_.back(), &table_.back());
    }

    const_iterator end() const {
        return const_iterator(&table_.back(), &table_.back());
    }

    [[nodiscard]] size_t size() const {
        return size_;
    }

    [[nodiscard]] bool empty() const {
        return (size_ == 0);
    }

    Hash hash_function() const {
        return hasher_;
    }

    ValueType &operator[](KeyType key) {
        insert({key, ValueType()});
        return find(key).get_node().el.second;
    }


    const ValueType &at(KeyType key) const {
        if (find(key) == end()) {
            throw std::out_of_range("No key in hashmap");
        }
        return find(key)->second;
    }

    void erase(const KeyType key) {
        iterator it = find(key);
        if (it == end()) {
            return;
        }
        (it.get_node()).deleted = true;
        --size_;
        if (size_ && size_ * 100 < table_.size()) {
            rebuild();
        }
    }

    void clear() {
        size_ = 0;
        for (size_t i = 0; i < table_.size(); ++i) {
            table_[i].dist = -1;
            table_[i].deleted = false;
        }
        table_.shrink_to_fit();
        table_.resize(10);
    }

    const_iterator find(const KeyType key) const {
        size_t pos = H(hasher_(key));
        for (int d = 0; d < int(table_.size()); ++d) {
            size_t ind = (pos + d) % (table_.size() - 1);
            if (table_[ind].dist < d) {
                return end();
            } else if (table_[ind].el.first == key && !table_[ind].deleted) {
                return const_iterator(&table_[ind], &table_.back());
            }
        }
        return end();
    }

    iterator find(const KeyType key) {
        size_t pos = H(hasher_(key));
        for (int d = 0; d < int(table_.size()); ++d) {
            size_t ind = (pos + d) % (table_.size() - 1);
            if (table_[ind].dist < d) {
                return end();
            } else if (table_[ind].el.first == key && !table_[ind].deleted) {
                return iterator(&table_[ind], &table_.back());
            }
        }
        return end();
    }

    void rebuild() {
        std::vector<std::pair<KeyType, ValueType>> val;
        for (auto &x: table_) {
            if (x.dist != -1) {
                if (!x.deleted) val.push_back(x.el);
                x.deleted = false;
                x.dist = -1;
            }
        }
        size_ = 0;
        table_.shrink_to_fit();
        table_.resize(10 * (val.size() + 1));
        for (auto &x: val) {
            insert(x);
        }
    }

    void insert(std::pair<KeyType, ValueType> el) {
        if (find(el.first) != end()) {
            return;
        }
        if (size_ * 2 >= table_.size()) {
            rebuild();
        }
        node cur = node(el, 0, false);
        ++size_;
        while (cur.dist != -1 && !cur.deleted) {
            size_t pos = H(hasher_(cur.el.first));
            size_t ind = (pos + cur.dist) % (table_.size() - 1);
            if (table_[ind].dist < cur.dist) {
                std::swap(cur, table_[ind]);
            } else {
                ++cur.dist;
            }
        }
    }

private:
    Hash hasher_;
    size_t size_ = 0;

    [[nodiscard]] size_t H(size_t key) const {
        return key % (table_.size() - 1);
    }
};

