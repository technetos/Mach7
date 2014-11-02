//
//  Mach7: Pattern Matching Library for C++
//
//  Copyright 2011-2013, Texas A&M University.
//  Copyright 2014 Yuriy Solodkyy.
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//      * Redistributions of source code must retain the above copyright
//        notice, this list of conditions and the following disclaimer.
//
//      * Redistributions in binary form must reproduce the above copyright
//        notice, this list of conditions and the following disclaimer in the
//        documentation and/or other materials provided with the distribution.
//
//      * Neither the names of Mach7 project nor the names of its contributors
//        may be used to endorse or promote products derived from this software
//        without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY
//  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

///
/// \file
///
/// This file is a part of Mach7 library test suite.
///
/// \author Yuriy Solodkyy <yuriy.solodkyy@gmail.com>
///
/// \see https://parasol.tamu.edu/xtl/
/// \see https://parasol.tamu.edu/mach7/
/// \see https://github.com/solodon4/Mach7
/// \see https://github.com/solodon4/SELL
///

#include <iostream>
#include "boost/variant.hpp"
#include "type_switchN-patterns-xtl.hpp"
#include "patterns/all.hpp"

#define VARIANT_P(params) BOOST_PP_ENUM_PARAMS(10, params)

// NOTE: We declare vtbl_of in boost namespace because we want it to be found 
//       via two-phase name lookup due to its argument boost::variant
namespace boost
{
#if XTL_SUPPORT(variadics)
    // Default implementation of vtbl_of grabs sizeof(intptr_t) bytes from the beginning
    // of the object, which on 64-bit machines is larger that size of variant's which member.
    template <class... Ts>
    inline std::intptr_t vtbl_of(const boost::variant<Ts...>* p) { return p->which(); }
#else
    template <VARIANT_P(class Ts)>
    inline std::intptr_t vtbl_of(const boost::variant<VARIANT_P(Ts)>* p) { return p->which(); }
#endif
}

namespace xtl
{
#if XTL_SUPPORT(variadics)
    template <class... Ts>
    struct is_poly_morphic<boost::variant<Ts...>>
    {
        static const bool value = true;
    };

    template <class S, class T, class... Ts>
    struct is_subtype<S,boost::variant<T,Ts...>>
    {
        static const bool value = is_subtype<S,T>::value
                               || is_subtype<S,boost::variant<Ts...>>::value;
    };
#else
    template <VARIANT_P(class Ts)>
    struct is_poly_morphic<boost::variant<VARIANT_P(Ts)>>
    {
        static const bool value = true;
    };

    template <class S, class T, VARIANT_P(class Ts)>
    struct is_subtype<S,boost::variant<T,VARIANT_P(Ts)>>
    {
        static const bool value = is_subtype<S,T>::value
                               || is_subtype<S,boost::variant<VARIANT_P(Ts)>>::value;
    };
#endif

    template <class S, class T>
    struct is_subtype<S,boost::variant<T>> : is_subtype<S,T> {};

#if XTL_SUPPORT(variadics)
	template <class... Ts, class S>
    boost::variant<Ts...> subtype_cast_impl(target<boost::variant<Ts...>>, const S& s)
    {
        return boost::variant<Ts...>(s); // FIX: Actually this should be boost::variant<Ts...>(xtl::subtype_cast<Ti>(s)) where S <: Ti
    }
#else
    template <VARIANT_P(class Ts), class S>
    boost::variant<VARIANT_P(Ts)> subtype_cast_impl(target<boost::variant<VARIANT_P(Ts)>>, const S& s)
    {
        return boost::variant<VARIANT_P(Ts)>(s); // FIX: Actually this should be boost::variant<Ts...>(xtl::subtype_cast<Ti>(s)) where S <: Ti
    }
#endif

    template <typename T>
    struct is_subtype_visitor : public boost::static_visitor<T*>
    {
        template <typename S>
        T* operator()(S& s) const
        {
            return xtl::subtype_dynamic_cast<T*>(&s);
        }
    };

#if XTL_SUPPORT(variadics)
    template <class T, class... Ts>
    typename std::enable_if<xtl::is_subtype<T, boost::variant<Ts...>>::value, T*>::type
    subtype_dynamic_cast_impl(target<T*>, boost::variant<Ts...>* pv)
    {
        is_subtype_visitor<T> visitor;
        return boost::apply_visitor(visitor, *pv);
    }

