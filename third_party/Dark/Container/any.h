#ifndef _DARK_ANY_H_
#define _DARK_ANY_H_

#include <utility>
#include <typeinfo>

namespace dark {

/**
 * A dynamic class which can hold any type (%Any/nullptr excluded)
 * and support basic coversion function.
 * Note that all the input type will be decayed, which means that 
 * the const / & / volatile label of input type won't go into effect.
 * 
 */
class Any {
  private:
    /* Base class (abstract class). */
    struct Base {
        virtual ~Base() = default;
        virtual Base *clone() const = 0;
    };

    /* The actual data holder. */
    template <typename T>
    struct Data : Base {
      public:
        T data;
        virtual ~Data() override = default;
        Data(T &&val) : data(std::move(val)) {}
        Data(const T &val) : data(val) {}

        /* Tries to copy an T object and return a T pointer towards the value.
           If attempt failed, nullptr will be returned instead. */
        virtual Base *clone() const noexcept override { return try_clone <> (); }

      private:

        /* Clone a copy-constructible object. */
        template <typename Q = T,
                  typename std::enable_if_t<std::is_copy_constructible<Q>::value, int> = 0>
        Base *try_clone() const noexcept {
            return new Data <T> (data);
        }

        /* Fail to clone, thus return nullptr. */
        template <typename Q = T,
                  typename std::enable_if_t<!std::is_copy_constructible<Q>::value, int> = 0>
        Base *try_clone() const noexcept {
            return nullptr;
        }
    };

    Base *ptr;
    

  public:
    ~Any() noexcept { delete ptr; }
    /* Initialize by setting as nullptr. */
    Any() noexcept : ptr(nullptr) {}
    /* Special case for nullptr. */
    Any(std::nullptr_t) noexcept : ptr(nullptr) {}

    /* Perfect forwarding initialization for normal types (%Any/nullptr excluded). */
    template <typename U>
    Any(U &&val) : ptr( new Data <std::decay_t <U>> (std::forward <U> (val)) ) {}

    /* Move the content from rhs by swaping pointers. */
    Any(Any &&rhs) noexcept : ptr(rhs.ptr) { rhs.ptr = nullptr; }
    /* Copy the content from rhs if the inner class of rhs is copy-constructible.
     * Otherwise, this Any object will be set as nullptr.*/
    Any(const Any &rhs) : ptr(rhs.ptr ? rhs.ptr->clone() : nullptr) {}
    Any(const Any &&rhs) : Any(rhs) {}

    /* Special case for nullptr. */
    Any &operator = (std::nullptr_t) noexcept {
        this->~Any();
        ptr = nullptr;
        return *this;
    }

    /**
     * @brief Move the content from rhs by simply swaping pointers.
     * Note that the original data of this will be deleted and if it
     * is a pointer, the data pointed to won't be touched.
     * It's user's responsibility to manage the them and delete them
     * before the assignment operation.
     * 
     * @param  rhs  The content source to move from.
     * @return Any& This object, which is equivently to previous rhs.
     */
    Any &operator = (Any &&rhs) noexcept {
        if(this != &rhs) {
            this->~Any();
            ptr     = rhs.ptr;
            rhs.ptr = nullptr;
        }
        return *this;
    }
    /**
     * @brief Copy the content from rhs if the inner class
     * of rhs is copy-constructible.
     * Otherwise, this Any object will be set as nullptr.
     * Note that the original data of this will be deleted and if it
     * is a pointer, the data pointed to won't be touched.
     * It's user's responsibility to manage the them and delete them
     * before the assignment operation.
     * 
     * @param rhs The content source to copy from.
     */
    Any &operator = (const Any &rhs) noexcept {
        if(this != &rhs) {
            this->~Any();
            if(rhs.ptr) ptr = rhs.ptr->clone();
            else        ptr = nullptr;
        }
        return *this;
    }
    /**
     * @brief Copy the content from rhs if the inner class
     * of rhs is copy-constructible.
     * Otherwise, this Any object will be set as nullptr.
     * Note that the original data of this will be deleted and if it
     * is a pointer, the data pointed to won't be touched.
     * It's user's responsibility to manage the them and delete them
     * before the assignment operation.
     * 
     * @param rhs The content source to copy from.
     */
    Any &operator = (Any &rhs) noexcept { return *this = (const Any &)rhs; }
    /**
     * @brief Copy the content from rhs if the inner class
     * of rhs is copy-constructible.
     * Otherwise, this Any object will be set as nullptr.
     * Note that the original data of this will be deleted and if it
     * is a pointer, the data pointed to won't be touched.
     * It's user's responsibility to manage the them and delete them
     * before the assignment operation.
     * 
     * @param rhs The content source to copy from.
     */
    Any &operator = (const Any &&rhs) noexcept { return *this = rhs; }

    /* Perfect forwarding assignment for normal types (%Any/nullptr excluded). */
    template <class U>
    Any &operator = (U &&rhs) {
        using T = std::decay_t <U>;
        Data <T> *tmp = dynamic_cast <Data <T> *> (ptr);
        if(!tmp) {
            this->~Any();
            ptr = new Data <std::decay_t <U>> (std::forward <U> (rhs));
        } else {
            tmp->data = std::forward <U> (rhs);
        }
        return *this;
    }

    /* Force to change into a non-const type.
       If attempt failed, std::bad_cast will be thrown. */
    template <typename U>
    operator U() { return as <U> (); }

    /* Force to change into a const type.
       If attempt failed, std::bad_cast will be thrown. */
    template <typename U>
    operator const U() const { return as <const U> (); }

    /**
     * @brief  Test whether the inner type this %Any object hold is U.
     * @tparam U    Type to be tested.
     * @return true if the inner type this %Any object hold is U.
     */
    template <typename U>
    bool is() const noexcept {
        using T = std::decay_t <U>;
        Data <T> *tmp = dynamic_cast <Data <T> *> (ptr);
        return tmp;
    }
     
    /**
     * @brief  Tries to convert this %Any object into type U.
     * @tparam U       Type to be converted into.
     * @return Decayed type reference to value stored.
     * @exception std::bad_cast if the inner class isn't U.
     */
    template <typename U>
    std::decay_t <U> &as() {
        using T = std::decay_t <U>;
        Data <T> *tmp = dynamic_cast <Data <T> *> (ptr);
        if(!tmp) throw std::bad_cast();
        return tmp->data;
    }

     /**
     * @brief  Tries to convert this %Any object into type U.
     * @tparam U       Type to be converted into.
     * @return Decayed type const reference to value stored.
     * @exception std::bad_cast if the inner class isn't U.
     */
    template <typename U>
    const std::decay_t <U> &as() const {
        using T = std::decay_t <U>;
        Data <T> *tmp = dynamic_cast <Data <T> *> (ptr);
        if(!tmp) throw std::bad_cast();
        return tmp->data;
    }

    /* Test whether the inner pointer is nullptr. */
    bool isNull()    const noexcept {return !ptr;}
    /* Test whether the inner pointer refers a value. */
    bool isNotNull() const noexcept {return  ptr;}
};

}



#endif
