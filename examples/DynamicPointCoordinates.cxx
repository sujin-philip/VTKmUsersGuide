#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayHandleCounting.h>
#include <vtkm/cont/DynamicPointCoordinates.h>
#include <vtkm/cont/PointCoordinatesArray.h>
#include <vtkm/cont/PointCoordinatesListTag.h>
#include <vtkm/cont/PointCoordinatesUniform.h>

#include <vtkm/cont/testing/Testing.h>

namespace {

void CreateDynamicPointCoordinates()
{
  ////
  //// BEGIN-EXAMPLE CreateDynamicPointCoordinates.cxx
  ////
  std::vector<vtkm::Vec<vtkm::Float32,3> > coordinatesBuffer;
  // Fill coordinatesBuffer...
  vtkm::cont::PointCoordinatesArray arrayPointCoords(
        vtkm::cont::make_ArrayHandle(coordinatesBuffer));
  vtkm::cont::DynamicPointCoordinates pointCoords1(arrayPointCoords);

  vtkm::cont::PointCoordinatesUniform uniformPointCoords(
        vtkm::Extent3(vtkm::Id3(0,0,0),vtkm::Id3(10,10,10)),
        vtkm::make_Vec(0, 0, 0),
        vtkm::make_Vec(1, 1, 1));
  vtkm::cont::DynamicPointCoordinates pointCoords2(uniformPointCoords);
  ////
  //// END-EXAMPLE CreateDynamicPointCoordinates.cxx
  ////

  VTKM_TEST_ASSERT(
        pointCoords1.IsPointCoordinateType(vtkm::cont::PointCoordinatesArray()),
        "Bad type.");
  VTKM_TEST_ASSERT(
        !pointCoords1.IsPointCoordinateType(vtkm::cont::PointCoordinatesUniform()),
        "Bad type.");

  VTKM_TEST_ASSERT(
        !pointCoords2.IsPointCoordinateType(vtkm::cont::PointCoordinatesArray()),
        "Bad type.");
  VTKM_TEST_ASSERT(
        pointCoords2.IsPointCoordinateType(vtkm::cont::PointCoordinatesUniform()),
        "Bad type.");
}

void QueryAndCast()
{
  ////
  //// BEGIN-EXAMPLE QueryDynamicPointCoordinates.cxx
  ////
  vtkm::cont::PointCoordinatesUniform uniformPointCoords(
        vtkm::Extent3(vtkm::Id3(0,0,0),vtkm::Id3(10,10,10)),
        vtkm::make_Vec(0, 0, 0),
        vtkm::make_Vec(1, 1, 1));
  vtkm::cont::DynamicPointCoordinates dynamicPointCoords(uniformPointCoords);

  // This returns true
  bool isUniform = dynamicPointCoords.IsPointCoordinateType(
                     vtkm::cont::PointCoordinatesUniform());

  // This returns false
  bool isArray = dynamicPointCoords.IsPointCoordinateType(
                   vtkm::cont::PointCoordinatesArray());
  ////
  //// END-EXAMPLE QueryDynamicPointCoordinates.cxx
  ////

  VTKM_TEST_ASSERT(isUniform, "Wrong type.");
  VTKM_TEST_ASSERT(!isArray, "Wrong type.");

  ////
  //// BEGIN-EXAMPLE CastDynamicPointCoordinates.cxx
  ////
  vtkm::cont::PointCoordinatesUniform concretePointCoords =
      dynamicPointCoords.CastToPointCoordinates(
        vtkm::cont::PointCoordinatesUniform());
  ////
  //// END-EXAMPLE CastDynamicPointCoordinates.cxx
  ////
}

////
//// BEGIN-EXAMPLE UsingPCCastAndCall.cxx
////
struct PrintPointCoordinatesFunctor
{
  template<typename ArrayHandleType>
  VTKM_CONT_EXPORT
  void operator()(const ArrayHandleType &array) const
  {
    typename ArrayHandleType::PortalConstControl portal =
          array.GetPortalConstControl();
    for (vtkm::Id index = 0; index < portal.GetNumberOfValues(); index++)
    {
      std::cout << " " << portal.Get(index);
    }
    std::cout << std::endl;
  }
};

template<typename DynamicPointCoordinatesType>
void PrintPointCoordinates(const DynamicPointCoordinatesType &pointCoords)
{
  pointCoords.CastAndCall(PrintPointCoordinatesFunctor());
}
////
//// END-EXAMPLE UsingPCCastAndCall.cxx
////

void TryPrintPointCoordinates()
{
  vtkm::cont::PointCoordinatesUniform uniformPointCoords(
        vtkm::Extent3(vtkm::Id3(0,0,0),vtkm::Id3(2,2,2)),
        vtkm::make_Vec(0, 0, 0),
        vtkm::make_Vec(1, 1, 1));
  vtkm::cont::DynamicPointCoordinates dynamicPointCoords(uniformPointCoords);

  PrintPointCoordinates(dynamicPointCoords);
}

////
//// BEGIN-EXAMPLE CustomPointCoordinates.cxx
////
class PointCoordinatesCounting :
    public vtkm::cont::internal::PointCoordinatesBase
{
public:
  VTKM_CONT_EXPORT
  PointCoordinatesCounting() : NumberOfPoints(0) {  }

  VTKM_CONT_EXPORT
  PointCoordinatesCounting(vtkm::Id numPoints) : NumberOfPoints(numPoints) {  }

  template<typename Functor, typename TypeList, typename StorageList>
  VTKM_CONT_EXPORT
  void CastAndCall(const Functor &f, TypeList, StorageList) const
  {
    typedef vtkm::Vec<vtkm::FloatDefault,3> PointCoordinatesType;

    vtkm::cont::ArrayHandleCounting<PointCoordinatesType>
        countingArray(PointCoordinatesType(0.0), this->NumberOfPoints);
    f(countingArray);
  }
private:
  vtkm::Id NumberOfPoints;
};
////
//// END-EXAMPLE CustomPointCoordinates.cxx
////

////
//// BEGIN-EXAMPLE UsingCustomPointCoordinates.cxx
////
struct PointCoordinatesListTagCounting :
    public vtkm::ListTagBase<PointCoordinatesCounting>
{  };

struct MyPointCoordinatesListTag :
    public vtkm::ListTagJoin<PointCoordinatesListTagCounting,
                             vtkm::cont::PointCoordinatesListTagCommon>
{  };

void Foo()
{
  PointCoordinatesCounting countingPoints(10);

  vtkm::cont::DynamicPointCoordinates dynamicPoints(countingPoints);

  PrintPointCoordinates(
        dynamicPoints.ResetPointCoordinatesList(MyPointCoordinatesListTag()));
}
////
//// END-EXAMPLE UsingCustomPointCoordinates.cxx
////

void Test()
{
  CreateDynamicPointCoordinates();
  QueryAndCast();
  TryPrintPointCoordinates();
  Foo();
}

} // anonymous namespace

int DynamicPointCoordinates(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