    template <class T, class... Ts>
    typename std::enable_if<xtl::is_subtype<T, boost::variant<Ts...>>::value, const T*>::type
    subtype_dynamic_cast_impl(target<const T*>, const boost::variant<Ts...>* pv)
    {
        is_subtype_visitor<const T> visitor;
        return boost::apply_visitor(visitor, *pv);
    }
#else
    template <class T, VARIANT_P(class Ts)>
    typename std::enable_if<xtl::is_subtype<T, boost::variant<VARIANT_P(Ts)>>::value, T*>::type
    subtype_dynamic_cast_impl(target<T*>, boost::variant<VARIANT_P(Ts)>* pv)
    {
        is_subtype_visitor<T> visitor;
        return boost::apply_visitor(visitor, *pv);
    }

    template <class T, VARIANT_P(class Ts)>
    typename std::enable_if<xtl::is_subtype<T, boost::variant<VARIANT_P(Ts)>>::value, const T*>::type
    subtype_dynamic_cast_impl(target<const T*>, const boost::variant<VARIANT_P(Ts)>* pv)
    {
        is_subtype_visitor<const T> visitor;
        return boost::apply_visitor(visitor, *pv);
    }
#endif
}

static_assert(xtl::is_subtype<int,int>::value,"No reflexivity");

struct A             { int    a; virtual void foo() { std::cout << "A" << std::endl; } virtual void foo() const { std::cout << "const A" << std::endl; } A() {} };
struct B : A         { int    b; virtual void foo() { std::cout << "B" << std::endl; } virtual void foo() const { std::cout << "const B" << std::endl; } B() {} };
struct C : A         { double c; virtual void foo() { std::cout << "C" << std::endl; } virtual void foo() const { std::cout << "const C" << std::endl; } };
struct D : C, B      { float  d; virtual void foo() { std::cout << "D" << std::endl; } virtual void foo() const { std::cout << "const D" << std::endl; } };
struct X : virtual A { char   x; virtual void foo() { std::cout << "X" << std::endl; } virtual void foo() const { std::cout << "const X" << std::endl; } };
struct Y : virtual A { short  y; virtual void foo() { std::cout << "Y" << std::endl; } virtual void foo() const { std::cout << "const Y" << std::endl; } };
struct Z : X, Y      { char   z; virtual void foo() { std::cout << "Z" << std::endl; } virtual void foo() const { std::cout << "const Z" << std::endl; } };

static_assert( xtl::is_subtype<      B ,       A >::value, "      B  <:       A ");
static_assert( xtl::is_subtype<      B*,       A*>::value, "      B* <:       A*");
static_assert( xtl::is_subtype<const B*, const A*>::value, "const B* <: const A*");
static_assert( xtl::is_subtype<      B*, const A*>::value, "      B* <: const A*");
static_assert(!xtl::is_subtype<const B*,       A*>::value, "const B* <:       A*");

static_assert(!xtl::is_subtype<      A ,       B >::value, "      A  <:       B ");
static_assert(!xtl::is_subtype<      A*,       B*>::value, "      A* <:       B*");
static_assert(!xtl::is_subtype<const A*, const B*>::value, "const A* <: const B*");
static_assert(!xtl::is_subtype<      A*, const B*>::value, "      A* <: const B*");
static_assert(!xtl::is_subtype<const A*,       B*>::value, "const A* <:       B*");

