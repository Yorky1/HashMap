#include <algorithm>
#include <functional>
#include <vector>
#include <iostream>
#include <initializer_list>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
private:
    Hash hasher;

    const size_t DEFAULT_SIZE = 2;
    const size_t COEF_OF_SIZE_TO_CHANGE = 4;
    const size_t COEF_OF_NEW_SIZE = 2;

    struct Node {
        Node* next;
        Node* prev;
        std::pair<const KeyType, ValueType> val;
        size_t hash;
        Node(std::pair<const KeyType, ValueType>& p) : val(p) {
            next = nullptr;
            prev = nullptr;
        }
    };

    Node* first;
    Node* last;
    size_t buffer_size;
    size_t map_size;
    std::vector<Node*> nodes;

    bool isPrime(size_t x) {
        for (size_t d = 2; d * d <= x; ++d) {
            if (x % d == 0) {
                return false;
            }
        }
        return true;
    }

    size_t nextPrimeNumber(size_t x) {
        while (!isPrime(x)) ++x;
        return x;
    }

    void eraseNode(Node* node) {
        if (nodes[node->hash] == node && node->next && node->next->hash == node->hash)
            nodes[node->hash] = node->next;
        else if (nodes[node->hash] == node)
            nodes[node->hash] = nullptr;
        if (node == first) first = node->next;
        if (node == last) last = node->prev;
        if (node->next) node->next->prev = node->prev;
        if (node->prev) node->prev->next = node->next;
        delete node;
    }

    void insertNode(Node* oldNode, Node* newNode) {
        if (!oldNode) {
            first = last = newNode;
            return;
        }
        if (oldNode == last) {
            last = newNode;
        }
        if (oldNode->next) {
            newNode->next = oldNode->next;
            oldNode->next->prev = newNode;
        }
        oldNode->next = newNode;
        newNode->prev = oldNode;
    }

    Node* findNode(KeyType key) const {
        size_t hash = hasher(key) % buffer_size;
        Node* now = nodes[hash];
        if (!now) return nullptr;
        while (now && now->hash == hash) {
            if (now->val.first == key) return now;
            now = now->next;
        }
        return nullptr;
    }

    void resize() {
        buffer_size = nextPrimeNumber(buffer_size * COEF_OF_NEW_SIZE);
        map_size = 0;
        nodes.clear();
        nodes.resize(buffer_size);
        Node* now = first;
        first = last = nullptr;
        while (now) {
            insert(now->val);
            Node* next = now->next;
            delete now;
            now = next;
        }
    }

public:

    class iterator {
    private:
        Node* node;

    public:

        iterator(Node* node_ = nullptr) {
            node = node_;
        }

        bool operator== (const iterator& other) const {
            return this->node == other.node;
        }

        bool operator!= (const iterator& other) const {
            return !(*this == other);
        }

        std::pair<const KeyType, ValueType>& operator*() {
            return node->val;
        }

        std::pair<const KeyType, ValueType>* operator->() {
            return &node->val;
        }

        iterator& operator++() {
            if (node) node = node->next;
            return *this;
        }

        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }
    };

    class const_iterator {
    private:
        Node* node;

    public:

        const_iterator(Node* node_ = nullptr) {
            node = node_;
        }

        bool operator== (const const_iterator& other) const {
            return this->node == other.node;
        }

        bool operator!= (const const_iterator& other) const {
            return !(*this == other);
        }

        const std::pair<const KeyType, ValueType>& operator*() {
            return node->val;
        }

        const std::pair<const KeyType, ValueType>* operator->() {
            return &node->val;
        }

        const_iterator& operator++() {
            if (node) node = node->next;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }
    };

    HashMap(Hash hasher_ = Hash()) : hasher(hasher_) {
        buffer_size = DEFAULT_SIZE;
        map_size = 0;
        nodes.resize(buffer_size);
        first = nullptr;
        last = nullptr;
    }

    template<typename Iterator>
    HashMap(Iterator first1, Iterator last1, Hash hasher_ = Hash()) : hasher(hasher_) {
        buffer_size = nextPrimeNumber(last1 - first1);
        map_size = 0;
        nodes.resize(buffer_size);
        first = nullptr;
        last = nullptr;
        while (first1 != last1) {
            insert(*first1);
            ++first1;
        }
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> list, Hash hasher_ = Hash()) : hasher(hasher_) {
        buffer_size = nextPrimeNumber(list.size());
        map_size = 0;
        nodes.resize(buffer_size);
        first = nullptr;
        last = nullptr;
        for (auto p : list) {
            insert(p);
        }
    }

    HashMap(const HashMap<KeyType, ValueType, Hash>& other) : hasher(other.hasher) {
        buffer_size = other.buffer_size;
        nodes.resize(buffer_size);
        map_size = 0;
        first = nullptr;
        last = nullptr;
        Node* now = other.first;
        while (now) {
            insert(now->val);
            now = now->next;
        }
    }

    HashMap& operator=(HashMap<KeyType, ValueType, Hash>& other) {
        if (*this == other) return *this;
        clear();
        buffer_size = std::max(other.buffer_size, buffer_size);
        for (auto p : other) {
            insert(p);
        }
        return *this;
    }

    bool operator==(const HashMap<KeyType, ValueType>& other) const {
        if (map_size != other.size()) return false;
        auto it = begin();
        auto otherIt = other.begin();
        while (it != end()) {
            if (!(*it == *otherIt)) return false;
            ++it;
            ++otherIt;
        }
        return true;
    }

    void erase(KeyType key) {
        Node* node = findNode(key);
        if (node) {
            eraseNode(node);
            --map_size;
        }
    }

    void insert(std::pair<const KeyType, ValueType> p) {
        Node* node = findNode(p.first);
        if (node) {
            return;
        }
        size_t hash = hasher(p.first) % buffer_size;
        Node* newNode = new Node(p);
        newNode->hash = hash;
        insertNode(nodes[hash] ? nodes[hash] : last, newNode);
        if (!nodes[hash]) nodes[hash] = newNode;
        ++map_size;
        if (COEF_OF_SIZE_TO_CHANGE * map_size > buffer_size) {
            resize();
        }
    }

    size_t size() const {
        return map_size;
    }

    bool empty() const {
        return map_size == 0;
    }

    Hash hash_function() const {
        return hasher;
    }

    ValueType& operator[] (KeyType key) {
        std::pair<const KeyType, ValueType> p = { key, ValueType() };
        insert(p);
        auto node = findNode(key);
        return node->val.second;
    }

    iterator begin() {
        return iterator(first);
    }

    iterator end() {
        return iterator(nullptr);
    }

    const_iterator begin() const {
        return const_iterator(first);
    }

    const_iterator end() const {
        return const_iterator(nullptr);
    }

    iterator find(KeyType key) {
        Node* node = findNode(key);
        if (!node) return end();
        return iterator(node);
    }

    const_iterator find(KeyType key) const {
        Node* node = findNode(key);
        if (!node) return end();
        return const_iterator(node);
    }

    const ValueType& at(KeyType key) const {
        Node* node = findNode(key);
        if (!node) throw std::out_of_range("No such key");
        return node->val.second;
    }

    void clear() {
        while (map_size != 0) {
            erase(first->val.first);
        }
    }

    ~HashMap() {
        clear();
    }
};
