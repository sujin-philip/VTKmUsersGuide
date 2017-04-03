#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_SERIAL

#include <vtkm/cont/arg/TypeCheckTagArray.h>
#include <vtkm/cont/arg/TypeCheckTagExecObject.h>
#include <vtkm/cont/arg/TransportTagArrayIn.h>
#include <vtkm/cont/arg/TransportTagArrayOut.h>
#include <vtkm/cont/arg/TransportTagExecObject.h>

#include <vtkm/cont/DeviceAdapter.h>

#include <vtkm/cont/testing/Testing.h>

namespace {

namespace TypeCheckNamespace {

////
//// BEGIN-EXAMPLE TypeCheck.cxx
////
struct MyExecObject : vtkm::exec::ExecutionObjectBase { vtkm::Id Value; };

void DoTypeChecks()
{
  ////
  //// PAUSE-EXAMPLE
  ////
  std::cout << "Testing type checks" << std::endl;
  ////
  //// RESUME-EXAMPLE
  ////
  using vtkm::cont::arg::TypeCheck;
  using vtkm::cont::arg::TypeCheckTagArray;
  using vtkm::cont::arg::TypeCheckTagExecObject;

  bool check1 = TypeCheck<TypeCheckTagExecObject, MyExecObject>::value; // true
  bool check2 = TypeCheck<TypeCheckTagExecObject, vtkm::Id>::value;     // false

  typedef vtkm::cont::ArrayHandle<vtkm::Float32> ArrayType;

  bool check3 =                                                         // true
      TypeCheck<TypeCheckTagArray<vtkm::TypeListTagField>, ArrayType>::value;
  bool check4 =                                                         // false
      TypeCheck<TypeCheckTagArray<vtkm::TypeListTagIndex>, ArrayType>::value;
  bool check5 = TypeCheck<TypeCheckTagExecObject, ArrayType>::value;    // false
  ////
  //// PAUSE-EXAMPLE
  ////
  VTKM_TEST_ASSERT(check1 == true, "Type check failed.");
  VTKM_TEST_ASSERT(check2 == false, "Type check failed.");
  VTKM_TEST_ASSERT(check3 == true, "Type check failed.");
  VTKM_TEST_ASSERT(check4 == false, "Type check failed.");
  VTKM_TEST_ASSERT(check5 == false, "Type check failed.");
  ////
  //// RESUME-EXAMPLE
  ////
}
////
//// END-EXAMPLE TypeCheck.cxx
////

} // namespace TypeCheckNamespace

using namespace TypeCheckNamespace;

namespace TransportNamespace {

////
//// BEGIN-EXAMPLE Transport.cxx
////
struct MyExecObject : vtkm::exec::ExecutionObjectBase { vtkm::Id Value; };

typedef vtkm::cont::ArrayHandle<vtkm::Id> ArrayType;

void DoTransport(const MyExecObject &inExecObject,
                 const ArrayType &inArray,
                 const ArrayType &outArray)
{
  ////
  //// PAUSE-EXAMPLE
  ////
  std::cout << "Testing transports." << std::endl;
  ////
  //// RESUME-EXAMPLE
  ////
  typedef VTKM_DEFAULT_DEVICE_ADAPTER_TAG Device;

  using vtkm::cont::arg::Transport;
  using vtkm::cont::arg::TransportTagArrayIn;
  using vtkm::cont::arg::TransportTagArrayOut;
  using vtkm::cont::arg::TransportTagExecObject;

  // The executive object transport just passes the object through.
  typedef Transport<TransportTagExecObject,MyExecObject,Device>
      ExecObjectTransport;
  MyExecObject passedExecObject =
      ExecObjectTransport()(inExecObject, inArray, 10, 10);

  // The array in transport returns a read-only array portal.
  typedef Transport<TransportTagArrayIn,ArrayType,Device> ArrayInTransport;
  ArrayInTransport::ExecObjectType inPortal =
      ArrayInTransport()(inArray, inArray, 10, 10);

  // The array out transport returns an allocated array portal.
  typedef Transport<TransportTagArrayOut,ArrayType,Device> ArrayOutTransport;
  ArrayOutTransport::ExecObjectType outPortal =
      ArrayOutTransport()(outArray, inArray, 10, 10);
  ////
  //// PAUSE-EXAMPLE
  ////
  VTKM_TEST_ASSERT(passedExecObject.Value == 5, "Bad exec object.");
  CheckPortal(inPortal);
  VTKM_TEST_ASSERT(outPortal.GetNumberOfValues() == 10, "Bad array out.");
  ////
  //// RESUME-EXAMPLE
  ////
}
////
//// END-EXAMPLE Transport.cxx
////

void DoTransport()
{
  MyExecObject execObject;
  execObject.Value = 5;

  vtkm::Id buffer[10];
  for (vtkm::Id index = 0; index < 10; index++)
  {
    buffer[index] = TestValue(index, vtkm::Id());
  }

  DoTransport(execObject, vtkm::cont::make_ArrayHandle(buffer,10), ArrayType());
}

} // namespace TransportNamespace

using namespace TransportNamespace;

void Test()
{
  DoTypeChecks();
  DoTransport();
}

} // anonymous namespace

int TransferringArguments(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
