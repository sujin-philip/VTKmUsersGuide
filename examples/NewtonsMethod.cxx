#include <vtkm/Matrix.h>
#include <vtkm/NewtonsMethod.h>

#include <vtkm/testing/Testing.h>

namespace {

////
//// BEGIN-EXAMPLE NewtonsMethod.cxx
////
// A functor for the mathematical function f(x) = [dot(x,x),x[0]*x[1]]
struct FunctionFunctor
{
  template<typename T>
  VTKM_EXEC_CONT
  vtkm::Vec<T,2> operator()(const vtkm::Vec<T,2> &x) const
  {
    return vtkm::make_Vec(vtkm::dot(x,x), x[0]*x[1]);
  }
};

// A functor for the Jacobian of the mathematical function
// f(x) = [dot(x,x),x[0]*x[1]], which is
//   | 2*x[0] 2*x[1] |
//   |   x[1]   x[0] |
struct JacobianFunctor
{
  template<typename T>
  VTKM_EXEC_CONT
  vtkm::Matrix<T,2,2> operator()(const vtkm::Vec<T,2> &x) const
  {
    vtkm::Matrix<T,2,2> jacobian;
    jacobian(0,0) = 2*x[0];
    jacobian(0,1) = 2*x[1];
    jacobian(1,0) = x[1];
    jacobian(1,1) = x[0];

    return jacobian;
  }
};

VTKM_EXEC
void SolveNonlinear()
{
  // Use Newton's method to solve the nonlinear system of equations:
  //
  //    x^2 + y^2 = 2
  //    x*y = 1
  //
  // There are two possible solutions, which are (x=1,y=1) and (x=-1,y=-1).
  // The one found depends on the starting value.
  vtkm::Vec<vtkm::Float32,2> answer1 =
      vtkm::NewtonsMethod(JacobianFunctor(),
                                FunctionFunctor(),
                                vtkm::make_Vec(2.0f, 1.0f),
                                vtkm::make_Vec(1.0f, 0.0f));
  // answer1 is [1,1]

  vtkm::Vec<vtkm::Float32,2> answer2 =
      vtkm::NewtonsMethod(JacobianFunctor(),
                                FunctionFunctor(),
                                vtkm::make_Vec(2.0f, 1.0f),
                                vtkm::make_Vec(0.0f, -2.0f));
  // answer2 is [-1,-1]
  //// PAUSE-EXAMPLE
  std::cout << answer1 << " " << answer2 << std::endl;

  VTKM_TEST_ASSERT(test_equal(answer1, vtkm::make_Vec(1,1), 0.01),
                   "Bad answer 1.");
  VTKM_TEST_ASSERT(test_equal(answer2, vtkm::make_Vec(-1,-1), 0.01),
                   "Bad answer 2.");
  //// RESUME-EXAMPLE
}
////
//// END-EXAMPLE NewtonsMethod.cxx
////

void Run()
{
  SolveNonlinear();
}

} // anonymous namespace

int NewtonsMethod(int, char*[])
{
  return vtkm::testing::Testing::Run(Run);
}
