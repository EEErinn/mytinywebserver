// // #include <stdio.h>

// // // #include "../src/log/a.h"
// // #include "../src/log/singleton.h"

// // class A : public Singleton<A> {
// //    private:
// //     // A() { printf("construct a new A with name %d\n", 1); }
// //     A(int i) { printf("construct a new A with name %d\n", i); }
// //     ~A() { printf("deconstuctor called\n"); }
// //     friend Singleton<A>;
// // };

// // int main() {
// //     A& a = A::getInstance(2);
// //     (void)a;
// //     A& b = A::getInstance(3);
// //     (void)b;
// //     // std::cout << (instance == nullptr) << std::endl;
// //     // SingletonE* a = SingletonE::getInstance();
// //     // std::cout << a->a << " " << a->b << std::endl;
// //     // std::cout << SingletonE::a << " " << SingletonE::b << std::endl;
// // }

// #include <stdio.h>

// // #include "../src/log/a.h"
// #include "../src/log/singleton.h"
// class A : public Singleton<A> {
//    private:
//     A(const std::string && name) {
//         printf("construct  name %s\n", name.c_str());
//     }
//     A(int i) { printf("construct a new A with name %d\n", i); }
//     // A(const std::string& name) {
//     //     printf("construct a new A with name %s\n", name.c_str());
//     // }
//     ~A() { printf("deconstuctor called\n"); }
//     friend Singleton<A>;
// };

// int main() {
//     // A *a = A::getInstance(std::string("chen"));
//     // const std::string s = "xi";
//     // A *b = A::getInstance(s);
//     // A *b = A::getInstance(std::string("xii"));
//     // A::show("chen");  // "chen" type const char*
//     // A::show("xi");

//     A* a = A::getInstance(1);
//     A* b = A::getInstance(2);
//     // A &a = A::GetInst(std::string("chen"));
//     // A &b =
//     //     A::GetInst(std::string("xi"));  // 非常邪门的是，这样的话构造两次
//     //     但如果
//     // GetInst("a")  和 GetInst("b")构造一次
//     (void)a;
//     (void)b;
//     return 0;
// }