int main()
{
    A a; const A ca;
    B b; const B cb;
    a = xtl::subtype_cast<A>(b);
    a = xtl::subtype_cast<A>(cb);
    //b = xtl::subtype_cast<B>(a); // error

    A* qa = xtl::subtype_cast<A*>(&b);
	const A* qaa= xtl::subtype_cast<const A*>(&cb);
    //B* qb = xtl::subtype_cast<B*>(&a); // error
    B* qb = xtl::subtype_dynamic_cast<B*>(qa);
    qa->foo();
    if (qb) qb->foo();

    std::cout << "===========" << std::endl;
    {
        A* pA = new A;
        B* pB = new B;
        C* pC = new C;
        D* pD = new D;
        X* pX = new X;
        Y* pY = new Y;
        Z* pZ = new Z;

        A* pa = xtl::subtype_cast<A*>(pA);
        A* pb = xtl::subtype_cast<A*>(pB);
        A* pc = xtl::subtype_cast<A*>(pC);
        A* pdb= xtl::subtype_cast<A*>((B*)pD);
        A* pdc= xtl::subtype_cast<A*>((C*)pD);
        A* px = xtl::subtype_cast<A*>(pX);
        A* py = xtl::subtype_cast<A*>(pY);
        A* pz = xtl::subtype_cast<A*>(pZ);

        if (A* p = xtl::subtype_dynamic_cast<A*>(pa)) p->foo();
        if (B* p = xtl::subtype_dynamic_cast<B*>(pb)) p->foo();
        if (C* p = xtl::subtype_dynamic_cast<C*>(pc)) p->foo();
        if (D* p = xtl::subtype_dynamic_cast<D*>(pdb))p->foo();
        if (D* p = xtl::subtype_dynamic_cast<D*>(pdc))p->foo();
        if (X* p = xtl::subtype_dynamic_cast<X*>(px)) p->foo();
        if (Y* p = xtl::subtype_dynamic_cast<Y*>(py)) p->foo();
        if (Z* p = xtl::subtype_dynamic_cast<Z*>(pz)) p->foo();
        if (B* p = xtl::subtype_dynamic_cast<B*>(pdc))p->foo();
        if (C* p = xtl::subtype_dynamic_cast<C*>(pdb))p->foo();

        if (const A* p = xtl::subtype_dynamic_cast<const A*>(pa)) p->foo();
        if (const B* p = xtl::subtype_dynamic_cast<const B*>(pb)) p->foo();
        if (const C* p = xtl::subtype_dynamic_cast<const C*>(pc)) p->foo();
        if (const D* p = xtl::subtype_dynamic_cast<const D*>(pdb))p->foo();
        if (const D* p = xtl::subtype_dynamic_cast<const D*>(pdc))p->foo();
        if (const X* p = xtl::subtype_dynamic_cast<const X*>(px)) p->foo();
        if (const Y* p = xtl::subtype_dynamic_cast<const Y*>(py)) p->foo();
        if (const Z* p = xtl::subtype_dynamic_cast<const Z*>(pz)) p->foo();
        if (const B* p = xtl::subtype_dynamic_cast<const B*>(pdc))p->foo();
        if (const C* p = xtl::subtype_dynamic_cast<const C*>(pdb))p->foo();
    }
    {
        const A* pA = new A;
        const B* pB = new B;
        const C* pC = new C;
        const D* pD = new D;
        const X* pX = new X;
        const Y* pY = new Y;
        const Z* pZ = new Z;

        const A* pa = xtl::subtype_cast<const A*>(pA);
        const A* pb = xtl::subtype_cast<const A*>(pB);
        const A* pc = xtl::subtype_cast<const A*>(pC);
        const A* pdb= xtl::subtype_cast<const A*>((B*)pD);
        const A* pdc= xtl::subtype_cast<const A*>((const C*)pD);
        const A* px = xtl::subtype_cast<const A*>(pX);
        const A* py = xtl::subtype_cast<const A*>(pY);
        const A* pz = xtl::subtype_cast<const A*>(pZ);

        if (const A* p = xtl::subtype_dynamic_cast<const A*>(pa)) p->foo();
        if (const B* p = xtl::subtype_dynamic_cast<const B*>(pb)) p->foo();
        if (const C* p = xtl::subtype_dynamic_cast<const C*>(pc)) p->foo();
        if (const D* p = xtl::subtype_dynamic_cast<const D*>(pdb))p->foo();
        if (const D* p = xtl::subtype_dynamic_cast<const D*>(pdc))p->foo();
        if (const X* p = xtl::subtype_dynamic_cast<const X*>(px)) p->foo();
        if (const Y* p = xtl::subtype_dynamic_cast<const Y*>(py)) p->foo();
        if (const Z* p = xtl::subtype_dynamic_cast<const Z*>(pz)) p->foo();
        if (const B* p = xtl::subtype_dynamic_cast<const B*>(pdc))p->foo();
        if (const C* p = xtl::subtype_dynamic_cast<const C*>(pdb))p->foo();

        if (const A* p = xtl::subtype_dynamic_cast<const A*>(pa)) p->foo();
        if (const B* p = xtl::subtype_dynamic_cast<const B*>(pb)) p->foo();
        if (const C* p = xtl::subtype_dynamic_cast<const C*>(pc)) p->foo();
        if (const D* p = xtl::subtype_dynamic_cast<const D*>(pdb))p->foo();
        if (const D* p = xtl::subtype_dynamic_cast<const D*>(pdc))p->foo();
        if (const X* p = xtl::subtype_dynamic_cast<const X*>(px)) p->foo();
        if (const Y* p = xtl::subtype_dynamic_cast<const Y*>(py)) p->foo();
        if (const Z* p = xtl::subtype_dynamic_cast<const Z*>(pz)) p->foo();
        if (const B* p = xtl::subtype_dynamic_cast<const B*>(pdc))p->foo();
        if (const C* p = xtl::subtype_dynamic_cast<const C*>(pdb))p->foo();
    }

    static_assert(xtl::is_subtype<B,A>::value,"Subclassing");

    static_assert(xtl::is_subtype<int,boost::variant<double,float,int,unsigned int*>>::value, "Not a subtype");

    typedef boost::variant<double,float,int,unsigned int*> my_variant_1;

    std::cout << "===========" << std::endl;
    {
        my_variant_1 v1 = xtl::subtype_cast<my_variant_1>(3.1415);
        std::cout << '(' << v1.which() << ',' << v1 << ')' << std::endl;

        my_variant_1 v2 = xtl::subtype_cast<my_variant_1>(42);
        std::cout << '(' << v2.which() << ',' << v2 << ')' << std::endl;

        std::cout << "-----------" << std::endl;

        double* p1 = xtl::subtype_dynamic_cast<double*>(&v1); std::cout << '(' << p1 << ',' << (p1 ? *p1 : 99999) << ')' << std::endl;
        float*  p2 = xtl::subtype_dynamic_cast<float* >(&v1); std::cout << '(' << p2 << ',' << (p2 ? *p2 : 88888) << ')' << std::endl;
        int*    p3 = xtl::subtype_dynamic_cast<int*   >(&v1); std::cout << '(' << p3 << ',' << (p3 ? *p3 : 77777) << ')' << std::endl;

        double* q1 = xtl::subtype_dynamic_cast<double*>(&v2); std::cout << '(' << q1 << ',' << (q1 ? *q1 : 66666) << ')' << std::endl;
        float*  q2 = xtl::subtype_dynamic_cast<float* >(&v2); std::cout << '(' << q2 << ',' << (q2 ? *q2 : 55555) << ')' << std::endl;
        int*    q3 = xtl::subtype_dynamic_cast<int*   >(&v2); std::cout << '(' << q3 << ',' << (q3 ? *q3 : 44444) << ')' << std::endl;

        std::cout << "sizeof(intptr_t)"  << sizeof(std::intptr_t) << std::endl;
        std::cout << "sizeof(v1.which_)" << sizeof(v1.which())    << std::endl;

    }

    std::cout << "===========" << std::endl;
    {
        const my_variant_1 v1 = xtl::subtype_cast<my_variant_1>(3.1415);
        std::cout << '(' << v1.which() << ',' << v1 << ')' << std::endl;

        const my_variant_1 v2 = xtl::subtype_cast<my_variant_1>(42);
        std::cout << '(' << v2.which() << ',' << v2 << ')' << std::endl;

        std::cout << "-----------" << std::endl;

        const double* p1 = xtl::subtype_dynamic_cast<const double*>(&v1); std::cout << '(' << p1 << ',' << (p1 ? *p1 : 99999) << ')' << std::endl;
        const float*  p2 = xtl::subtype_dynamic_cast<const float* >(&v1); std::cout << '(' << p2 << ',' << (p2 ? *p2 : 88888) << ')' << std::endl;
        const int*    p3 = xtl::subtype_dynamic_cast<const int*   >(&v1); std::cout << '(' << p3 << ',' << (p3 ? *p3 : 77777) << ')' << std::endl;

        const double* q1 = xtl::subtype_dynamic_cast<const double*>(&v2); std::cout << '(' << q1 << ',' << (q1 ? *q1 : 66666) << ')' << std::endl;
        const float*  q2 = xtl::subtype_dynamic_cast<const float* >(&v2); std::cout << '(' << q2 << ',' << (q2 ? *q2 : 55555) << ')' << std::endl;
        const int*    q3 = xtl::subtype_dynamic_cast<const int*   >(&v2); std::cout << '(' << q3 << ',' << (q3 ? *q3 : 44444) << ')' << std::endl;
    }

    using namespace mch;

    var<double> d;
    var<float>  f;
    var<int>    n;
    var<unsigned int*> p;
    std::cout << "===========" << std::endl;

    my_variant_1 vars[3] = {42, 3.14f, 9.876};

    for (int k = 0; k < 2; ++k)
    for (int i = 0; i < 3; ++i)
    {
        Match(vars[i])
        {
            Case(mch::C<double>(d)) std::cout << "double " << d << std::endl; match0 += 1.0; break;
            Case(mch::C<float> (f)) std::cout << "float  " << f << std::endl; match0 -= 1.0; break;
            Case(mch::C<int>   (n)) std::cout << "int    " << n << std::endl; match0++;      break;
        }
        EndMatch
    }

    const my_variant_1 vars2[6] = {42, 3.14f, 9.876, 7, 2.72f, 1.2345};

    for (int i = 0; i < 6; ++i)
    {
        Match(vars2[i])
        {
            Case(mch::C<double>(d)) std::cout << "double " << d << std::endl; break;
            Case(mch::C<float> (f)) std::cout << "float  " << f << std::endl; break;
            Case(mch::C<int>   (n)) std::cout << "int    " << n << std::endl; break;
//            Case(mch::C<unsigned int*>(p)) std::cout << "unsigned int* " << p << "->" << *p << std::endl; break; // FIX: fails to compile
        }
        EndMatch
    }

}