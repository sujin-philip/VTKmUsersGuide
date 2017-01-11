#include <vtkm/TypeTraits.h>
#include <vtkm/VecTraits.h>

#include <vtkm/testing/Testing.h>

#include <typeinfo>

////
//// BEGIN-EXAMPLE TypeTraits.cxx
////
#include <vtkm/TypeTraits.h>

#include <vtkm/Math.h>
//// PAUSE-EXAMPLE
namespace TraitsExamples {
//// RESUME-EXAMPLE

template<typename T>
T AnyRemainder(const T &numerator, const T &denominator);

namespace detail {

template<typename T>
T AnyRemainderImpl(const T &numerator,
                   const T &denominator,
                   vtkm::TypeTraitsIntegerTag,
                   vtkm::TypeTraitsScalarTag)
{
  return numerator % denominator;
}

template<typename T>
T AnyRemainderImpl(const T &numerator,
                   const T &denominator,
                   vtkm::TypeTraitsRealTag,
                   vtkm::TypeTraitsScalarTag)
{
  // The VTK-m math library contains a Remainder function that operates on
  // floating point numbers.
  return vtkm::Remainder(numerator, denominator);
}

template<typename T, typename NumericTag>
T AnyRemainderImpl(const T &numerator,
                   const T &denominator,
                   NumericTag,
                   vtkm::TypeTraitsVectorTag)
{
  T result;
  for (int componentIndex = 0;
       componentIndex < T::NUM_COMPONENTS;
       componentIndex++)
  {
    result[componentIndex] =
        AnyRemainder(numerator[componentIndex], denominator[componentIndex]);
  }
  return result;
}

} // namespace detail

template<typename T>
T AnyRemainder(const T &numerator, const T &denominator)
{
  return detail::AnyRemainderImpl(
        numerator,
        denominator,
        typename vtkm::TypeTraits<T>::NumericTag(),
        typename vtkm::TypeTraits<T>::DimensionalityTag());
}
////
//// END-EXAMPLE TypeTraits.cxx
////

void TryRemainder()
{
  vtkm::Id m1 = AnyRemainder(7, 3);
  VTKM_TEST_ASSERT(m1 == 1, "Got bad remainder");

  vtkm::Float32 m2 = AnyRemainder(7.0f, 3.0f);
  VTKM_TEST_ASSERT(test_equal(m2, 1), "Got bad remainder");

  vtkm::Id3 m3 = AnyRemainder(vtkm::Id3(10, 9, 8), vtkm::Id3(7, 6, 5));
  VTKM_TEST_ASSERT(test_equal(m3, vtkm::Id3(3, 3, 3)), "Got bad remainder");

  vtkm::Vec<vtkm::Float32,3> m4 =
      AnyRemainder(vtkm::make_Vec(10, 9, 8), vtkm::make_Vec(7, 6, 5));
  VTKM_TEST_ASSERT(test_equal(m4, vtkm::make_Vec(3, 3, 3)), "Got bad remainder");
}

template<typename T>
struct TypeTraits;

////
//// BEGIN-EXAMPLE TypeTraitsImpl.cxx
////
//// PAUSE-EXAMPLE
#if 0
//// RESUME-EXAMPLE
namespace vtkm {
//// PAUSE-EXAMPLE
#endif
//// RESUME-EXAMPLE

template<>
struct TypeTraits<vtkm::Float32>
{
  using NumericTag = vtkm::TypeTraitsRealTag;
  using DimensionalityTag = vtkm::TypeTraitsScalarTag;

  VTKM_EXEC_CONT
  static vtkm::Float32 ZeroInitialization() { return vtkm::Float32(0); }
};

//// PAUSE-EXAMPLE
#if 0
//// RESUME-EXAMPLE
}
//// PAUSE-EXAMPLE
#endif
//// RESUME-EXAMPLE
////
//// END-EXAMPLE TypeTraitsImpl.cxx
////

void TryCustomTypeTraits()
{
  using CustomTraits = TraitsExamples::TypeTraits<vtkm::Float32>;
  using OriginalTraits = vtkm::TypeTraits<vtkm::Float32>;

  VTKM_STATIC_ASSERT((std::is_same<
                        CustomTraits::NumericTag,
                        OriginalTraits::NumericTag>::value));
  VTKM_STATIC_ASSERT((std::is_same<
                        CustomTraits::DimensionalityTag,
                        OriginalTraits::DimensionalityTag>::value));

  VTKM_TEST_ASSERT(CustomTraits::ZeroInitialization()
                   == OriginalTraits::ZeroInitialization(),
                   "Bad zero initialization.");
}

} // namespace TraitsExamples

