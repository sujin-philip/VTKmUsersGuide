#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayHandleCounting.h>
#include <vtkm/cont/DeviceAdapter.h>

#include <vtkm/cont/testing/Testing.h>

////
//// BEGIN-EXAMPLE FictitiousFieldStorage.cxx
////
#include <deque>

//// PAUSE-EXAMPLE
namespace {
//// RESUME-EXAMPLE
////
struct FooFields {
  float Pressure;
  float Temperature;
  float Velocity[3];
  // And so on...
};

typedef std::deque<FooFields> FooFieldsDeque;
////
//// END-EXAMPLE FictitiousFieldStorage.cxx
////

} // anonymous namespace

////
//// BEGIN-EXAMPLE ArrayPortalAdapter.cxx
////
#include <vtkm/cont/Assert.h>
#include <vtkm/cont/internal/IteratorFromArrayPortal.h>

//// PAUSE-EXAMPLE
namespace {
//// RESUME-EXAMPLE
////
// DequeType expected to be either FooFieldsDeque or const FooFieldsDeque
template<typename DequeType>
class ArrayPortalFooPressure
{
public:
  typedef float ValueType;

  VTKM_CONT_EXPORT
  ArrayPortalFooPressure() : Container(NULL) {  }

  VTKM_CONT_EXPORT
  ArrayPortalFooPressure(DequeType *container) : Container(container) {  }

  // Required to copy compatible types of ArrayPortalFooPressure. Really needed
  // to copy from non-const to const versions of array portals.
  template<typename OtherDequeType>
  VTKM_CONT_EXPORT
  ArrayPortalFooPressure(const ArrayPortalFooPressure<OtherDequeType> &other)
    : Container(other.GetContainer()) {  }

  VTKM_CONT_EXPORT
  vtkm::Id GetNumberOfValues() const {
    return static_cast<vtkm::Id>(this->Container->size());
  }

  VTKM_CONT_EXPORT
  ValueType Get(vtkm::Id index) const {
    VTKM_ASSERT_CONT(index >= 0);
    VTKM_ASSERT_CONT(index < this->GetNumberOfValues());
    return (*this->Container)[index].Pressure;
  }

  VTKM_CONT_EXPORT
  void Set(vtkm::Id index, ValueType value) const {
    VTKM_ASSERT_CONT(index >= 0);
    VTKM_ASSERT_CONT(index < this->GetNumberOfValues());
    (*this->Container)[index].Pressure = value;
  }

  // Here for the copy constructor.
  VTKM_CONT_EXPORT
  DequeType *GetContainer() const { return this->Container; }

private:
  DequeType *Container;
};
////
//// END-EXAMPLE ArrayPortalAdapter.cxx
////

}

////
//// BEGIN-EXAMPLE StoragePrototype.cxx
////
namespace vtkm {
namespace cont {
namespace internal {

template<typename T, class StorageTag>
class Storage;

}
}
} // namespace vtkm::cont::internal
////
//// END-EXAMPLE StoragePrototype.cxx
////

////
//// BEGIN-EXAMPLE StorageAdapter.cxx
////
// Includes or definition for ArrayPortalFooPressure

struct StorageTagFooPressure {  };

namespace vtkm {
namespace cont {
namespace internal {

template<>
class Storage<float, StorageTagFooPressure>
{
public:
  typedef float ValueType;

  typedef ArrayPortalFooPressure<FooFieldsDeque> PortalType;
  typedef ArrayPortalFooPressure<const FooFieldsDeque> PortalConstType;

  VTKM_CONT_EXPORT
  Storage() : Container(NULL) {  }

  VTKM_CONT_EXPORT
  Storage(FooFieldsDeque *container) : Container(container) {  }

  VTKM_CONT_EXPORT
  PortalType GetPortal() { return PortalType(this->Container); }

  VTKM_CONT_EXPORT
  PortalConstType GetPortalConst() const {
    return PortalConstType(this->Container);
  }

  VTKM_CONT_EXPORT
  vtkm::Id GetNumberOfValues() const {
    return static_cast<vtkm::Id>(this->Container->size());
  }

  VTKM_CONT_EXPORT
  void Allocate(vtkm::Id numberOfValues) {
    this->Container->resize(numberOfValues);
  }

  VTKM_CONT_EXPORT
  void Shrink(vtkm::Id numberOfValues) {
    this->Container->resize(numberOfValues);
  }

  VTKM_CONT_EXPORT
  void ReleaseResources() { this->Container->clear(); }

private:
  FooFieldsDeque *Container;
};

}
}
} // namespace vtkm::cont::internal
////
//// END-EXAMPLE StorageAdapter.cxx
////

namespace {

////
//// BEGIN-EXAMPLE ArrayHandleAdapter.cxx
////
class ArrayHandleFooPressure
    : public vtkm::cont::ArrayHandle<float, StorageTagFooPressure>
{
private:
  typedef vtkm::cont::internal::Storage<float, StorageTagFooPressure>
      StorageType;

public:
  typedef vtkm::cont::ArrayHandle<float, StorageTagFooPressure>
      Superclass;

  VTKM_CONT_EXPORT
  ArrayHandleFooPressure(FooFieldsDeque *container)
    : Superclass(StorageType(container)) {  }
};
////
//// END-EXAMPLE ArrayHandleAdapter.cxx
////

////
//// BEGIN-EXAMPLE UsingArrayHandleAdapter.cxx
////
template<typename GridType>
VTKM_CONT_EXPORT
void GetElevationAirPressure(const GridType &grid, FooFieldsDeque *fields)
{
  // Make an array handle that points to the pressure values in the fields.
  ArrayHandleFooPressure pressureHandle(fields);

  // This is currently commented out because worklets are not yet implemented.
//  // Run an elevation worklet.
//  vtkm::worklet::Elevation elevation(vtkm::make_Vector3(0.0, 0.0, 0.0),
//                                     vtkm::make_Vector3(0.0, 0.0, 10.0),
//                                     vtkm::make_Vector2(0.02, 0.0));
//  vtkm::cont::DispatcherMapField<vtkm::worklet::Elevation>
//      dispatcher(elevation);
//  dispatcher.Invoke(grid.GetPointCoordinates(), pressureHandle);
  //// PAUSE-EXAMPLE

  // In lieu of running something interesting, for now just copy from one array
  // to another in the execution environment.
  vtkm::cont::ArrayHandleCounting<float> countingArray(1, 50);
  vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>::Copy(
        countingArray, pressureHandle);

  //// RESUME-EXAMPLE

  // Make sure the values are flushed back to the control environment.
  pressureHandle.GetPortalConstControl();

  // Now the pressure field is in the fields container.
}
////
//// END-EXAMPLE UsingArrayHandleAdapter.cxx
////

void Test()
{
  FooFieldsDeque fields(50);
  GetElevationAirPressure(NULL, &fields);

  for (vtkm::Id index = 0; index < 50; index++)
  {
    VTKM_TEST_ASSERT(fields[index].Pressure == index+1, "Bad value.");
  }
}

} // anonymous namespace

int ArrayHandleAdapt(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
