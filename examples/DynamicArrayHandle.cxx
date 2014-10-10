#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayHandleCounting.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/DynamicArrayHandle.h>

#include <vtkm/cont/internal/StorageError.h>

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
    vtkm::cont::ArrayHandle<vtkm::Float32> concreteArray =
        vtkm::cont::make_ArrayHandle(
          reinterpret_cast<const vtkm::Float32*>(buffer), length);
    handle = concreteArray;
  } else if (type == "int") {
    vtkm::cont::ArrayHandle<vtkm::Int32> concreteArray =
        vtkm::cont::make_ArrayHandle(
          reinterpret_cast<const vtkm::Int32*>(buffer), length);
    handle = concreteArray;
  }
  return handle;
}
////
//// END-EXAMPLE CreateDynamicArrayHandle.cxx
////

void TryLoadDynamicArray()
{
  vtkm::Float32 scalarBuffer[10];
  vtkm::cont::DynamicArrayHandle handle =
      LoadDynamicArray(scalarBuffer, 10, "float");
  VTKM_TEST_ASSERT(
        handle.IsTypeAndStorage(vtkm::Float32(),
                                VTKM_DEFAULT_STORAGE_TAG()),
        "Type not right.");
  VTKM_TEST_ASSERT(
        !handle.IsTypeAndStorage(vtkm::Int32(),
                                 VTKM_DEFAULT_STORAGE_TAG()),
        "Type not right.");

  vtkm::Int32 idBuffer[10];
  handle = LoadDynamicArray(idBuffer, 10, "int");
  VTKM_TEST_ASSERT(
        handle.IsTypeAndStorage(vtkm::Int32(),
                                VTKM_DEFAULT_STORAGE_TAG()),
        "Type not right.");
  VTKM_TEST_ASSERT(
        !handle.IsTypeAndStorage(vtkm::Float32(),
                                 VTKM_DEFAULT_STORAGE_TAG()),
        "Type not right.");
}

void QueryCastDynamicArrayHandle()
{
  ////
  //// BEGIN-EXAMPLE QueryDynamicArrayHandle.cxx
  ////
  std::vector<vtkm::Float32> scalarBuffer(10);
  vtkm::cont::DynamicArrayHandle dynamicHandle(
        vtkm::cont::make_ArrayHandle(scalarBuffer));

  // This returns true
  bool isFloat32 = dynamicHandle.IsTypeAndStorage(vtkm::Float32(),
                                                  VTKM_DEFAULT_STORAGE_TAG());

  // This returns false
  bool isId = dynamicHandle.IsTypeAndStorage(vtkm::Id(),
                                             VTKM_DEFAULT_STORAGE_TAG());

  // This returns false
  bool isErrorStorage = dynamicHandle.IsTypeAndStorage(
                            vtkm::Float32(),
                            vtkm::cont::internal::StorageTagError());
  ////
  //// END-EXAMPLE QueryDynamicArrayHandle.cxx
  ////

  VTKM_TEST_ASSERT(isFloat32, "Didn't query right.");
  VTKM_TEST_ASSERT(!isId, "Didn't query right.");
  VTKM_TEST_ASSERT(!isErrorStorage, "Didn't query right.");

  ////
  //// BEGIN-EXAMPLE CastDynamicArrayHandle.cxx
  ////
  vtkm::cont::ArrayHandle<vtkm::Float32> concreteHandle =
      dynamicHandle.CastToArrayHandle(vtkm::Float32(),
                                      VTKM_DEFAULT_STORAGE_TAG());
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
  template<typename T, typename Storage>
  VTKM_CONT_EXPORT
  void operator()(const vtkm::cont::ArrayHandle<T,Storage> &array) const
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
//// BEGIN-EXAMPLE CastAndCallStorage.cxx
////
struct MyIdStorageList :
    vtkm::ListTagBase<
        vtkm::cont::StorageTagBasic,
        vtkm::cont::ArrayHandleCounting<vtkm::Id>::StorageTag>
{  };

void PrintIds(vtkm::cont::DynamicArrayHandle array)
{
  PrintArrayContents(array.ResetStorageList(MyIdStorageList()));
}
////
//// END-EXAMPLE CastAndCallStorage.cxx
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
  //// BEGIN-EXAMPLE CastAndCallTypeAndStorage.cxx
  ////
  PrintArrayContents(dynamicArray.
                     ResetTypeList(vtkm::TypeListTagId()).
                     ResetStorageList(MyIdStorageList()));
  ////
  //// END-EXAMPLE CastAndCallTypeAndStorage.cxx
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
