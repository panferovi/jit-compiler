#ifndef UTILS_INTRUSIVE_LIST_H
#define UTILS_INTRUSIVE_LIST_H

#include "utils/macros.h"

#include <cstddef>
#include <stdexcept>
#include <iterator>

namespace compiler::utils {

// Represents node of circular doubly-linked list
template <typename T>
struct IntrusiveListNode {
    using Node = IntrusiveListNode;

    Node *Prev()
    {
        return prev_;
    }

    Node *Next()
    {
        return next_;
    }

    // Links this node before next in list
    void LinkBefore(Node *next)
    {
        ASSERT(!IsLinked());

        prev_ = next->prev_;
        prev_->next_ = this;
        next_ = next;
        next->prev_ = this;
    }

    // Is this node linked in a circular list?
    bool IsLinked() const
    {
        return next_ != nullptr;
    }

    // Unlink this node from current list
    void Unlink()
    {
        if (next_) {
            next_->prev_ = prev_;
        }
        if (prev_) {
            prev_->next_ = next_;
        }
        next_ = prev_ = nullptr;
    }

    T *AsItem()
    {
        return static_cast<T *>(this);
    }

    Node *prev_ = nullptr;
    Node *next_ = nullptr;
};

// Implemented as circular doubly-linked list with sentinel node
template <typename T>
class IntrusiveList {
    using Node = IntrusiveListNode<T>;
    using List = IntrusiveList<T>;

public:
    void PushBack(Node *node)
    {
        node->LinkBefore(&head_);
    }

    void PushFront(Node *node)
    {
        node->LinkBefore(head_.next_);
    }

    // Returns nullptr if empty
    T *PopFront()
    {
        if (IsEmpty()) {
            return nullptr;
        }
        Node *front = head_.next_;
        front->Unlink();
        return front->AsItem();
    }

    // Returns nullptr if empty
    T *PopBack()
    {
        if (IsEmpty()) {
            return nullptr;
        }
        Node *back = head_.prev_;
        back->Unlink();
        return back->AsItem();
    }

    // Append (= move, re-link) all nodes from `that` list to the end of this list
    // Post-condition: that.IsEmpty() == true
    void Append(List &that)
    {
        if (that.IsEmpty()) {
            return;
        }

        Node *that_front = that.head_.next_;
        Node *that_back = that.head_.prev_;

        that_back->next_ = &head_;
        that_front->prev_ = head_.prev_;

        Node *back = head_.prev_;

        head_.prev_ = that_back;
        back->next_ = that_front;

        that.head_.next_ = that.head_.prev_ = &that.head_;
    }

    bool IsEmpty() const
    {
        return head_.next_ == &head_;
    }

    bool NonEmpty() const
    {
        return !IsEmpty();
    }

    bool HasItems() const
    {
        return !IsEmpty();
    }

    IntrusiveList()
    {
        InitEmpty();
    }

    IntrusiveList(List &&that)
    {
        InitEmpty();
        Append(that);
    }

    // Intentionally disabled
    // Be explicit: use UnlinkAll + Append
    NO_MOVE_OPERATOR(IntrusiveList);

    NO_COPY_SEMANTIC(IntrusiveList);

    ~IntrusiveList() = default;

    // Complexity: O(size)
    size_t Size() const
    {
        return std::distance(begin(), end());
    }

    // Complexity: O(1)
    void Swap(List &with)
    {
        List tmp;
        tmp.Append(*this);
        Append(with);
        with.Append(tmp);
    }

    // Complexity: O(size)
    template <typename Less>
    void Sort(Less less)
    {
        List sorted {};

        while (NonEmpty()) {
            Node *candidate = head_.Next();
            ASSERT(candidate != &head_);

            Node *curr = candidate->Next();
            while (curr != &head_) {
                if (less(curr->AsItem(), candidate->AsItem())) {
                    candidate = curr;
                }
                curr = curr->Next();
            }

            ASSERT(candidate->IsLinked());
            candidate->Unlink();

            sorted.PushBack(candidate);
        }

        Swap(sorted);
    }

    void UnlinkAll()
    {
        Node *current = head_.next_;
        while (current != &head_) {
            Node *next = current->next_;
            current->Unlink();
            current = next;
        }
    }

    void Clear()
    {
        UnlinkAll();
    }

    // Unlinked tagged node
    static void Unlink(Node *node)
    {
        node->Unlink();
    }

    static bool IsLinked(Node *node)
    {
        return node->IsLinked();
    }

    // Iteration
    template <bool Next, class NodeT, class ItemT>
    class IteratorImpl {
        using Iterator = IteratorImpl<Next, NodeT, ItemT>;

    public:
        using value_type = ItemT;
        using pointer = value_type *;
        using reference = value_type *;
        using difference_type = ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

    public:
        IteratorImpl(NodeT *start) : current_(start) {}

        // prefix increment
        Iterator &operator++()
        {
            if constexpr (Next) {
                current_ = current_->next_;
            } else {
                current_ = current_->prev_;
            }
            return *this;
        }

        Iterator &operator--()
        {
            current_ = current_->prev_;
            return *this;
        }

        bool operator==(const Iterator &that) const
        {
            return current_ == that.current_;
        }

        bool operator!=(const Iterator &that) const
        {
            return !(*this == that);
        }

        ItemT *operator*() const
        {
            return Item();
        }

        ItemT *operator->() const
        {
            return Item();
        }

        ItemT *Item() const
        {
            return static_cast<ItemT *>(current_);
        }

    private:
        NodeT *current_;
    };

    // Forward iteration
    using Iterator = IteratorImpl<true, Node, T>;
    using ConstIterator = IteratorImpl<true, const Node, const T>;

    Iterator begin()
    {
        return Iterator(head_.next_);
    }

    Iterator end()
    {
        return Iterator(&head_);
    }

    ConstIterator begin() const
    {
        return ConstIterator(head_.next_);
    }

    ConstIterator end() const
    {
        return ConstIterator(&head_);
    }

    // Reverse iterator
    using ReverseIterator = IteratorImpl<false, Node, T>;
    using ConstReverseIterator = IteratorImpl<false, const Node, const T>;

    ReverseIterator rbegin()
    {
        return ReverseIterator(head_.prev_);
    }

    ReverseIterator rend()
    {
        return ReverseIterator(&head_);
    }

    ConstReverseIterator rbegin() const
    {
        return ConstReverseIterator(head_.prev_);
    }

    ConstReverseIterator rend() const
    {
        return ConstReverseIterator(&head_);
    }

private:
    void InitEmpty()
    {
        head_.next_ = head_.prev_ = &head_;
    }

    Node head_;
};

}  // namespace compiler::utils

#endif  // UTILS_INTRUSIVE_LIST_H
