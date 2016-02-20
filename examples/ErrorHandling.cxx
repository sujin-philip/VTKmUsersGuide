#include <vtkm/cont/ArrayHandleCounting.h>
#include <vtkm/cont/Error.h>
#include <vtkm/cont/ErrorControlBadValue.h>

#include <vtkm/worklet/DispatcherMapField.h>
#include <vtkm/worklet/WorkletMapField.h>

#include <vtkm/cont/testing/Testing.h>

namespace ErrorHandlingNamespace {

////
//// BEGIN-EXAMPLE CatchingErrors.cxx
////
int main(int argc, char **argv)
{
  try
  {
    // Do something cool with VTK-m
    // ...
    //// PAUSE-EXAMPLE
    throw vtkm::cont::ErrorControlBadValue("Oh, no!");
    //// RESUME-EXAMPLE
  }
  catch (vtkm::cont::Error error)
  {
    std::cout << error.GetMessage() << std::endl;
    return 1;
  }
  return 0;
}
////
//// END-EXAMPLE CatchingErrors.cxx
////

////
//// BEGIN-EXAMPLE ExecutionErrors.cxx
////
struct SquareRoot : vtkm::worklet::WorkletMapField
{
public:
  typedef void ControlSignature(FieldIn<Scalar>, FieldOut<Scalar>);
  typedef _2 ExecutionSignature(_1);

  template<typename T>
  VTKM_EXEC_EXPORT
  T operator()(T x) const
  {
    if (x < 0)
      {
      this->RaiseError("Cannot take the square root of a negative number.");
      }
    return vtkm::Sqrt(x);
  }
};
////
//// END-EXAMPLE ExecutionErrors.cxx
////

VTKM_CONT_EXPORT
void TrySquareRoot()
{
  vtkm::cont::ArrayHandle<vtkm::Float32> output;

  vtkm::worklet::DispatcherMapField<SquareRoot> dispatcher;

  std::cout << "Trying valid input." << std::endl;
  vtkm::cont::ArrayHandleCounting<vtkm::Float32> validInput(0.0f, 1.0f, 10);
  dispatcher.Invoke(validInput, output);

  std::cout << "Trying invalid input." << std::endl;
  vtkm::cont::ArrayHandleCounting<vtkm::Float32> invalidInput(-2.0, 1.0f, 10);
  bool errorCaught = false;
  try
  {
    dispatcher.Invoke(invalidInput, output);
  }
  catch (vtkm::cont::ErrorExecution error)
  {
    std::cout << "Caught this error:" << std::endl;
    std::cout << error.GetMessage() << std::endl;
    errorCaught = true;
  }
  VTKM_TEST_ASSERT(errorCaught, "Did not get expected error.");
}

void Test()
{
  VTKM_TEST_ASSERT(ErrorHandlingNamespace::main(0, NULL) != 0, "No error?");
  TrySquareRoot();
}

} // namespace ErrorHandlingNamespace

int ErrorHandling(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(ErrorHandlingNamespace::Test);
}
