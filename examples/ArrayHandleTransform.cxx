#include <vtkm/Types.h>

////
//// BEGIN-EXAMPLE TransformArrayFunctor.cxx
////
template<typename T>
struct ScaleBiasFunctor
{
  VTKM_EXEC_CONT
  ScaleBiasFunctor(T scale = T(1), T bias = T(0))
    : Scale(scale), Bias(bias) {  }

  VTKM_EXEC_CONT
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

template<typename ArrayHandleType>
class ArrayHandleScaleBias
    : public vtkm::cont::ArrayHandleTransform<
          typename ArrayHandleType::ValueType,
          ArrayHandleType,
          ScaleBiasFunctor<typename ArrayHandleType::ValueType> >
{
public:
  VTKM_ARRAY_HANDLE_SUBCLASS(
      ArrayHandleScaleBias,
      (ArrayHandleScaleBias<ArrayHandleType>),
      (vtkm::cont::ArrayHandleTransform<
         typename ArrayHandleType::ValueType,
         ArrayHandleType,
         ScaleBiasFunctor<typename ArrayHandleType::ValueType> >)
      );

  VTKM_CONT
  ArrayHandleScaleBias(const ArrayHandleType &array,
                       ValueType scale,
                       ValueType bias)
    : Superclass(array, ScaleBiasFunctor<ValueType>(scale, bias)) {  }
};

template<typename ArrayHandleType>
VTKM_CONT
ArrayHandleScaleBias<ArrayHandleType>
make_ArrayHandleScaleBias(const ArrayHandleType &array,
                          typename ArrayHandleType::ValueType scale,
                          typename ArrayHandleType::ValueType bias)
{
  return ArrayHandleScaleBias<ArrayHandleType>(array, scale, bias);
}
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
  //// BEGIN-EXAMPLE MakeArrayHandleTransform.cxx
  ////
  vtkm::cont::make_ArrayHandleTransform<vtkm::Float32>(
          array, ScaleBiasFunctor<vtkm::Float32>(2,3))
  ////
  //// END-EXAMPLE MakeArrayHandleTransform.cxx
  ////
  );

  ArrayHandleScaleBias<vtkm::cont::ArrayHandle<vtkm::Float32> >
      transformArray(array, 2, 3);

  CheckArray(transformArray);

  CheckArray(make_ArrayHandleScaleBias(array, 2, 3));
}

} // anonymous namespace

int ArrayHandleTransform(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
