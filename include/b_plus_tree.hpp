#pragma once

#include <algorithm>

#include "segment.hpp"

namespace IN2118 {

template <class K, class CMP = std::less<K> >
class CBPlusTree : public CSegment {
    class CNode {
    protected:
        bool _leaf;
        size_t _count;

        CNode(bool leaf)
            : _leaf(leaf)
            , _count(0){};

    public:
        inline bool isLeaf() { return _leaf; }

        bool empty() { return _count == 0; }

        virtual bool isFull() = 0;

        virtual K maximumKey() = 0;

        virtual K minimumKey() = 0;

        virtual K split(CBufferFrame* bufferFrame) = 0;
    };

    class CInnerNode : public CNode {
        static const size_t order = (PAGE_SIZE - sizeof(CNode) - sizeof(uint64_t)) / (sizeof(K) + sizeof(uint64_t));
        K _keys[order];
        uint64_t _children[order + 1];

        CInnerNode()
            : CNode(false){};

    public:
        CInnerNode(uint64_t firstChild, uint64_t secondChild, K separator)
            : CNode(false)
        {
            _keys[0] = separator;
            _children[0] = firstChild;
            _children[1] = secondChild;
            this->_count = 2;
        };

        bool isFull() { return this->_count == order + 1; }

        K maximumKey() { return _keys[this->_count - 2]; }

        K minimumKey() { return _keys[0]; }

        size_t getKeyIndex(K key) { return std::lower_bound(_keys, _keys + this->_count - 1, key, _cmp) - _keys; }

        uint64_t getChild(K key) { return _children[getKeyIndex(key)]; }

        void insert(K key, uint64_t child)
        {
            auto index = getKeyIndex(key);

            if (index < this->_count - 1 && !_cmp(key, _keys[index])) {
                _children[index] = child;
                return;
            }

            memmove(_keys + index + 1, _keys + index, (this->_count - index - 1) * sizeof(K));
            memmove(_children + index + 2, _children + index + 1, (this->_count - index - 1) * sizeof(uint64_t));

            _keys[index] = key;
            _children[index + 1] = child;
            this->_count++;
        }

        bool remove(K key)
        {
            auto index = getKeyIndex(key);
            if (index == this->_count)
                return false;

            memmove(_keys + index, _keys + index + 1, (this->_count - index - 1) * sizeof(K));
            memmove(_children + index, _children + index + 1, (this->_count - index - 1) * sizeof(uint64_t));
            this->_count--;
            return true;
        }

        bool invalidate(K key)
        {
            auto index = getKeyIndex(key);
            if (index == this->_count)
                return false;

            _children[index] = -1;
            return true;
        }

        void reactivate(K key, uint64_t child) { _children[getKeyIndex(key)] = child; };

        K split(CBufferFrame& bufferFrame)
        {
            CInnerNode* newCInnerNode = new (bufferFrame.getData()) CInnerNode();

            auto middle = this->_count / 2;
            this->_count -= middle;
            newCInnerNode->_count = middle;

            auto keysSecondHalf = _keys + this->_count;
            auto childrenSecondHalf = _children + this->_count;
            std::move(keysSecondHalf, keysSecondHalf + (middle - 1) * sizeof(K), newCInnerNode->_keys);
            std::move(childrenSecondHalf, childrenSecondHalf + middle * sizeof(uint64_t), newCInnerNode->_children);

            return _keys[this->_count - 1];
        }
    };

    class CLeafNode : public CNode {
        static const size_t order = (PAGE_SIZE - sizeof(CNode)) / (sizeof(K) + sizeof(TID));

        K _keys[order];
        TID _tids[order];

    public:
        CLeafNode()
            : CNode(true){};

        bool isFull() { return this->_count == order; }

        K maximumKey() { return _keys[this->_count - 1]; }

        K minimumKey() { return _keys[0]; }

        size_t getKeyIndex(K key) { return std::lower_bound(_keys, _keys + this->_count, key, _cmp) - _keys; }

        bool getTID(K key, TID& tid)
        {
            auto index = getKeyIndex(key);

            if (index == this->_count || _cmp(key, _keys[index]))
                return false;

            tid = _tids[index];
            return true;
        }

        void insert(K key, TID tid)
        {
            auto index = getKeyIndex(key);
            if (index < this->_count && !_cmp(key, _keys[index])) {
                _tids[index] = tid;
                return;
            }

            memmove(_keys + index + 1, _keys + index, (this->_count - index) * sizeof(K));
            memmove(_tids + index + 1, _tids + index, (this->_count - index) * sizeof(TID));

            _keys[index] = key;
            _tids[index] = tid;
            this->_count++;
        }

        bool remove(K key)
        {
            auto index = getKeyIndex(key);
            if (index == this->_count)
                return false;

            memmove(_keys + index, _keys + index + 1, (this->_count - index - 1) * sizeof(K));
            memmove(_tids + index, _tids + index + 1, (this->_count - index - 1) * sizeof(TID));
            this->_count--;
            return true;
        }

        K split(CBufferFrame& bufferFrame)
        {
            CLeafNode* newLeaf = new (bufferFrame.getData()) CLeafNode();

            auto middle = this->_count / 2;
            this->_count -= middle;
            newLeaf->_count = middle;

            auto keysSecondHalf = _keys + this->_count;
            auto tidsSecondHalf = _tids + this->_count;
            std::move(keysSecondHalf, keysSecondHalf + middle, newLeaf->_keys);
            std::move(tidsSecondHalf, tidsSecondHalf + middle, newLeaf->_tids);

            return this->maximumKey();
        }
    };

public:
    CBPlusTree(CBufferManager& _buffer_manager, uint16_t segmentId)
        : CSegment(_buffer_manager, segmentId)
        , _root(0)
        , _entries_count(0){};

    size_t getNumberOfEntries() { return _entries_count; }
    bool lookup(K key, TID& tid);
    void insert(K key, TID tid);
    bool erase(K key);

protected:
    static CMP _cmp;

private:
    uint64_t _root;

    size_t _entries_count;

    std::list<uint64_t> _freed_pages;

    uint64_t getFreePage()
    {
        if (!_freed_pages.empty()) {
            auto page = _freed_pages.front();
            _freed_pages.pop_front();
            return page;
        }
        return ++this->_size;
    }
};
}; // ns
