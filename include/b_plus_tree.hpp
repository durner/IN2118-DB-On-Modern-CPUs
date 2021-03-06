#pragma once

#include "segment.hpp"
#include <algorithm>
#include <string.h>

namespace IN2118 {

template <class K, class CMP = std::less<K> >
class CBPlusTree : public CSegment {
    CMP _cmp;

    class CNode {
    protected:
        CMP _cmp;
        bool _leaf;
        size_t _count;

        CNode(bool leaf)
            : _leaf(leaf)
            , _count(0){};

    public:
        inline bool isLeaf() { return _leaf; }
        virtual bool isFull() = 0;
    };

    class CInnerNode : public CNode {
        static const size_t space = (PAGE_SIZE - sizeof(CNode) - sizeof(uint64_t)) / (sizeof(K) + sizeof(uint64_t));
        K _keys[space];
        uint64_t _children[space + 1];

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

        bool isFull() { return this->_count == space + 1; }

        size_t getKeyIndex(K key) { return std::lower_bound(_keys, _keys + this->_count - 1, key, this->_cmp) - _keys; }

        uint64_t getChild(K key) { return _children[getKeyIndex(key)]; }

        void insert(K key, uint64_t child)
        {
            auto index = getKeyIndex(key);
            if (index < this->_count - 1 && !this->_cmp(key, _keys[index])) {
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

        K split(CBufferFrame& bufferFrame)
        {
            CInnerNode* inner = new (bufferFrame.getData()) CInnerNode();
            auto middle = this->_count / 2;
            this->_count -= middle;
            inner->_count = middle;
            memcpy(inner->_keys, _keys + this->_count, (middle - 1) * sizeof(K));
            memcpy(inner->_children, _children + this->_count, middle * sizeof(uint64_t));
            return _keys[this->_count - 1];
        }
    };

    class CLeafNode : public CNode {
        static const size_t space = (PAGE_SIZE - sizeof(CNode)) / (sizeof(K) + sizeof(TID));
        K _keys[space];
        TID _tids[space];

    public:
        CLeafNode()
            : CNode(true){};

        bool isFull() { return this->_count == space; }

        size_t getKeyIndex(K key) { return std::lower_bound(_keys, _keys + this->_count, key, this->_cmp) - _keys; }

        bool getTID(K key, TID& tid)
        {
            auto index = getKeyIndex(key);
            if (index == this->_count || this->_cmp(key, _keys[index]))
                return false;
            tid = _tids[index];
            return true;
        }

        void insert(K key, TID tid)
        {
            auto index = getKeyIndex(key);
            if (index < this->_count && !this->_cmp(key, _keys[index])) {
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
            std::move(_keys + this->_count, _keys + this->_count + middle, newLeaf->_keys);
            std::move(_tids + this->_count, _tids + this->_count + middle, newLeaf->_tids);
            return this->_keys[this->_count - 1];
        }
    };

public:
    CBPlusTree(CBufferManager& _buffer_manager, uint16_t segmentId)
        : CSegment(_buffer_manager, segmentId)
        , _root(0)
        , _entries_count(0)
    {
        CBufferFrame& bf = _buffer_manager.fixPage(_root, true);
        void* data = bf.getData();
        new (data) CLeafNode();
        _buffer_manager.unfixPage(bf, true);
    };

    size_t size() { return _entries_count; }

    bool lookup(K key, TID& tid);
    void insert(K key, TID tid);
    bool erase(K key);

private:
    uint64_t _root;
    size_t _entries_count;

    void updateParent(K& key, K& separator, CBufferFrame** frame, CBufferFrame** insert, CBufferFrame** parent);
    CBufferFrame& GetLeaf(K& key, bool exclusive, CLeafNode** leafPtr);
};
}; // ns
