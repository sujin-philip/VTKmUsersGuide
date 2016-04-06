#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/DataSetBuilderUniform.h>
#include <vtkm/cont/DeviceAdapter.h>

#include <vtkm/worklet/PointElevation.h>

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
  VTKM_ARRAY_HANDLE_SUBCLASS_NT(
      ArrayHandleFooPressure,
      (vtkm::cont::ArrayHandle<float, StorageTagFooPressure>));

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
VTKM_CONT_EXPORT
void GetElevationAirPressure(vtkm::cont::DataSet grid, FooFieldsDeque *fields)
{
  // Make an array handle that points to the pressure values in the fields.
  ArrayHandleFooPressure pressureHandle(fields);

  // Use the elevation worklet to estimate atmospheric pressure based on the
  // height of the point coordinates. Atmospheric pressure is 101325 Pa at
  // sea level and drops about 12 Pa per meter.
  vtkm::worklet::PointElevation elevation;
  elevation.SetLowPoint(vtkm::make_Vec(0.0, 0.0, 0.0));
  elevation.SetHighPoint(vtkm::make_Vec(0.0, 0.0, 2000.0));
  elevation.SetRange(101325.0, 77325.0);

  vtkm::worklet::DispatcherMapField<vtkm::worklet::PointElevation>
      dispatcher(elevation);
  dispatcher.Invoke(grid.GetCoordinateSystem().GetData(), pressureHandle);

  // Make sure the values are flushed back to the control environment.
  pressureHandle.GetPortalConstControl();

  // Now the pressure field is in the fields container.
}
////
//// END-EXAMPLE UsingArrayHandleAdapter.cxx
////

void Test()
{
  vtkm::cont::DataSet grid =
      vtkm::cont::DataSetBuilderUniform::Create(vtkm::Id3(2, 2, 50));

  FooFieldsDeque fields(4*50);
  GetElevationAirPressure(grid, &fields);

  vtkm::Float32 value = 101325.0f;
  for (vtkm::Id heightIndex = 0; heightIndex < 50; heightIndex++)
  {
    for (vtkm::Id slabIndex = 0; slabIndex < 4; slabIndex++)
    {
      VTKM_TEST_ASSERT(test_equal(fields[4*heightIndex+slabIndex].Pressure,
                                  value),
                       "Bad value.");
    }
    value -= 12.0f;
  }
}

} // anonymous namespace

int ArrayHandleAdapt(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
