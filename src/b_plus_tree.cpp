#include "b_plus_tree.hpp"

namespace IN2118 {

template <typename K, typename CMP>
bool CBPlusTree<K, CMP>::lookup(K key, TID& tid)
{
    CLeafNode* leaf;
    CBufferFrame& bf = GetLeaf(key, false, &leaf);

    bool found = leaf->getTID(key, tid);
    _buffer_manager.unfixPage(bf, false);
    return found;
}

template <typename K, typename CMP>
CBufferFrame& CBPlusTree<K, CMP>::GetLeaf(K& key, bool exclusive, CLeafNode** leafptr)
{
    CBufferFrame* frame = &this->_buffer_manager.fixPage(_root, exclusive);
    CNode* node = static_cast<CNode*>(frame->getData());

    while (!node->isLeaf()) {
        CInnerNode* inner = reinterpret_cast<CInnerNode*>(node);
        CBufferFrame* next_frame = &this->_buffer_manager.fixPage(inner->getChild(key), exclusive);
        this->_buffer_manager.unfixPage(*frame, false);
        frame = next_frame;
        node = static_cast<CNode*>(frame->getData());
    }

    *leafptr = reinterpret_cast<CLeafNode*>(node);
    return *frame;
}

template <typename K, typename CMP>
void CBPlusTree<K, CMP>::updateParent(
    K& key, K& separator, CBufferFrame** frame, CBufferFrame** insert_frame, CBufferFrame** parent_frame)
{
    if (*parent_frame == NULL) {
        CBufferFrame* root_frame = &this->_buffer_manager.fixPage(++this->_size, true);
        memcpy((*root_frame).getData(), (*frame)->getData(), PAGE_SIZE);
        new ((*frame)->getData()) CInnerNode(this->_size, (*insert_frame)->getPageId(), separator);
        *parent_frame = *frame;
        *frame = root_frame;
    }
    else {
        CInnerNode* parent = static_cast<CInnerNode*>((*parent_frame)->getData());
        parent->insert(separator, (*insert_frame)->getPageId());
    }

    if (!_cmp(separator, key)) {
        this->_buffer_manager.unfixPage(*(*insert_frame), true);
        *insert_frame = NULL;
    }
    else {
        this->_buffer_manager.unfixPage(*(*frame), true);
        *frame = *insert_frame;
    }
}

template <typename K, typename CMP>
void CBPlusTree<K, CMP>::insert(K key, TID tid)
{
    CBufferFrame* frame = &this->_buffer_manager.fixPage(_root, true);
    CNode* node = static_cast<CNode*>(frame->getData());
    CBufferFrame* parent_frame = NULL;

    // go through innernode tree
    while (!node->isLeaf()) {
        if (node->isFull()) {
            CBufferFrame* insert_frame = &this->_buffer_manager.fixPage(++this->_size, true);

            K separator;
            CInnerNode* oldInner = reinterpret_cast<CInnerNode*>(node);
            separator = oldInner->split(*insert_frame);

            updateParent(key, separator, &frame, &insert_frame, &parent_frame);

            node = static_cast<CNode*>(frame->getData());
        }
        else {
            CInnerNode* inner = reinterpret_cast<CInnerNode*>(node);

            CBufferFrame* next_frame = &this->_buffer_manager.fixPage(inner->getChild(key), true);
            if (parent_frame != NULL) {
                this->_buffer_manager.unfixPage(*parent_frame, true);
            }

            parent_frame = frame;
            frame = next_frame;

            node = static_cast<CNode*>(frame->getData());
        }
    }

    // insert in leaf
    if (node->isFull()) {
        CBufferFrame* insert_frame = &this->_buffer_manager.fixPage(++this->_size, true);

        K separator;
        CLeafNode* oldLeaf = reinterpret_cast<CLeafNode*>(node);
        separator = oldLeaf->split(*insert_frame);

        updateParent(key, separator, &frame, &insert_frame, &parent_frame);
        node = static_cast<CNode*>(frame->getData());
    }

    CLeafNode* leaf = reinterpret_cast<CLeafNode*>(node);
    leaf->insert(key, tid);
    this->_entries_count++;

    if (parent_frame != NULL) {
        this->_buffer_manager.unfixPage(*parent_frame, true);
    }

    this->_buffer_manager.unfixPage(*frame, true);
}

template <typename K, typename CMP>
bool CBPlusTree<K, CMP>::erase(K key)
{
    CLeafNode* leaf;
    CBufferFrame& bf = GetLeaf(key, true, &leaf);

    bool found = leaf->remove(key);
    this->_buffer_manager.unfixPage(bf, found);
    if (found)
        this->_entries_count--;
    return found;
}
}; // ns
