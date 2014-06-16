#include <vtkm/cont/Error.h>
#include <vtkm/cont/ErrorControlBadValue.h>

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

void Test()
{
  VTKM_TEST_ASSERT(ErrorHandlingNamespace::main(0, NULL) != 0, "No error?");
}

} // namespace ErrorHandlingNamespace

int ErrorHandling(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(ErrorHandlingNamespace::Test);
}
