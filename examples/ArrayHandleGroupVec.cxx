#include <vtkm/cont/ArrayHandleGroupVec.h>
#include <vtkm/cont/ArrayHandleConstant.h>
#include <vtkm/cont/ArrayHandleCounting.h>
#include <vtkm/cont/ArrayHandleIndex.h>
#include <vtkm/cont/ArrayHandleUniformPointCoordinates.h>

#include <vtkm/cont/testing/Testing.h>

namespace {

template<typename ArrayHandleType>
void CheckArray(ArrayHandleType array)
{
  vtkm::cont::printSummary_ArrayHandle(array, std::cout);
  std::cout << std::endl;
  typename ArrayHandleType::PortalConstControl portal =
      array.GetPortalConstControl();

  vtkm::Id expectedValue = 0;
  for (vtkm::Id vecIndex = 0; vecIndex < portal.GetNumberOfValues(); vecIndex++)
  {
    typename ArrayHandleType::ValueType vecValue = portal.Get(vecIndex);
    for (vtkm::IdComponent componentIndex = 0;
         componentIndex < vecValue.GetNumberOfComponents();
         componentIndex++)
    {
      VTKM_TEST_ASSERT(vecValue[componentIndex] == expectedValue,
                       "Got bad value.");
      expectedValue++;
    }
  }
}

void ArrayHandleGroupVecBasic()
{
  ////
  //// BEGIN-EXAMPLE ArrayHandleGroupVecBasic.cxx
  ////
  // Create an array containing [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
  typedef vtkm::cont::ArrayHandleIndex ArrayType;
  ArrayType sourceArray(12);

  // Create an array containing [(0,1), (2,3), (4,5), (6,7), (8,9), (10,11)]
  vtkm::cont::ArrayHandleGroupVec<ArrayType,2> vec2Array(sourceArray);

  // Create an array containing [(0,1,2), (3,4,5), (6,7,8), (9,10,11)]
  vtkm::cont::ArrayHandleGroupVec<ArrayType,3> vec3Array(sourceArray);
  ////
  //// END-EXAMPLE ArrayHandleGroupVecBasic.cxx
  ////
  CheckArray(vec2Array);
  CheckArray(vec3Array);

  CheckArray(
  ////
  //// BEGIN-EXAMPLE MakeArrayHandleGroupVec.cxx
  ////
  // Create an array containing [(0,1,2,3), (4,5,6,7), (8,9,10,11)]
  vtkm::cont::make_ArrayHandleGroupVec<4>(sourceArray)
  ////
  //// END-EXAMPLE MakeArrayHandleGroupVec.cxx
  ////
        );
}

void Test()
{
  ArrayHandleGroupVecBasic();
}

} // anonymous namespace

int ArrayHandleGroupVec(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
