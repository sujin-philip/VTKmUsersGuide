#include <vtkm/cont/ArrayHandleUniformPointCoordinates.h>

#include <vtkm/worklet/DispatcherMapField.h>
#include <vtkm/worklet/ScatterCounting.h>
#include <vtkm/worklet/WorkletMapField.h>

#include <vtkm/cont/testing/Testing.h>

namespace {

////
//// BEGIN-EXAMPLE ScatterCounting.cxx
////
struct ClipPoints
{
  class Count : public vtkm::worklet::WorkletMapField
  {
  public:
    typedef void ControlSignature(FieldIn<Vec3> points,
                                  FieldOut<IdComponentType> count);
    typedef _2 ExecutionSignature(_1);
    using InputDomain = _1;

    template<typename T>
    VTKM_CONT
    Count(const vtkm::Vec<T,3> &boundsMin, const vtkm::Vec<T,3> &boundsMax)
      : BoundsMin(boundsMin[0], boundsMin[1], boundsMin[2]),
        BoundsMax(boundsMax[0], boundsMax[1], boundsMax[2])
    {  }

    template<typename T>
    VTKM_EXEC
    vtkm::IdComponent operator()(const vtkm::Vec<T,3> &point) const
    {
      return static_cast<vtkm::IdComponent>((this->BoundsMin[0] < point[0]) &&
                                            (this->BoundsMin[1] < point[1]) &&
                                            (this->BoundsMin[2] < point[2]) &&
                                            (this->BoundsMax[0] > point[0]) &&
                                            (this->BoundsMax[1] > point[1]) &&
                                            (this->BoundsMax[2] > point[2]));
    }

  private:
    vtkm::Vec<vtkm::FloatDefault,3> BoundsMin;
    vtkm::Vec<vtkm::FloatDefault,3> BoundsMax;
  };

  class Generate : public vtkm::worklet::WorkletMapField
  {
  public:
    typedef void ControlSignature(FieldIn<Vec3> inPoints,
                                  FieldOut<Vec3> outPoints);
    typedef void ExecutionSignature(_1, _2);
    using InputDomain = _1;

    ////
    //// BEGIN-EXAMPLE DeclareScatter.cxx
    ////
    using ScatterType = vtkm::worklet::ScatterCounting;

    VTKM_CONT
    ScatterType GetScatter() const { return this->Scatter; }
    ////
    //// END-EXAMPLE DeclareScatter.cxx
    ////

    template<typename CountArrayType, typename DeviceAdapterTag>
    VTKM_CONT
    Generate(const CountArrayType &countArray, DeviceAdapterTag)
      : Scatter(countArray, DeviceAdapterTag())
    {
      VTKM_IS_ARRAY_HANDLE(CountArrayType);
    }

    template<typename InType, typename OutType>
    VTKM_EXEC
    void operator()(const vtkm::Vec<InType,3> &inPoint,
                    vtkm::Vec<OutType,3> &outPoint) const
    {
      // The scatter ensures that this method is only called for input points
      // that are passed to the output (where the count was 1). Thus, in this
      // case we know that we just need to copy the input to the output.
      outPoint = vtkm::Vec<OutType,3>(inPoint[0], inPoint[1], inPoint[2]);
    }

  private:
    ScatterType Scatter;
  };

  template<typename T, typename Storage, typename DeviceAdapterTag>
  VTKM_CONT
  static vtkm::cont::ArrayHandle<vtkm::Vec<T,3> >
  Run(const vtkm::cont::ArrayHandle<vtkm::Vec<T,3>, Storage> &pointArray,
      vtkm::Vec<T,3> boundsMin,
      vtkm::Vec<T,3> boundsMax,
      DeviceAdapterTag)
  {
    vtkm::cont::ArrayHandle<vtkm::IdComponent> countArray;

    ClipPoints::Count workletCount(boundsMin, boundsMax);
    vtkm::worklet::DispatcherMapField<ClipPoints::Count, DeviceAdapterTag>
        dispatcherCount(workletCount);
    dispatcherCount.Invoke(pointArray, countArray);

    vtkm::cont::ArrayHandle<vtkm::Vec<T,3> > clippedPointsArray;

    ClipPoints::Generate workletGenerate(countArray, DeviceAdapterTag());
    vtkm::worklet::DispatcherMapField<ClipPoints::Generate, DeviceAdapterTag>
        dispatcherGenerate(workletGenerate);
    dispatcherGenerate.Invoke(pointArray, clippedPointsArray);

    return clippedPointsArray;
  }
};
////
//// END-EXAMPLE ScatterCounting.cxx
////

void Run()
{
  std::cout << "Trying clip points." << std::endl;
  vtkm::cont::ArrayHandleUniformPointCoordinates points(vtkm::Id3(10, 10, 10));
  vtkm::Vec<vtkm::FloatDefault,3> boundsMin(0.5f, 0.5f, 0.5f);
  vtkm::Vec<vtkm::FloatDefault,3> boundsMax(8.5f, 8.5f, 8.5f);

  VTKM_TEST_ASSERT(points.GetNumberOfValues() == 1000,
                   "Unexpected number of input points.");

  vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault,3> > clippedPoints =
      ClipPoints::Run(points,
                      boundsMin,
                      boundsMax,
                      VTKM_DEFAULT_DEVICE_ADAPTER_TAG());

  vtkm::cont::printSummary_ArrayHandle(clippedPoints, std::cout);
  std::cout << std::endl;
  VTKM_TEST_ASSERT(clippedPoints.GetNumberOfValues() == 512,
                   "Unexpected number of output points.");
}

} // anonymous namespace

int ScatterCounting(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Run);
}
