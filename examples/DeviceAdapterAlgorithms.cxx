#include <vtkm/cont/DeviceAdapter.h>

#include <vtkm/cont/testing/Testing.h>

////
//// BEGIN-EXAMPLE DeviceAdapterAlgorithmPrototype.cxx
////
namespace vtkm {
namespace cont {

template<typename DeviceAdapterTag>
struct DeviceAdapterAlgorithm;

}
} // namespace vtkm::cont
////
//// END-EXAMPLE DeviceAdapterAlgorithmPrototype.cxx
////

namespace {

void Test()
{

}

} // anonymous namespace

int DeviceAdapterAlgorithms(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
