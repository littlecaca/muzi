/**
 * This class can only be inherited.
 * [status] : done
 */
#ifndef MUZI_BASE_NONCOPYABLE_H_
#define MUZI_BASE_NONCOPYABLE_H_

namespace muzi {
class noncopyable
{
public:
    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;
protected:
    noncopyable() = default;
};

}   // namespace muzi

#endif  // MUZI_BASE_NONCOPYABLE_H_
