#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/Timer.h>

#include <vtkm/filter/PointElevation.h>

#include <vtkm/cont/testing/MakeTestDataSet.h>
#include <vtkm/cont/testing/Testing.h>

namespace {

void DoTiming()
{
  vtkm::cont::DataSet dataSet =
      vtkm::cont::testing::MakeTestDataSet().Make2DUniformDataSet0();
  ////
  //// BEGIN-EXAMPLE Timer.cxx
  ////
  vtkm::filter::PointElevation elevationFilter;

  vtkm::cont::Timer<> timer;

  vtkm::filter::FieldResult fieldResult =
      elevationFilter.Execute(dataSet, dataSet.GetCoordinateSystem());

  // This code makes sure data is pulled back to the host in a host/device
  // architecture.
  vtkm::cont::ArrayHandle<vtkm::Float64> outArray;
  fieldResult.FieldAs(outArray);
  outArray.GetPortalConstControl();

  vtkm::Float64 elapsedTime = timer.GetElapsedTime();

  std::cout << "Time to run: " << elapsedTime << std::endl;
  ////
  //// END-EXAMPLE Timer.cxx
  ////
}

} // anonymous namespace

int Timer(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(DoTiming);
}
