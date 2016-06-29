////
//// BEGIN-EXAMPLE CustomTypeLists.cxx
////
#define VTKM_DEFAULT_TYPE_LIST_TAG MyCommonTypes

#include <vtkm/ListTag.h>
#include <vtkm/TypeListTag.h>
//// PAUSE-EXAMPLE
namespace {
//// RESUME-EXAMPLE

// A list of 2D vector types.
struct Vec2List
    : vtkm::ListTagBase<vtkm::Id2,
                        vtkm::Vec<vtkm::Float32,2>,
                        vtkm::Vec<vtkm::Float64,2> > {  };

// An application that uses 2D geometry might commonly encounter this list of
// types.
struct MyCommonTypes : vtkm::ListTagJoin<Vec2List,vtkm::TypeListTagCommon> {  };
////
//// END-EXAMPLE CustomTypeLists.cxx
////

} // anonymous namespace

#include <vtkm/VecTraits.h>

#include <vtkm/testing/Testing.h>

#include <algorithm>
#include <string>
#include <vector>

////
//// BEGIN-EXAMPLE BaseListTags.cxx
////
#include <vtkm/ListTag.h>
//// PAUSE-EXAMPLE
namespace {
//// RESUME-EXAMPLE

// Placeholder classes representing things that might be in a template
// metaprogram list.
class Foo;
class Bar;
class Baz;
class Qux;
class Xyzzy;

// The names of the following tags are indicative of the lists they contain.

struct FooList : vtkm::ListTagBase<Foo> {  };

struct FooBarList : vtkm::ListTagBase<Foo,Bar> {  };

struct BazQuxXyzzyList : vtkm::ListTagBase<Baz,Qux,Xyzzy> {  };

struct QuxBazBarFooList : vtkm::ListTagBase<Qux,Baz,Bar,Foo> {  };

struct FooBarBazQuxXyzzyList
    : vtkm::ListTagJoin<FooBarList, BazQuxXyzzyList> {  };
////
//// END-EXAMPLE BaseListTags.cxx
////

class Foo {};
class Bar {};
class Baz {};
class Qux {};
class Xyzzy {};

struct ListTagsFunctor
{
  std::string FoundTags;

  template<typename T>
  void operator()(T) {
    this->FoundTags.append(vtkm::testing::TypeName<T>::Name());
  }

  void operator()(Foo) { this->FoundTags.append("Foo"); }
  void operator()(Bar) { this->FoundTags.append("Bar"); }
  void operator()(Baz) { this->FoundTags.append("Baz"); }
  void operator()(Qux) { this->FoundTags.append("Qux"); }
  void operator()(Xyzzy) { this->FoundTags.append("Xyzzy"); }
};

template<typename ListTag>
void TryListTag(ListTag, const char *expectedString)
{
  ListTagsFunctor checkFunctor;
  vtkm::ListForEach(checkFunctor, ListTag());
  std::cout << std::endl
            << "Expected " << expectedString << std::endl
            << "Found    " << checkFunctor.FoundTags << std::endl;
  VTKM_TEST_ASSERT(checkFunctor.FoundTags == expectedString, "List wrong");
}

void TestBaseListTags()
{
  TryListTag(FooList(), "Foo");
  TryListTag(FooBarList(), "FooBar");
  TryListTag(BazQuxXyzzyList(), "BazQuxXyzzy");
  TryListTag(QuxBazBarFooList(), "QuxBazBarFoo");
  TryListTag(FooBarBazQuxXyzzyList(), "FooBarBazQuxXyzzy");
}

void TestCustomTypeLists()
{
#ifdef VTKM_USE_64BIT_IDS
  TryListTag(Vec2List(), "vtkm::Vec< vtkm::Int64, 2 >vtkm::Vec< vtkm::Float32, 2 >vtkm::Vec< vtkm::Float64, 2 >");
  TryListTag(MyCommonTypes(), "vtkm::Vec< vtkm::Int64, 2 >vtkm::Vec< vtkm::Float32, 2 >vtkm::Vec< vtkm::Float64, 2 >vtkm::Int32vtkm::Int64vtkm::Float32vtkm::Float64vtkm::Vec< vtkm::Float32, 3 >vtkm::Vec< vtkm::Float64, 3 >");
#else
  TryListTag(Vec2List(), "vtkm::Vec< vtkm::Int32, 2 >vtkm::Vec< vtkm::Float32, 2 >vtkm::Vec< vtkm::Float64, 2 >");
  TryListTag(MyCommonTypes(), "vtkm::Vec< vtkm::Int32, 2 >vtkm::Vec< vtkm::Float32, 2 >vtkm::Vec< vtkm::Float64, 2 >vtkm::Int32vtkm::Int64vtkm::Float32vtkm::Float64vtkm::Vec< vtkm::Float32, 3 >vtkm::Vec< vtkm::Float64, 3 >");
#endif
}

////
//// BEGIN-EXAMPLE ListForEach.cxx
////
struct MyArrayBase {
  // A virtual destructor makes sure C++ RTTI will be generated. It also helps
  // ensure subclass destructors are called.
  virtual ~MyArrayBase() {  }
};

template<typename T>
struct MyArrayImpl : public MyArrayBase {
  std::vector<T> Array;
};

template<typename T>
void PrefixSum(std::vector<T> &array)
{
  T sum(typename vtkm::VecTraits<T>::ComponentType(0));
  for (typename std::vector<T>::iterator iter = array.begin();
       iter != array.end();
       iter++)
  {
    sum = sum + *iter;
    *iter = sum;
  }
}

struct PrefixSumFunctor {
  MyArrayBase *ArrayPointer;

  PrefixSumFunctor(MyArrayBase *arrayPointer) : ArrayPointer(arrayPointer) {  }

  template<typename T>
  void operator()(T) {
    typedef MyArrayImpl<T> ConcreteArrayType;
    ConcreteArrayType *concreteArray =
        dynamic_cast<ConcreteArrayType *>(this->ArrayPointer);
    if (concreteArray != NULL)
    {
      PrefixSum(concreteArray->Array);
    }
  }
};

void DoPrefixSum(MyArrayBase *array)
{
  PrefixSumFunctor functor = PrefixSumFunctor(array);
  vtkm::ListForEach(functor, vtkm::TypeListTagCommon());
}
////
//// END-EXAMPLE ListForEach.cxx
////

void TestPrefixSum()
{
  MyArrayImpl<vtkm::Id> array;
  array.Array.resize(10);
  std::fill(array.Array.begin(), array.Array.end(), 1);
  DoPrefixSum(&array);
  for (vtkm::Id index = 0; index < 10; index++)
  {
    VTKM_TEST_ASSERT(array.Array[index] == index+1, "Got bad prefix sum.");
  }
}

void Test()
{
  TestBaseListTags();
  TestCustomTypeLists();
  TestPrefixSum();
}

} // anonymous namespace

int ListTags(int, char *[])
{
  return vtkm::testing::Testing::Run(Test);
}
