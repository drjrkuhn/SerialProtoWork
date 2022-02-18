// _CppTests.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <cassert>
#include <iostream>

template <class DERIVED, typename S>
struct StreamAdapter
{
    StreamAdapter(S& stream) : stream_(stream) {
        std::cout << "==Constructed StreamAdapter==" << std::endl;
    }
    virtual ~StreamAdapter()
    {
        std::cout << "==Destructed StreamAdapter==" << std::endl;
    }
    void send(char* buf, int length)
    {
        static_cast<DERIVED*>(this)->send_impl(buf, length);
    }
    //void send_impl(char* buf, int length)   {       assert(false);   }
    S& stream_;
};

template<typename S>
struct OStreamAdapter : public StreamAdapter<OStreamAdapter<S>,S>
{
    typedef StreamAdapter<OStreamAdapter<S>,S> BASE;
    OStreamAdapter(S& out) : BASE(out) {
        std::cout << "==Constructed Derived OStreamAdapter==" << std::endl;
    }
    virtual ~OStreamAdapter()
    {
        std::cout << "==Destructed Derived OStreamAdapter==" << std::endl;
    }

    void send_impl(char* buf, int length) {
        //StreamAdapter<S>::send(buf, length);
        stream_.write(buf, length);
    }
    using BASE::stream_;
//    S& out_;
};

template <typename S>
struct type_return {    typedef S type; };

template <typename S>
class Base
{
 public:
    Base(S value) : value_(value)
    {
    }

    virtual void increment() = 0;

 protected:
    S value_;
};


template <typename S>
class Derived : public Base<S>
{
 public:
    Derived(int initial) : Base<S>(initial) {}
    void increment()
    {
        value_++;
    }
    S value() {
        return value_;
    }

 protected:
     using Base<S>::value_;
};

int main()
{
    using namespace std;
    Derived<int> t(100);
    t.increment();
    t.increment();
    cout << "after increment t.value() is " << t.value() << endl;
    cout << "Hello World!" << endl;

    OStreamAdapter<ostream> output(cout);
    char buf[]{"FooBar"};
    output.send(buf, 3);
    cout << endl;

}
