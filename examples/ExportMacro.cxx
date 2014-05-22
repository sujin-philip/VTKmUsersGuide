#include <vtkm/Types.h>

namespace {

template<typename ValueType>
VTKM_EXEC_CONT_EXPORT
ValueType Square(const ValueType &inValue)
{
  return inValue * inValue;
}

} // anonymous namespace

int ExportMacro(int, char *[])
{
  return !(Square(2) == 4);
}
