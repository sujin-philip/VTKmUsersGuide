#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayHandleCounting.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/DynamicArrayHandle.h>

#include <vtkm/cont/internal/ArrayContainerControlError.h>

#include <vtkm/cont/testing/Testing.h>

namespace {

////
//// BEGIN-EXAMPLE CreateDynamicArrayHandle.cxx
////
VTKM_CONT_EXPORT
vtkm::cont::DynamicArrayHandle
LoadDynamicArray(const void *buffer, vtkm::Id length, std::string type)
{
  vtkm::cont::DynamicArrayHandle handle;
  if (type == "float")
  {
    vtkm::cont::ArrayHandle<vtkm::Scalar> concreteArray =
        vtkm::cont::make_ArrayHandle(
          reinterpret_cast<const vtkm::Scalar*>(buffer), length);
    handle = concreteArray;
  } else if (type == "int") {
    vtkm::cont::ArrayHandle<vtkm::Id> concreteArray =
        vtkm::cont::make_ArrayHandle(
          reinterpret_cast<const vtkm::Id*>(buffer), length);
    handle = concreteArray;
  }
  return handle;
}
////
//// END-EXAMPLE CreateDynamicArrayHandle.cxx
////

void TryLoadDynamicArray()
{
  vtkm::Scalar scalarBuffer[10];
  vtkm::cont::DynamicArrayHandle handle =
      LoadDynamicArray(scalarBuffer, 10, "float");
  VTKM_TEST_ASSERT(
        handle.IsTypeAndContainer(vtkm::Scalar(),
                                  VTKM_DEFAULT_ARRAY_CONTAINER_CONTROL_TAG()),
        "Type not right.");
  VTKM_TEST_ASSERT(
        !handle.IsTypeAndContainer(vtkm::Id(),
                                   VTKM_DEFAULT_ARRAY_CONTAINER_CONTROL_TAG()),
        "Type not right.");

  vtkm::Id idBuffer[10];
  handle = LoadDynamicArray(idBuffer, 10, "int");
  VTKM_TEST_ASSERT(
        handle.IsTypeAndContainer(vtkm::Id(),
                                   VTKM_DEFAULT_ARRAY_CONTAINER_CONTROL_TAG()),
        "Type not right.");
  VTKM_TEST_ASSERT(
        !handle.IsTypeAndContainer(vtkm::Scalar(),
                                  VTKM_DEFAULT_ARRAY_CONTAINER_CONTROL_TAG()),
        "Type not right.");
}

void QueryCastDynamicArrayHandle()
{
  ////
  //// BEGIN-EXAMPLE QueryDynamicArrayHandle.cxx
  ////
  std::vector<vtkm::Scalar> scalarBuffer(10);
  vtkm::cont::DynamicArrayHandle dynamicHandle(
        vtkm::cont::make_ArrayHandle(scalarBuffer));

  // This returns true
  bool isScalar = dynamicHandle.IsTypeAndContainer(
                    vtkm::Scalar(), VTKM_DEFAULT_ARRAY_CONTAINER_CONTROL_TAG());

  // This returns false
  bool isId = dynamicHandle.IsTypeAndContainer(
                vtkm::Id(), VTKM_DEFAULT_ARRAY_CONTAINER_CONTROL_TAG());

  // This returns false
  bool isErrorContainer = dynamicHandle.IsTypeAndContainer(
                            vtkm::Scalar(),
                            vtkm::cont::internal::ArrayContainerControlTagError());
  ////
  //// END-EXAMPLE QueryDynamicArrayHandle.cxx
  ////

  VTKM_TEST_ASSERT(isScalar, "Didn't query right.");
  VTKM_TEST_ASSERT(!isId, "Didn't query right.");
  VTKM_TEST_ASSERT(!isErrorContainer, "Didn't query right.");

  ////
  //// BEGIN-EXAMPLE CastDynamicArrayHandle.cxx
  ////
  vtkm::cont::ArrayHandle<vtkm::Scalar> concreteHandle =
      dynamicHandle.CastToArrayHandle(vtkm::Scalar(),
                                      VTKM_DEFAULT_ARRAY_CONTAINER_CONTROL_TAG());
  ////
  //// END-EXAMPLE CastDynamicArrayHandle.cxx
  ////

  VTKM_TEST_ASSERT(concreteHandle.GetNumberOfValues() == 10,
                   "Unexpected length");
}

////
//// BEGIN-EXAMPLE UsingCastAndCall.cxx
////
struct PrintArrayContentsFunctor
{
  template<typename T, typename Container>
  VTKM_CONT_EXPORT
  void operator()(const vtkm::cont::ArrayHandle<T,Container> &array) const
  {
    this->PrintArrayPortal(array.GetPortalConstControl());
  }

private:
  template<typename PortalType>
  VTKM_CONT_EXPORT
  void PrintArrayPortal(const PortalType &portal) const
  {
    for (vtkm::Id index = 0; index < portal.GetNumberOfValues(); index++)
    {
      std::cout << " " << portal.Get(index);
    }
    std::cout << std::endl;
  }
};

template<typename DynamicArrayType>
void PrintArrayContents(const DynamicArrayType &array)
{
  array.CastAndCall(PrintArrayContentsFunctor());
}
////
//// END-EXAMPLE UsingCastAndCall.cxx
////

////
//// BEGIN-EXAMPLE CastAndCallContainer.cxx
////
struct MyIdContainers :
    vtkm::ListTagBase2<
        vtkm::cont::ArrayContainerControlTagBasic,
        vtkm::cont::ArrayHandleCounting<vtkm::Id>::ArrayContainerControlTag>
{  };

void PrintIds(vtkm::cont::DynamicArrayHandle array)
{
  PrintArrayContents(array.ResetContainerList(MyIdContainers()));
}
////
//// END-EXAMPLE CastAndCallContainer.cxx
////

void TryPrintArrayContents()
{
  vtkm::cont::ArrayHandleCounting<vtkm::Id> implicitArray(0, 10);

  vtkm::cont::ArrayHandle<vtkm::Id> concreteArray;
  vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>::Copy(
        implicitArray, concreteArray);

  vtkm::cont::DynamicArrayHandle dynamicArray = concreteArray;

  PrintArrayContents(dynamicArray);

  ////
  //// BEGIN-EXAMPLE CastAndCallAllTypes.cxx
  ////
  PrintArrayContents(dynamicArray.ResetTypeList(vtkm::TypeListTagAll()));
  ////
  //// END-EXAMPLE CastAndCallAllTypes.cxx
  ////

  ////
  //// BEGIN-EXAMPLE CastAndCallSingleType.cxx
  ////
  PrintArrayContents(dynamicArray.ResetTypeList(vtkm::TypeListTagId()));
  ////
  //// END-EXAMPLE CastAndCallSingleType.cxx
  ////

  dynamicArray = implicitArray;

  PrintIds(dynamicArray);

  ////
  //// BEGIN-EXAMPLE CastAndCallTypeAndContainer.cxx
  ////
  PrintArrayContents(dynamicArray.
                       ResetTypeList(vtkm::TypeListTagId()).
                       ResetContainerList(MyIdContainers()));
  ////
  //// END-EXAMPLE CastAndCallTypeAndContainer.cxx
  ////
}

void Test()
{
  TryLoadDynamicArray();
  QueryCastDynamicArrayHandle();
  TryPrintArrayContents();
}

} // anonymous namespace

int DynamicArrayHandle(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
