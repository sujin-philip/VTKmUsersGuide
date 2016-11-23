#include <vtkm/cont/ArrayHandleCompositeVector.h>
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

  // [(0,3,2,0), (1,1,7,0), (2,4,1,0), (3,1,8,0), (4,5,2,0)].
  VTKM_TEST_ASSERT(test_equal(portal.Get(0), vtkm::make_Vec(0,3,2,0)),
                   "Bad value in array.");
  VTKM_TEST_ASSERT(test_equal(portal.Get(1), vtkm::make_Vec(1,1,7,0)),
                   "Bad value in array.");
  VTKM_TEST_ASSERT(test_equal(portal.Get(2), vtkm::make_Vec(2,4,1,0)),
                   "Bad value in array.");
  VTKM_TEST_ASSERT(test_equal(portal.Get(3), vtkm::make_Vec(3,1,8,0)),
                   "Bad value in array.");
}

void ArrayHandleCompositeVectorBasic()
{
  ////
  //// BEGIN-EXAMPLE ArrayHandleCompositeVectorBasic.cxx
  ////
  // Create an array with [0, 1, 2, 3, 4]
  typedef vtkm::cont::ArrayHandleIndex ArrayType1;
  ArrayType1 array1(5);

  // Create an array with [3, 1, 4, 1, 5]
  typedef vtkm::cont::ArrayHandle<vtkm::Id> ArrayType2;
  ArrayType2 array2;
  array2.Allocate(5);
  ArrayType2::PortalControl arrayPortal2 = array2.GetPortalControl();
  arrayPortal2.Set(0, 3);
  arrayPortal2.Set(1, 1);
  arrayPortal2.Set(2, 4);
  arrayPortal2.Set(3, 1);
  arrayPortal2.Set(4, 5);

  // Create an array with [2, 7, 1, 8, 2]
  typedef vtkm::cont::ArrayHandle<vtkm::Id> ArrayType3;
  ArrayType3 array3;
  array3.Allocate(5);
  ArrayType2::PortalControl arrayPortal3 = array3.GetPortalControl();
  arrayPortal3.Set(0, 2);
  arrayPortal3.Set(1, 7);
  arrayPortal3.Set(2, 1);
  arrayPortal3.Set(3, 8);
  arrayPortal3.Set(4, 2);

  // Create an array with [0, 0, 0, 0]
  typedef vtkm::cont::ArrayHandleConstant<vtkm::Id> ArrayType4;
  ArrayType4 array4(0, 5);

  // Use ArrayhandleCompositeVector to create the array
  // [(0,3,2,0), (1,1,7,0), (2,4,1,0), (3,1,8,0), (4,5,2,0)].
  typedef vtkm::cont::ArrayHandleCompositeVectorType<
      ArrayType1, ArrayType2, ArrayType3, ArrayType4>::type CompositeArrayType;
  CompositeArrayType compositeArray(array1, 0,
                                    array2, 0,
                                    array3, 0,
                                    array4, 0);
  ////
  //// END-EXAMPLE ArrayHandleCompositeVectorBasic.cxx
  ////
  CheckArray(compositeArray);

  CheckArray(
  ////
  //// BEGIN-EXAMPLE MakeArrayHandleCompositeVector.cxx
  ////
  vtkm::cont::make_ArrayHandleCompositeVector(array1, 0,
                                              array2, 0,
                                              array3, 0,
                                              array4, 0)
  ////
  //// END-EXAMPLE MakeArrayHandleCompositeVector.cxx
  ////
        );
}

////
//// BEGIN-EXAMPLE ArrayHandleCompositeVectorComponents.cxx
////
template<typename CoordinateArrayType, typename ElevationArrayType>
VTKM_CONT
typename vtkm::cont::ArrayHandleCompositeVectorType<
    CoordinateArrayType, CoordinateArrayType, ElevationArrayType>::type
ElevateCoordianteArray(const CoordinateArrayType &coordinateArray,
                       const ElevationArrayType &elevationArray)
{
  VTKM_IS_ARRAY_HANDLE(CoordinateArrayType);
  VTKM_IS_ARRAY_HANDLE(ElevationArrayType);

  return vtkm::cont::make_ArrayHandleCompositeVector(coordinateArray, 0,
                                                     coordinateArray, 1,
                                                     elevationArray, 0);
}
////
//// END-EXAMPLE ArrayHandleCompositeVectorComponents.cxx
////

void TryElevateCoordinateArray()
{
  typedef vtkm::cont::ArrayHandleUniformPointCoordinates CoordinateArrayType;
  CoordinateArrayType coordinateArray(vtkm::Id3(5, 5, 3));

  typedef vtkm::cont::ArrayHandleCounting<vtkm::FloatDefault>
      ElevationArrayType;
  ElevationArrayType elevationArray(0.0f, 1.0f, 5*5*3);

  typedef vtkm::cont::ArrayHandleCompositeVectorType<
      CoordinateArrayType, CoordinateArrayType, ElevationArrayType>::type
      ElevatedCoordsType;
  ElevatedCoordsType elevatedCoords =
      ElevateCoordianteArray(coordinateArray, elevationArray);

  vtkm::cont::printSummary_ArrayHandle(elevatedCoords, std::cout);
  std::cout << std::endl;
  ElevatedCoordsType::PortalConstControl elevatedPortal =
      elevatedCoords.GetPortalConstControl();
  VTKM_TEST_ASSERT(elevatedPortal.GetNumberOfValues() == 5*5*3, "Wrong size.");
  vtkm::Id flatIndex = 0;
  for (vtkm::Id kIndex = 0; kIndex < 3; kIndex++)
  {
    for (vtkm::Id jIndex = 0; jIndex < 5; jIndex++)
    {
      for (vtkm::Id iIndex = 0; iIndex < 5; iIndex++)
      {
        VTKM_TEST_ASSERT(test_equal(elevatedPortal.Get(flatIndex),
                                    vtkm::make_Vec(iIndex, jIndex, flatIndex)),
                         "Bad value.");
        flatIndex++;
      }
    }
  }
}

void Test()
{
  ArrayHandleCompositeVectorBasic();
  TryElevateCoordinateArray();
}

} // anonymous namespace

int ArrayHandleCompositeVector(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
