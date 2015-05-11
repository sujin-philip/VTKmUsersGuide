#include <vtkm/Types.h>

////
//// BEGIN-EXAMPLE TransformArrayFunctor.cxx
////
template<typename T>
struct ScaleBiasFunctor
{
  VTKM_EXEC_CONT_EXPORT
  ScaleBiasFunctor(T scale = T(1), T bias = T(0))
    : Scale(scale), Bias(bias) {  }

  VTKM_EXEC_CONT_EXPORT
  T operator()(T x) const
  {
    return this->Scale*x + this->Bias;
  }

  T Scale;
  T Bias;
};
////
//// END-EXAMPLE TransformArrayFunctor.cxx
////

////
//// BEGIN-EXAMPLE TransformArrayHandle.cxx
////
#include <vtkm/cont/ArrayHandleTransform.h>

template<typename ValueType, typename ArrayHandleType>
class ArrayHandleScaleBias
    : public vtkm::cont::ArrayHandleTransform<
          ValueType, ArrayHandleType, ScaleBiasFunctor<ValueType> >
{
  typedef vtkm::cont::ArrayHandleTransform<
      ValueType, ArrayHandleType, ScaleBiasFunctor<ValueType> > Superclass;
public:
  VTKM_CONT_EXPORT
  ArrayHandleScaleBias(const ArrayHandleType &array,
                       ValueType scale,
                       ValueType bias)
    : Superclass(array, ScaleBiasFunctor<ValueType>(scale, bias)) {  }
};
////
//// END-EXAMPLE TransformArrayHandle.cxx
////

#include <vtkm/cont/DeviceAdapter.h>

#include <vtkm/cont/testing/Testing.h>

#include <vector>

namespace {

template<typename ArrayHandleType>
void CheckArray(const ArrayHandleType &array)
{
  for(vtkm::Id index = 0; index < array.GetNumberOfValues(); index++)
  {
    VTKM_TEST_ASSERT(test_equal(array.GetPortalConstControl().Get(index),
                                2*TestValue(index,vtkm::Float32()) + 3),
                     "Bad transformed value.");
  }
}

void Test()
{
  std::vector<vtkm::Float32> buffer(10);
  for (size_t index = 0; index < buffer.size(); index++)
  {
    buffer[index] = TestValue(index, vtkm::Float32());
  }

  vtkm::cont::ArrayHandle<vtkm::Float32> array =
      vtkm::cont::make_ArrayHandle(buffer);

  CheckArray(
  ////
  //// BEGIN-EXAMPLE MakeArrayHandleImplicit.cxx
  ////
  vtkm::cont::make_ArrayHandleTransform<vtkm::Float32>(
          array, ScaleBiasFunctor<vtkm::Float32>(2,3))
  ////
  //// END-EXAMPLE MakeArrayHandleImplicit.cxx
  ////
  );

  ArrayHandleScaleBias<vtkm::Float32, vtkm::cont::ArrayHandle<vtkm::Float32> >
      transformArray(array, 2, 3);

  CheckArray(transformArray);
}

} // anonymous namespace

int ArrayHandleTransform(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
