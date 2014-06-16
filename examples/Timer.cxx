#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/Timer.h>

#include <vtkm/cont/testing/Testing.h>

namespace {

void DoTiming()
{
  ////
  //// BEGIN-EXAMPLE Timer.cxx
  ////
  vtkm::cont::ArrayHandle<vtkm::Scalar> results;
//  vtkm::cont::DispatchMapField<vtkm::worklet::Elevation> dispatcher;

  vtkm::cont::Timer<> timer;

//  dispatcher.Invoke(grid.GetPointCoordinates(), results);

  // This call makes sure data is pulled back to the host in a host/device
  // architecture.
  results.GetPortalConstControl();

  vtkm::Scalar elapsedTime = timer.GetElapsedTime();

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
