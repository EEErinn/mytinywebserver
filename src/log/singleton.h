#pragma once


// #include <functional>
#include <iostream>
// #include <memory>
#include <mutex>
// #include <utility>

// 饿汉模式: 线程安全，但即使不调用用getInstance(),
// instance也会被创建，这样会占用内存资源。
// 饿汉模式二：把对象放在静态区，不使用new。因此，不需要在类中释放内存。
// template <class T>
// class Singleton {
//    public:
//     // 局部静态变量
//     static T* getInstance() {
//         static T instance;
//         return &instance;
//     }

//    protected:
//     // 非公有，防止外界创建实例；
//     // protected singleton需要成为T的友元，调用T的构造函数
//     Singleton() {}
//     ~Singleton() {}

//    private:
//     // 禁止拷贝和移动
//     // 拷贝构造函数
//     Singleton(const Singleton&) = delete;
//     // 拷贝赋值函数
//     Singleton& operator=(const Singleton&) = delete;
//     // 移动拷贝函数
//     Singleton(Singleton&&) = delete;
//     // 移动赋值函数
//     Singleton& operator=(Singleton&&) = delete;
// };

// 饿汉模式三：把对象放在堆区，使用new。因此，需要在类中释放内存。
// template <class T>
// class SingletonNew {
//    public:
//     static T* getInstance() { return instance; }

//    private:
//     class Garb {
//        public:
//         ~Garb() {
//             if (instance) {
//                 delete instance;
//                 instance = nullptr;
//             }
//         }
//     };
//     static T* instance = new T();
//     static Garb garb;

//    protected:
//     SingletonNew() {}
//     ~SingletonNew() {}
// };

// 带有参数的单例模板类
template <typename T>
class Singleton {
   public:
    template <typename... Args>
    static T* getInstance(Args&&... args) {
        if (nullptr == instance_) {
            mutex_.lock();
            if (nullptr == instance_) {
                instance_ = new T(std::forward<Args>(args)...);
            }
            mutex_.unlock();
        }
        return instance_;
    }

   protected:
    class Garb {
       public:
        ~Garb() {
            if (instance_) {
                delete instance_;
                instance_ = nullptr;
            }
        }
    };

    Singleton() = default;
    virtual ~Singleton() = default;
    Singleton(const Singleton&) = delete;
    Singleton(const Singleton&&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton& operator=(Singleton&&) = delete;

   private:
    static T* instance_;
    static Garb garb;
    static std::mutex mutex_;
};

template <typename T>
T* Singleton<T>::instance_ = nullptr;

template <typename T>
std::mutex Singleton<T>::mutex_;

// template <typename T>
// class Singleton {
//    public:
//     void operator=(const T&) = delete;
//     template <typename... Args>
//     static T& getInstance(const Args&... args) {
//         static T instance_(args...);
//         return instance_;
//     }

//    protected:
//     Singleton() = default;
//     virtual ~Singleton() = default;
//     Singleton(const Singleton&) = delete;
//     Singleton(const Singleton&&) = delete;
//     // Singleton& operator=(const T&) = delete;
//     // Singleton& operator=(Singleton&&) = delete;
// };
// #include <string>
// #include <type_traits>
// #include <typeinfo>

// template <typename T>
// std::string type_name() {
//     typedef typename std::remove_reference<T>::type TR;
//     std::string r = typeid(TR).name();
//     if (std::is_const<TR>::value) r += " const";
//     if (std::is_volatile<TR>::value) r += " volatile";
//     if (std::is_lvalue_reference<T>::value)
//         r += "&";
//     else if (std::is_rvalue_reference<T>::value)
//         r += "&&";
//     return r;
// }

// template <typename T>
// class Singleton {
//    protected:
//     Singleton() = default;
//     Singleton(const Singleton&) = delete;
//     Singleton(const Singleton&&) = delete;
//     Singleton& operator=(const Singleton&) = delete;
//     Singleton& operator=(Singleton&&) = delete;
//     virtual ~Singleton() = default;

//     // class Garb {
//     //    public:
//     //     ~Garb() {
//     //         if (instance_) {
//     //             delete instance_;
//     //             instance_ = nullptr;
//     //         }
//     //     }
//     // };

//    public:
//     static void show() {}
//     template <typename P, typename... Args>
//     static void show(P value, Args... args) {
//         std::cout << type_name<P>() << " " << value << std::endl;
//         show(args...);
//     }

//     template <typename... Args>
//     static T* getInstance(Args&&... args) {
//         static T* inst = new T(std::forward<Args>(args)...);
//         return inst;
//     }

//    private:
//     static T* instance_;
//     // static Garb garb;
// };

// template <typename T>
// T* Singleton<T>::instance_ = nullptr;

// template <typename T>
// class Singleton {
//    public:
//     Singleton(const Singleton&) = delete;
//     Singleton& operator=(const Singleton&) = delete;

//     template <typename... Args>
//     static T* getInstance(Args&&... args) {
//         std::call_once(
//             once_, Initialize<Args...>, std::forward<Args>(args)...);
//         // static T instance_(args...);
//         return instance_;
//     }

//    protected:
//     Singleton() = default;
//     ~Singleton() = default;
    
//     Singleton(const Singleton&&) = delete;
//     Singleton& operator=(Singleton&&) = delete;

//    private:
//     template <typename... Args>
//     static void Initialize(Args&&... args) {
//         if (nullptr != instance_) {
//             return;
//         }
//         instance_ = new T(std::forward<Args>(args)...);
//         std::atexit(Destroy);
//     }
//     static void Destroy() { delete instance_; }

//     static T* instance_;
//     static std::once_flag once_;
// };

// template <typename T>
// T* Singleton<T>::instance_ = nullptr;

// template <typename T>
// std::once_flag Singleton<T>::once_;

// 饿汉模式

// class SingletonE {
//    private:
//     static SingletonE* instance_;

//    public:
//     static int a;
//     static int b;
//     static SingletonE* getInstance() { return instance_; }

//    private:
//     // 构造函数
//     SingletonE() {
//         a = 1;
//         b = 2;
//     }
//     // virtual ~SingletonE() = default;
//     SingletonE(const SingletonE&) = delete;
//     SingletonE& operator=(const SingletonE&) = delete;
// };
