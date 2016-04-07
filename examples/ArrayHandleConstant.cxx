#include <vtkm/cont/ArrayHandleConstant.h>

#include <vtkm/cont/testing/Testing.h>

namespace {

template<typename ArrayHandleType>
void CheckArray(const ArrayHandleType array,
                vtkm::Id expectedLength,
                typename ArrayHandleType::ValueType expectedValue)
{
  VTKM_TEST_ASSERT(array.GetNumberOfValues() == expectedLength,
                   "Array has wrong size.");

  typename ArrayHandleType::PortalConstControl portal =
      array.GetPortalConstControl();
  VTKM_TEST_ASSERT(portal.GetNumberOfValues() == expectedLength,
                   "Portal has wrong size.");

  for (vtkm::Id index = 0; index < expectedLength; index++)
  {
    VTKM_TEST_ASSERT(test_equal(portal.Get(index), expectedValue),
                     "Array has wrong value.");
  }
}

void Test()
{
  ////
  //// BEGIN-EXAMPLE ArrayHandleConstant.cxx
  ////
  // Create an array of 50 entries, all containing the number 3. This could be
  // used, for example, to represent the sizes of all the polygons in a set
  // where we know all the polygons are triangles.
  vtkm::cont::ArrayHandleConstant<vtkm::Id> constantArray(3, 50);
  ////
  //// END-EXAMPLE ArrayHandleConstant.cxx
  ////

  CheckArray(constantArray, 50, 3);

  CheckArray(
  ////
  //// BEGIN-EXAMPLE MakeArrayHandleConstant.cxx
  ////
  // Create an array of 50 entries, all containing the number 3.
  vtkm::cont::make_ArrayHandleConstant(3, 50)
  ////
  //// END-EXAMPLE MakeArrayHandleConstant.cxx
  ////
        , 50, 3);
}

} // anonymous namespace

int ArrayHandleConstant(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
