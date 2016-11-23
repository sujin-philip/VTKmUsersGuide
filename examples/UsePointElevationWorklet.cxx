#include <vtkm/cont/ArrayHandle.h>

#include <vtkm/worklet/DispatcherMapField.h>
#include <vtkm/worklet/PointElevation.h>

#include <vtkm/cont/testing/Testing.h>

namespace {

////
//// BEGIN-EXAMPLE UsePointElevationWorklet.cxx
////
VTKM_CONT
vtkm::cont::ArrayHandle<vtkm::FloatDefault>
ComputeAirPressure(
    vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault,3> > pointCoordinates)
{
  vtkm::worklet::PointElevation elevationWorklet;

  // Use the elevation worklet to estimate atmospheric pressure based on the
  // height of the point coordinates. Atmospheric pressure is 101325 Pa at
  // sea level and drops about 12 Pa per meter.
  elevationWorklet.SetLowPoint(vtkm::Vec<vtkm::Float64,3>(0.0, 0.0, 0.0));
  elevationWorklet.SetHighPoint(vtkm::Vec<vtkm::Float64,3>(0.0, 0.0, 2000.0));
  elevationWorklet.SetRange(101325.0, 77325.0);

  vtkm::worklet::DispatcherMapField<vtkm::worklet::PointElevation>
      elevationDispatcher(elevationWorklet);

  vtkm::cont::ArrayHandle<vtkm::FloatDefault> pressure;

  elevationDispatcher.Invoke(pointCoordinates, pressure);

  return pressure;
}
////
//// END-EXAMPLE UsePointElevationWorklet.cxx
////

void DoPointElevation()
{
  using InputArrayType =
      vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault,3> >;
  InputArrayType pointCoordinates;

  const vtkm::Id ARRAY_SIZE = 10;
  pointCoordinates.Allocate(ARRAY_SIZE);
  InputArrayType::PortalControl inputPortal =
      pointCoordinates.GetPortalControl();
  for (vtkm::Id index = 0; index < ARRAY_SIZE; index++)
  {
    inputPortal.Set(index, vtkm::Vec<vtkm::FloatDefault,3>(
                      static_cast<vtkm::FloatDefault>(index)));
  }

  using OutputArrayType = vtkm::cont::ArrayHandle<vtkm::FloatDefault>;
  OutputArrayType pressure = ComputeAirPressure(pointCoordinates);

  vtkm::cont::printSummary_ArrayHandle(pressure, std::cout);
  std::cout << std::endl;

  OutputArrayType::PortalConstControl outputPortal =
      pressure.GetPortalConstControl();
  for (vtkm::Id index = 0; index < ARRAY_SIZE; index++)
  {
    vtkm::FloatDefault value = outputPortal.Get(index);
    VTKM_TEST_ASSERT(test_equal(value, 101325.0 - 12*index),
                     "Bad value from worklet result.");
  }
}

void Test()
{
  DoPointElevation();
}

} // anonymous namespace

int UsePointElevationWorklet(int, char*[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