////
//// BEGIN-EXAMPLE VecTraits.cxx
////
#include <vtkm/VecTraits.h>
//// PAUSE-EXAMPLE
namespace TraitsExamples {
//// RESUME-EXAMPLE

// This functor provides a total ordering of vectors. Every compared vector
// will be either less, greater, or equal (assuming all the vector components
// also have a total ordering).
template<typename T>
struct LessTotalOrder
{
  VTKM_EXEC_CONT
  bool operator()(const T &left, const T &right)
  {
    for (int index = 0; index < vtkm::VecTraits<T>::NUM_COMPONENTS; index++)
    {
      typedef typename vtkm::VecTraits<T>::ComponentType ComponentType;
      const ComponentType &leftValue =
          vtkm::VecTraits<T>::GetComponent(left, index);
      const ComponentType &rightValue =
          vtkm::VecTraits<T>::GetComponent(right, index);
      if (leftValue < rightValue) { return true; }
      if (rightValue < leftValue) { return false; }
    }
    // If we are here, the vectors are equal (or at least equivalent).
    return false;
  }
};

// This functor provides a partial ordering of vectors. It returns true if and
// only if all components satisfy the less operation. It is possible for
// vectors to be neither less, greater, nor equal, but the transitive closure
// is still valid.
template<typename T>
struct LessPartialOrder
{
  VTKM_EXEC_CONT
  bool operator()(const T &left, const T &right)
  {
    for (int index = 0; index < vtkm::VecTraits<T>::NUM_COMPONENTS; index++)
    {
      typedef typename vtkm::VecTraits<T>::ComponentType ComponentType;
      const ComponentType &leftValue =
          vtkm::VecTraits<T>::GetComponent(left, index);
      const ComponentType &rightValue =
          vtkm::VecTraits<T>::GetComponent(right, index);
      if (!(leftValue < rightValue)) { return false; }
    }
    // If we are here, all components satisfy less than relation.
    return true;
  }
};
////
//// END-EXAMPLE VecTraits.cxx
////

void TryLess()
{
  LessTotalOrder<vtkm::Id> totalLess1;
  VTKM_TEST_ASSERT(totalLess1(1,2), "Bad less.");
  VTKM_TEST_ASSERT(!totalLess1(2,1), "Bad less.");
  VTKM_TEST_ASSERT(!totalLess1(1,1), "Bad less.");

  LessPartialOrder<vtkm::Id> partialLess1;
  VTKM_TEST_ASSERT(partialLess1(1,2), "Bad less.");
  VTKM_TEST_ASSERT(!partialLess1(2,1), "Bad less.");
  VTKM_TEST_ASSERT(!partialLess1(1,1), "Bad less.");

  LessTotalOrder<vtkm::Id3> totalLess3;
  VTKM_TEST_ASSERT(totalLess3(vtkm::Id3(1,2,3),vtkm::Id3(3,2,1)), "Bad less.");
  VTKM_TEST_ASSERT(!totalLess3(vtkm::Id3(3,2,1),vtkm::Id3(1,2,3)), "Bad less.");
  VTKM_TEST_ASSERT(!totalLess3(vtkm::Id3(1,2,3),vtkm::Id3(1,2,3)), "Bad less.");
  VTKM_TEST_ASSERT(totalLess3(vtkm::Id3(1,2,3),vtkm::Id3(2,3,4)), "Bad less.");

  LessPartialOrder<vtkm::Id3> partialLess3;
  VTKM_TEST_ASSERT(!partialLess3(vtkm::Id3(1,2,3),vtkm::Id3(3,2,1)), "Bad less.");
  VTKM_TEST_ASSERT(!partialLess3(vtkm::Id3(3,2,1),vtkm::Id3(1,2,3)), "Bad less.");
  VTKM_TEST_ASSERT(!partialLess3(vtkm::Id3(1,2,3),vtkm::Id3(1,2,3)), "Bad less.");
  VTKM_TEST_ASSERT(partialLess3(vtkm::Id3(1,2,3),vtkm::Id3(2,3,4)), "Bad less.");
}

template<typename T>
struct VecTraits;

////
//// BEGIN-EXAMPLE VecTraitsImpl.cxx
////
//// PAUSE-EXAMPLE
#if 0
//// RESUME-EXAMPLE
namespace vtkm {
//// PAUSE-EXAMPLE
#endif
//// RESUME-EXAMPLE

template<>
struct VecTraits<vtkm::Id3>
{
  typedef vtkm::Id ComponentType;
  static const int NUM_COMPONENTS = 3;
  typedef vtkm::VecTraitsTagSizeStatic IsSizeStatic;
  typedef vtkm::VecTraitsTagMultipleComponents HasMultipleComponents;

