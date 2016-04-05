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

#include <vtkm/cont/tbb/DeviceAdapterTBB.h>

#include <vtkm/exec/Assert.h>
#include <vtkm/exec/FunctorBase.h>

#include <vtkm/cont/testing/Testing.h>

namespace {

////
//// BEGIN-EXAMPLE DefaultDeviceTemplateArg.cxx
////
template<typename Device = VTKM_DEFAULT_DEVICE_ADAPTER_TAG>
struct SetPortalFunctor : vtkm::exec::FunctorBase
{
  VTKM_IS_DEVICE_ADAPTER_TAG(Device);

  typedef typename vtkm::cont::ArrayHandle<vtkm::Id>::
       ExecutionTypes<Device>::Portal ExecPortalType;
  ExecPortalType Portal;

  VTKM_CONT_EXPORT
  SetPortalFunctor(vtkm::cont::ArrayHandle<vtkm::Id> array, vtkm::Id size)
    : Portal(array.PrepareForOutput(size, Device()))
  {  }

  //// PAUSE-EXAMPLE
  VTKM_CONT_EXPORT
  SetPortalFunctor(const ExecPortalType &portal) : Portal(portal)
  {  }
  //// RESUME-EXAMPLE
  VTKM_EXEC_EXPORT
  void operator()(vtkm::Id index) const
  {
    //// PAUSE-EXAMPLE
    VTKM_ASSERT_EXEC(index >= 0, *this);
    VTKM_ASSERT_EXEC(index < this->Portal.GetNumberOfValues(), *this);
    //// RESUME-EXAMPLE
    typedef typename ExecPortalType::ValueType ValueType;
    this->Portal.Set(index, TestValue(index, ValueType()));
  }
};
////
//// END-EXAMPLE DefaultDeviceTemplateArg.cxx
////

template<typename ExecPortalType,
         typename ArrayHandleType,
         typename Device>
VTKM_CONT_EXPORT
void TryUsingExecPortal(const ExecPortalType &execPortal,
                        const ArrayHandleType &arrayHandle,
                        Device)
{
  typedef typename ArrayHandleType::ValueType ValueType;

  SetPortalFunctor<Device> functor(execPortal);

  vtkm::cont::DeviceAdapterAlgorithm<Device>::Schedule(
        functor, arrayHandle.GetNumberOfValues());

  typename ArrayHandleType::PortalConstControl contPortal =
      arrayHandle.GetPortalConstControl();
  for (vtkm::Id index = 0; index < arrayHandle.GetNumberOfValues(); index++)
  {
    VTKM_TEST_ASSERT(contPortal.Get(index) == TestValue(index, ValueType()),
                     "Bad value set.");
  }
}

void UseTBBDeviceAdapter()
{
  vtkm::cont::ArrayHandle<vtkm::Id> arrayHandle;

  vtkm::cont::ArrayHandle<vtkm::Id>::
      ExecutionTypes<vtkm::cont::DeviceAdapterTagTBB>::Portal portal =
  ////
  //// BEGIN-EXAMPLE SpecifyDeviceAdapter.cxx
  ////
  arrayHandle.PrepareForOutput(50, vtkm::cont::DeviceAdapterTagTBB());
  ////
  //// END-EXAMPLE SpecifyDeviceAdapter.cxx
  ////

  TryUsingExecPortal(portal, arrayHandle, vtkm::cont::DeviceAdapterTagTBB());
}

void UseDefaultDeviceAdapter()
{
  vtkm::cont::ArrayHandle<vtkm::Id> arrayHandle;

  SetPortalFunctor<> functor(arrayHandle, 50);

  TryUsingExecPortal(functor.Portal,
                     arrayHandle,
                     VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
}

void Test()
{
  UseTBBDeviceAdapter();
  UseDefaultDeviceAdapter();
}

} // anonymous namespace

int DeviceAdapterTag(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
