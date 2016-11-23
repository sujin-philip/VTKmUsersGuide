#include <vtkm/Types.h>

#include <vtkm/testing/Testing.h>

namespace {

////
//// BEGIN-EXAMPLE EnvironmentModifierMacro.cxx
////
template<typename ValueType>
VTKM_EXEC_CONT
ValueType Square(const ValueType &inValue)
{
  return inValue * inValue;
}
////
//// END-EXAMPLE EnvironmentModifierMacro.cxx
////

////
//// BEGIN-EXAMPLE SuppressExecWarnings.cxx
////
VTKM_SUPPRESS_EXEC_WARNINGS
template<typename Functor>
VTKM_EXEC_CONT
void OverlyComplicatedForLoop(Functor &functor, vtkm::Id numInterations)
{
  for (vtkm::Id index = 0; index < numInterations; index++)
  {
    functor();
  }
}
////
//// END-EXAMPLE SuppressExecWarnings.cxx
////

struct TestFunctor
{
  vtkm::Id Count;

  VTKM_CONT
  TestFunctor() : Count(0) {  }

  VTKM_CONT
  void operator()() { this->Count++; }
};

void Test()
{
  VTKM_TEST_ASSERT(Square(2) == 4, "Square function doesn't square.");

  TestFunctor functor;
  OverlyComplicatedForLoop(functor, 10);
  VTKM_TEST_ASSERT(functor.Count == 10, "Bad iterations.");
}

} // anonymous namespace

int EnvironmentModifierMacros(int, char *[])
{
  return vtkm::testing::Testing::Run(Test);
}
