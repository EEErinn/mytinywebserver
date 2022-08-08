#include <functional>
#include <iostream>
#include <memory>

using namespace std;

function<void()> callback;  //回调函数

class Testbind : public enable_shared_from_this<Testbind> {
   public:
    // 当使用shared_ptr
    // ptr构造对象时，enable_shared_from_this的weak_ptr已经初始化好了。
    // 可以使用ptr也可以使用this来调用，因为此时的this和ptr只向同一块内存。
    Testbind() { callback = std::bind(&Testbind::handleRead, this); }

    shared_ptr<Testbind> get_ptr() {
        return shared_from_this();  //返回一个智能指针,
    }

    using ReadCallBack = std::function<void(const shared_ptr<Testbind>&)>;
    void setReadCallBack(ReadCallBack v) { readCallBack = v; }

    void handleRead() {  //用户函数
        readCallBack(this->shared_from_this());
    }

    ReadCallBack readCallBack;
};

void onRead(const shared_ptr<Testbind>& conn) {
    cout << "onRead" << conn.use_count() << endl;
}

/**
 * @brief 打印3处，Error: terminate called after throwing an instance of
 * 'std::bad_weak_ptr'
 * 需要使用shared_from_this时，创建类的对象需要使用智能指针包裹，不能直接用new创建。
 * shared_from_this()是enable_shared_from_this<T>的成员函数，返回shared_ptr<T>；
 * 注意的是，这个函数仅在shared_ptr<T>的构造函数被调用之后才能使用。
 * 原因是enable_shared_from_this::weak_ptr并不在构造函数中设置，而是在shared_ptr<T>的构造函数中设置。
 * 因此，如果想在类内部用 shared_from_this，this 必须是一个
 * shared_ptr包装的指针才可以。
 * */

int main() {
    auto obj = new Testbind();
    shared_ptr<Testbind> testb(obj);
    cout << testb.use_count() << endl;  //打印1
    // testb->setReadCallBack(onRead);
    // cout << testb.use_count() << endl;  //打印1
    // callback = std::bind(&Testbind::handleRead, obj);
    // callback();
    // cout << testb.use_count() << endl;  //打印1
    auto anotherptr = testb->get_ptr();
    cout << testb.use_count() << endl;  //打印2
    auto otherptr = testb->get_ptr();
    cout << testb.use_count() << endl;  //打印2

    // https://blog.csdn.net/yuhan61659/article/details/81984340
    // auto other = new Testbind();
    // shared_ptr<Testbind> testa(other);
    // auto b = other->get_ptr();
    // cout << b.use_count() << endl;  //打印3
    // callback = bind(&Testbind::testfun,
    //                 anotherptr);  //绑定用户函数和智能指针到回调函数上
    // cout << anotherptr.use_count() << endl;  //打印4
    // callback = NULL;
    // cout << testb.use_count() << endl;  //打印5
    return 0;
}
