#include <vtkm/TypeTraits.h>
#include <vtkm/VectorTraits.h>

#include <vtkm/testing/Testing.h>

// This should be replaced with VTK-m math when available!!!
#include <math.h>

////
//// BEGIN-EXAMPLE TypeTraits.cxx
////
#include <vtkm/TypeTraits.h>
//// PAUSE-EXAMPLE
namespace {
//// RESUME-EXAMPLE

template<typename T>
T Remainder(const T &numerator, const T &denominator);

namespace detail {

template<typename T>
T RemainderImpl(const T &numerator,
              const T &denominator,
              vtkm::TypeTraitsIntegerTag,
              vtkm::TypeTraitsScalarTag)
{
  return numerator % denominator;
}

template<typename T>
T RemainderImpl(const T &numerator,
              const T &denominator,
              vtkm::TypeTraitsRealTag,
              vtkm::TypeTraitsScalarTag)
{
  T quotient = numerator / denominator;
  return (quotient - floor(quotient))*denominator;
}

template<typename T, typename NumericTag>
T RemainderImpl(const T &numerator,
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
        Remainder(numerator[componentIndex], denominator[componentIndex]);
  }
  return result;
}

} // namespace detail

template<typename T>
T Remainder(const T &numerator, const T &denominator)
{
  return detail::RemainderImpl(numerator,
                             denominator,
                             typename vtkm::TypeTraits<T>::NumericTag(),
                             typename vtkm::TypeTraits<T>::DimensionalityTag());
}
////
//// END-EXAMPLE TypeTraits.cxx
////

void TryRemainder()
{
  vtkm::Id m1 = Remainder(7, 3);
  VTKM_TEST_ASSERT(m1 == 1, "Got bad remainder");

  vtkm::Scalar m2 = Remainder(7.0, 3.0);
  VTKM_TEST_ASSERT(test_equal(m2, vtkm::Scalar(1)), "Got bad remainder");

  vtkm::Id3 m3 = Remainder(vtkm::Id3(10, 9, 8), vtkm::Id3(7, 6, 5));
  VTKM_TEST_ASSERT(test_equal(m3, vtkm::Id3(3, 3, 3)), "Got bad remainder");

  vtkm::Vector3 m4 = Remainder(vtkm::Vector3(10, 9, 8), vtkm::Vector3(7, 6, 5));
  VTKM_TEST_ASSERT(test_equal(m4, vtkm::Vector3(3, 3, 3)), "Got bad remainder");
}

} // anonymous namespace

////
//// BEGIN-EXAMPLE VectorTraits.cxx
////
#include <vtkm/VectorTraits.h>
//// PAUSE-EXAMPLE
namespace {
//// RESUME-EXAMPLE

// This functor provides a total ordering of vectors. Every compared vector
// will be either less, greater, or equal (assuming all the vector components
// also have a total ordering).
template<typename T>
struct LessTotalOrder
{
  VTKM_EXEC_CONT_EXPORT
  bool operator()(const T &left, const T &right)
  {
    for (int index = 0; index < vtkm::VectorTraits<T>::NUM_COMPONENTS; index++)
    {
      typedef typename vtkm::VectorTraits<T>::ComponentType ComponentType;
      const ComponentType &leftValue =
          vtkm::VectorTraits<T>::GetComponent(left, index);
      const ComponentType &rightValue =
          vtkm::VectorTraits<T>::GetComponent(right, index);
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
  VTKM_EXEC_CONT_EXPORT
  bool operator()(const T &left, const T &right)
  {
    for (int index = 0; index < vtkm::VectorTraits<T>::NUM_COMPONENTS; index++)
    {
      typedef typename vtkm::VectorTraits<T>::ComponentType ComponentType;
      const ComponentType &leftValue =
          vtkm::VectorTraits<T>::GetComponent(left, index);
      const ComponentType &rightValue =
          vtkm::VectorTraits<T>::GetComponent(right, index);
      if (!(leftValue < rightValue)) { return false; }
    }
    // If we are here, all components satisfy less than relation.
    return true;
  }
};
////
//// END-EXAMPLE VectorTraits.cxx
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

void Test()
{
  TryRemainder();
  TryLess();
}

} // anonymous namespace

int Traits(int, char *[])
{
  return vtkm::testing::Testing::Run(Test);
}
