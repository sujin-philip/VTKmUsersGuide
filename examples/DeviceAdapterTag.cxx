////
//// BEGIN-EXAMPLE DefaultDeviceAdapter.cxx
////
// Uncomment one (and only one) of the following to reconfigure the Dax
// code to use a particular device. Comment them all to automatically pick a
// device.
#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_SERIAL
//#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_CUDA
//#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_OPENMP
//#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_TBB

#include <vtkm/cont/DeviceAdapter.h>
////
//// END-EXAMPLE DefaultDeviceAdapter.cxx
////

#include <vtkm/cont/testing/Testing.h>

namespace {

void Test()
{

}

} // anonymous namespace

int DeviceAdapterTag(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
