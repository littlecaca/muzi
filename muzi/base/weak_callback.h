#ifndef MUZI_BASE_WEAK_CALLBACK_H_
#define MUZI_BASE_WEAK_CALLBACK_H_


#include <functional>
#include <memory>

namespace muzi
{
template <typename T, typename... Args>
class WeakCallback
{
public:
    WeakCallback(const std::weak_ptr<T> &object, std::function<void(T *, Args...)> callback)
        : object_(object), cb_(std::move(callback))
    {
    }

    void operator()(Args&&... args)
    {
        std::shared_ptr<T> ptr = object_.lock();
        if (ptr)
        {
            cb_(ptr.get(), std::forward<Args>(args)...);
        }
    }

    operator bool()
    {
        return !object_.expired();
    }

private:
    std::weak_ptr<T> object_;
    std::function<void(T *, Args...)> cb_;
};

/// Type Incompatibility: A pointer to a const member function and a
/// pointer to a non-const member function are two different types.
/// C++ treats them as distinct, and you cannot convert one to 
/// the other implicitly. This is why you need separate overloads to handle each case.

template <typename T, typename... Args>
WeakCallback<T, Args...> MakeWeakCallback(const std::shared_ptr<T> &object, 
    void (T::*callback)(Args...))
{
    return WeakCallback<T, Args...>(object, callback);
}

template <typename T, typename... Args>
WeakCallback<T, Args...> MakeWeakCallback(const std::shared_ptr<T> &object, 
    void (T::*callback)(Args...) const)
{
    return WeakCallback<T, Args...>(object, callback);
}

}   // namespace muzi


#endif  // MUZI_BASE_WEAK_CALLBACK_H_
