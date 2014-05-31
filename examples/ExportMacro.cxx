#include <vtkm/Types.h>

#include <vtkm/testing/Testing.h>

namespace {

////
//// BEGIN-EXAMPLE ExportMacro.cxx
////
template<typename ValueType>
VTKM_EXEC_CONT_EXPORT
ValueType Square(const ValueType &inValue)
{
  return inValue * inValue;
}
////
//// END-EXAMPLE ExportMacro.cxx
////

void Test()
{
  VTKM_TEST_ASSERT(Square(2) == 4, "Square function doesn't square.");
}

} // anonymous namespace

int ExportMacro(int, char *[])
{
  return vtkm::testing::Testing::Run(Test);
}