  VTKM_EXEC_CONT
  static vtkm::IdComponent GetNumberOfComponents(const vtkm::Id3 &) {
    return NUM_COMPONENTS;
  }

  VTKM_EXEC_CONT
  static const vtkm::Id &GetComponent(const vtkm::Id3 &vector, int component) {
    return vector[component];
  }
  VTKM_EXEC_CONT
  static vtkm::Id &GetComponent(vtkm::Id3 &vector, int component) {
    return vector[component];
  }

  VTKM_EXEC_CONT
  static void SetComponent(vtkm::Id3 &vector, int component, vtkm::Id value) {
    vector[component] = value;
  }

  template<vtkm::IdComponent DestSize>
  VTKM_EXEC_CONT
  static void
  CopyInto(const vtkm::Id3 &src, vtkm::Vec<vtkm::Id,DestSize> &dest)
  {
    for (vtkm::IdComponent index = 0;
         (index < NUM_COMPONENTS) && (index < DestSize);
         index++)
    {
      dest[index] = src[index];
    }
  }
};

//// PAUSE-EXAMPLE
#if 0
//// RESUME-EXAMPLE
} // namespace vtkm
//// PAUSE-EXAMPLE
#endif
//// RESUME-EXAMPLE
////
//// END-EXAMPLE VecTraitsImpl.cxx
////

void TryCustomVecTriats()
{
  using CustomTraits = TraitsExamples::VecTraits<vtkm::Id3>;
  using OriginalTraits = vtkm::VecTraits<vtkm::Id3>;

  VTKM_STATIC_ASSERT((std::is_same<
                        CustomTraits::ComponentType,
                        OriginalTraits::ComponentType>::value));
  VTKM_STATIC_ASSERT(CustomTraits::NUM_COMPONENTS
                     == OriginalTraits::NUM_COMPONENTS);
  VTKM_STATIC_ASSERT((std::is_same<
                        CustomTraits::HasMultipleComponents,
                        OriginalTraits::HasMultipleComponents>::value));
  VTKM_STATIC_ASSERT((std::is_same<
                        CustomTraits::IsSizeStatic,
                        OriginalTraits::IsSizeStatic>::value));

  vtkm::Id3 value = TestValue(10, vtkm::Id3());
  VTKM_TEST_ASSERT(CustomTraits::GetNumberOfComponents(value)
                   == OriginalTraits::GetNumberOfComponents(value),
                   "Wrong size.");
  VTKM_TEST_ASSERT(CustomTraits::GetComponent(value, 1)
                   == OriginalTraits::GetComponent(value, 1),
                   "Wrong component.");

  CustomTraits::SetComponent(value, 2, 0);
  VTKM_TEST_ASSERT(value[2] == 0, "Did not set component.");

  vtkm::Id2 shortValue;
  CustomTraits::CopyInto(value, shortValue);
  VTKM_TEST_ASSERT(test_equal(shortValue, TestValue(10, vtkm::Id2())),
                   "Bad extract.");
}

void Test()
{
  TryRemainder();
  TryCustomTypeTraits();
  TryLess();
  TryCustomVecTriats();
}

} // namespace TraitsExamples

int Traits(int, char *[])
{
  return vtkm::testing::Testing::Run(TraitsExamples::Test);
}
