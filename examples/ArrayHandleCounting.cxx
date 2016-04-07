#include <vtkm/cont/ArrayHandleCounting.h>
#include <vtkm/cont/ArrayHandleIndex.h>

#include <vtkm/cont/testing/Testing.h>

namespace {

template<typename ArrayHandleType>
void CheckArray(const ArrayHandleType array,
                typename ArrayHandleType::ValueType startValue,
                typename ArrayHandleType::ValueType stepValue,
                vtkm::Id expectedLength)
{
  VTKM_TEST_ASSERT(array.GetNumberOfValues() == expectedLength,
                   "Array has wrong size.");

  typename ArrayHandleType::PortalConstControl portal =
      array.GetPortalConstControl();
  VTKM_TEST_ASSERT(portal.GetNumberOfValues() == expectedLength,
                   "Portal has wrong size.");

  typename ArrayHandleType::ValueType expectedValue = startValue;
  for (vtkm::Id index = 0; index < expectedLength; index++)
  {
    VTKM_TEST_ASSERT(test_equal(portal.Get(index), expectedValue),
                     "Array has wrong value.");
    expectedValue = expectedValue + stepValue;
  }
}

void Test()
{
  ////
  //// BEGIN-EXAMPLE ArrayHandleIndex.cxx
  ////
  // Create an array containing [0, 1, 2, 3, ..., 49].
  vtkm::cont::ArrayHandleIndex indexArray(50);
  ////
  //// END-EXAMPLE ArrayHandleIndex.cxx
  ////
  CheckArray(indexArray, 0, 1, 50);

  ////
  //// BEGIN-EXAMPLE ArrayHandleCountingBasic.cxx
  ////
  // Create an array containing [-1.0, -0.9, -0.8, ..., 0.9, 1.0]
  vtkm::cont::ArrayHandleCounting<vtkm::Float32> sampleArray(-1.0f, 0.1f, 21);
  ////
  //// END-EXAMPLE ArrayHandleCountingBasic.cxx
  ////
  CheckArray(sampleArray, -1.0f, 0.1f, 21);

  CheckArray(
  ////
  //// BEGIN-EXAMPLE MakeArrayHandleCountingBasic.cxx
  ////
  // Create an array of 50 entries, all containing the number 3.
  vtkm::cont::make_ArrayHandleCounting(-1.0f, 0.1f, 21)
  ////
  //// END-EXAMPLE MakeArrayHandleCountingBasic.cxx
  ////
        , -1.0f, 0.1f, 21);
  ////
  //// BEGIN-EXAMPLE ArrayHandleCountingBackward.cxx
  ////
  // Create an array containing [49, 48, 47, 46, ..., 0].
  vtkm::cont::ArrayHandleCounting<vtkm::Id> backwardIndexArray(49, -1, 50);
  ////
  //// END-EXAMPLE ArrayHandleCountingBackward.cxx
  ////
  CheckArray(backwardIndexArray, 49, -1, 50);

  CheckArray(
  ////
  //// BEGIN-EXAMPLE ArrayHandleCountingVec.cxx
  ////
  // Create an array containg [(0,-3,75), (1,2,25), (3,7,-25)]
  vtkm::cont::make_ArrayHandleCounting(vtkm::make_Vec(0, -3, 75),
                                       vtkm::make_Vec(1, 5, -50),
                                       3)
  ////
  //// END-EXAMPLE ArrayHandleCountingVec.cxx
  ////
        , vtkm::make_Vec(0, -3, 75), vtkm::make_Vec(1, 5, -50), 3);
}

} // anonymous namespace

int ArrayHandleCounting(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
