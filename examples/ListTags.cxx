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
struct Vectors2List : vtkm::ListTagBase2<vtkm::Id2, vtkm::Vector2> {  };

// An application that uses 2D geometry might commonly encounter this list of
// types.
struct MyCommonTypes :
    vtkm::ListTagJoin<Vectors2List,vtkm::TypeListTagCommon> {  };
////
//// END-EXAMPLE CustomTypeLists.cxx
////

} // anonymous namespace

#include <vtkm/VectorTraits.h>

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

struct FooBarList : vtkm::ListTagBase2<Foo,Bar> {  };

struct BazQuxXyzzyList : vtkm::ListTagBase3<Baz,Qux,Xyzzy> {  };

struct QuxBazBarFooList : vtkm::ListTagBase4<Qux,Baz,Bar,Foo> {  };

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

  void operator()(Foo) { this->FoundTags.append("Foo"); }
  void operator()(Bar) { this->FoundTags.append("Bar"); }
  void operator()(Baz) { this->FoundTags.append("Baz"); }
  void operator()(Qux) { this->FoundTags.append("Qux"); }
  void operator()(Xyzzy) { this->FoundTags.append("Xyzzy"); }
  void operator()(vtkm::Id) { this->FoundTags.append("Id"); }
  void operator()(vtkm::Id2) { this->FoundTags.append("Id2"); }
  void operator()(vtkm::Id3) { this->FoundTags.append("Id3"); }
  void operator()(vtkm::Scalar) { this->FoundTags.append("Scalar"); }
  void operator()(vtkm::Vector2) { this->FoundTags.append("Vector2"); }
  void operator()(vtkm::Vector3) { this->FoundTags.append("Vector3"); }
  void operator()(vtkm::Vector4) { this->FoundTags.append("Vector4"); }
};

template<typename ListTag>
void TryListTag(ListTag, const char *expectedString)
{
  ListTagsFunctor checkFunctor;
  vtkm::ListForEach(checkFunctor, ListTag());
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
  TryListTag(Vectors2List(), "Id2Vector2");
  TryListTag(MyCommonTypes(), "Id2Vector2IdScalarVector3");
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
  T sum(typename vtkm::VectorTraits<T>::ComponentType(0));
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
    VTKM_TEST_ASSERT(array.Array[index] = index+1, "Got bad prefix sum.");
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
