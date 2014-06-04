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

#include <vtkm/testing/Testing.h>

#include <string>

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

void Test()
{
  TestBaseListTags();
  TestCustomTypeLists();
}

} // anonymous namespace

int ListTags(int, char *[])
{
  return vtkm::testing::Testing::Run(Test);
}
