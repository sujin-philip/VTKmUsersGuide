#include <vtkm/cont/ArrayHandle.h>

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
  typedef vtkm::Scalar ValueType;

  VTKM_CONT_EXPORT
  ArrayPortalFooPressure() : Container(NULL) {  }

  VTKM_CONT_EXPORT
  ArrayPortalFooPressure(DequeType *container) : Container(container) {  }

  VTKM_CONT_EXPORT
  vtkm::Id GetNumberOfValues() const {
    return static_cast<vtkm::Id>(this->Container->size());
  }

  VTKM_CONT_EXPORT
  vtkm::Scalar Get(vtkm::Id index) const {
    VTKM_ASSERT_CONT(index >= 0);
    VTKM_ASSERT_CONT(index < this->GetNumberOfValues());
    return static_cast<vtkm::Scalar>((*this->Container)[index].Pressure);
  }

  VTKM_CONT_EXPORT
  vtkm::Scalar Set(vtkm::Id index, vtkm::Scalar value) const {
    VTKM_ASSERT_CONT(index >= 0);
    VTKM_ASSERT_CONT(index < this->GetNumberOfValues());
    (*this->Container)[index].Pressure = value;
  }

  typedef vtkm::cont::internal::IteratorFromArrayPortal<
       ArrayPortalFooPressure<DequeType> > IteratorType;

  VTKM_CONT_EXPORT
  IteratorType GetIteratorBegin() const {
    return IteratorType(*this, 0);
  }

  VTKM_CONT_EXPORT
  IteratorType GetIteratorEnd() const {
    return IteratorType(*this, this->GetNumberOfValues());
  }

private:
  DequeType *Container;
};
////
//// END-EXAMPLE ArrayPortalAdapter.cxx
////

}

////
//// BEGIN-EXAMPLE ArrayContainerControlPrototype.cxx
////
namespace vtkm {
namespace cont {
namespace internal {

template<typename T, class ArrayContainerControlTag>
class ArrayContainerControl;

}
}
} // namespace vtkm::cont::internal
////
//// END-EXAMPLE ArrayContainerControlPrototype.cxx
////

////
//// BEGIN-EXAMPLE ArrayContainerControlAdapter.cxx
////
// Includes or definition for ArrayPortalFooPressure

struct ArrayContainerControlTagFooPressure {  };

namespace vtkm {
namespace cont {
namespace internal {

template<>
class ArrayContainerControl<vtkm::Scalar, ArrayContainerControlTagFooPressure>
{
public:
  typedef vtkm::Scalar ValueType;

  typedef ArrayPortalFooPressure<FooFieldsDeque> PortalType;
  typedef ArrayPortalFooPressure<const FooFieldsDeque> PortalConstType;

  VTKM_CONT_EXPORT
  ArrayContainerControl() : Container(NULL) {  }

  VTKM_CONT_EXPORT
  ArrayContainerControl(FooFieldsDeque *container) : Container(container) {  }

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
//// END-EXAMPLE ArrayContainerControlAdapter.cxx
////

namespace {

////
//// BEGIN-EXAMPLE ArrayHandleAdapter.cxx
////
class ArrayHandleFooPressure
    : public vtkm::cont::ArrayHandle<
          vtkm::Scalar, ArrayContainerControlTagFooPressure>
{
private:
  typedef vtkm::cont::internal::ArrayContainerControl<
      vtkm::Scalar, ArrayContainerControlTagFooPressure>
      ArrayContainerControlType;

public:
  typedef vtkm::cont::ArrayHandle<
      vtkm::Scalar, ArrayContainerControlTagFooPressure> Superclass;

  VTKM_CONT_EXPORT
  ArrayHandleFooPressure(FooFieldsDeque *container)
    : Superclass(ArrayContainerControlType(container)) {  }
};
////
//// END-EXAMPLE ArrayHandleAdapter.cxx
////

void Test()
{

}

} // anonymous namespace

int ArrayHandleAdapt(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
