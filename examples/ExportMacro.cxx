#include <vtkm/Types.h>

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

} // anonymous namespace

int ExportMacro(int, char *[])
{
  return !(Square(2) == 4);
}
