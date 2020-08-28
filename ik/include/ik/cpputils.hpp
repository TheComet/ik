#pragma once

#ifdef __cplusplus

#include "ik/refcount.h"

namespace ik {

template <class T>
class Ref
{
public:
    Ref() noexcept : obj_(nullptr)
    {}

    Ref(std::nullptr_t) noexcept : obj_(nullptr)
    {}

    Ref(const Ref<T>& other) noexcept : obj_(other.obj_)
    {
        addRef();
    }

    Ref(T* ptr) noexcept : obj_(ptr)
    {
        addRef();
    }

    ~Ref() noexcept
    {
        releaseRef();
    }

    Ref<T>& operator=(Ref<T> other)
    {
        swap(*this, other);
        return *this;
    }

    T* operator->() const
    {
        assert(obj_);
        return obj_;
    }

    T& operator*() const
    {
        assert(obj_);
        return obj_;
    }

    T& operator[](int idx)
    {
        assert(obj_);
        assert(idx < obj_->obj_count);
        return obj_[idx];
    }

    operator T*() const
    {
        return obj_;
    }

    operator ik_refcounted*() const
    {
        return (ik_refcounted*)obj_;
    }

    template <class U>
    bool operator<(const Ref<T>& other) const
    {
        return obj_ < other.obj_;
    }

    template <class U>
    bool operator==(const Ref<T>& other) const
    {
        return obj_ == other.obj_;
    }

    template <class U>
    bool operator!=(const Ref<T>& other) const
    {
        return operator==(other);
    }

    friend void swap(Ref<T>& first, Ref<T>& second) noexcept
    {
        using std::swap;
        swap(first.obj_, second.obj_);
    }

    void reset()
    {
        releaseRef();
    }

    bool isNull() const
    {
        return obj_ == nullptr;
    }

    T* get() const
    {
        return obj_;
    }

    int refcount() const
    {
        return obj_ ? IK_REFCOUNT(obj_) : 0;
    }

private:
    void addRef()
    {
        IK_XINCREF(obj_);
    }

    void releaseRef()
    {
        IK_XDECREF(obj_);
        obj_ = nullptr;
    }

private:
    T* obj_;
};

}

#endif
