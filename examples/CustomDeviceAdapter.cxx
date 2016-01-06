////
//// BEGIN-EXAMPLE DeviceAdapterTagCxx11Thread.h
////
#include <vtkm/cont/internal/DeviceAdapterTag.h>

// If this device adapter were to be contributed to VTK-m, then this macro
// declaration should be moved to DeviceAdapterTag.h and given a unique
// number.
#define VTKM_DEVICE_ADAPTER_CXX11_THREAD 101

VTKM_VALID_DEVICE_ADAPTER(Cxx11Thread, VTKM_DEVICE_ADAPTER_CXX11_THREAD);
////
//// END-EXAMPLE DeviceAdapterTagCxx11Thread.h
////

#include <vtkm/cont/internal/ArrayManagerExecution.h>

////
//// BEGIN-EXAMPLE ArrayManagerExecutionPrototype.cxx
////
namespace vtkm {
namespace cont {
namespace internal {

template<typename T, typename StorageTag, typename DeviceAdapterTag>
class ArrayManagerExecution;

}
}
} // namespace vtkm::cont::internal
////
//// END-EXAMPLE ArrayManagerExecutionPrototype.cxx
////

////
//// BEGIN-EXAMPLE ArrayManagerExecutionCxx11Thread.h
////
//// PAUSE-EXAMPLE
// We did not really put the device adapter components in separate header
// files, but for the purposes of an example we are pretending we are.
#if 0
//// RESUME-EXAMPLE
#include <vtkm/cont/cxx11/internal/DeviceAdapterTagCxx11Thread.h>
//// PAUSE-EXAMPLE
#endif
//// RESUME-EXAMPLE

#include <vtkm/cont/internal/ArrayManagerExecution.h>
#include <vtkm/cont/internal/ArrayManagerExecutionShareWithControl.h>

namespace vtkm {
namespace cont {
namespace internal {

template<typename T, typename StorageTag>
class ArrayManagerExecution<
      T, StorageTag, vtkm::cont::DeviceAdapterTagCxx11Thread>
    : public vtkm::cont::internal::ArrayManagerExecutionShareWithControl<
        T, StorageTag>
{
  typedef vtkm::cont::internal::ArrayManagerExecutionShareWithControl
      <T, StorageTag> Superclass;

public:
  VTKM_CONT_EXPORT
  ArrayManagerExecution(typename Superclass::StorageType *storage)
    : Superclass(storage) {  }
};

}
}
} // namespace vtkm::cont::internal
////
//// END-EXAMPLE ArrayManagerExecutionCxx11Thread.h
////

////
//// BEGIN-EXAMPLE DeviceAdapterAlgorithmCxx11Thread.h
////
//// PAUSE-EXAMPLE
// We did not really put the device adapter components in separate header
// files, but for the purposes of an example we are pretending we are.
#if 0
//// RESUME-EXAMPLE
#include <vtkm/cont/cxx11/internal/DeviceAdapterTagCxx11Thread.h>
//// PAUSE-EXAMPLE
#endif
//// RESUME-EXAMPLE

#include <vtkm/cont/DeviceAdapterAlgorithm.h>
#include <vtkm/cont/internal/DeviceAdapterAlgorithmGeneral.h>

#include <thread>

namespace vtkm {
namespace cont {

template<>
struct DeviceAdapterAlgorithm<vtkm::cont::DeviceAdapterTagCxx11Thread>
    : vtkm::cont::internal::DeviceAdapterAlgorithmGeneral<
          DeviceAdapterAlgorithm<vtkm::cont::DeviceAdapterTagCxx11Thread>,
          vtkm::cont::DeviceAdapterTagCxx11Thread>
{
private:
  template<typename FunctorType>
  struct ScheduleKernel1D
  {
    VTKM_CONT_EXPORT
    ScheduleKernel1D(const FunctorType &functor)
      : Functor(functor)
    {  }

    VTKM_EXEC_EXPORT
    void operator()() const
    {
      for (vtkm::Id threadId = this->BeginId;
           threadId < this->EndId;
           threadId++)
      {
        this->Functor(threadId);
        // If an error is raised, abort execution.
        if (this->ErrorMessage.IsErrorRaised()) { return; }
      }
    }

    FunctorType Functor;
    vtkm::exec::internal::ErrorMessageBuffer ErrorMessage;
    vtkm::Id BeginId;
    vtkm::Id EndId;
  };

  template<typename FunctorType>
  struct ScheduleKernel3D
  {
    VTKM_CONT_EXPORT
    ScheduleKernel3D(const FunctorType &functor, vtkm::Id3 maxRange)
      : Functor(functor), MaxRange(maxRange)
    {  }

    VTKM_EXEC_EXPORT
    void operator()() const
    {
      vtkm::Id3 threadId3D(this->BeginId%this->MaxRange[0],
                           (this->BeginId/this->MaxRange[0])%this->MaxRange[1],
                           this->BeginId/(this->MaxRange[0]*this->MaxRange[1]));

      for (vtkm::Id threadId = this->BeginId;
           threadId < this->EndId;
           threadId++)
      {
        this->Functor(threadId3D);
        // If an error is raised, abort execution.
        if (this->ErrorMessage.IsErrorRaised()) { return; }

        threadId3D[0]++;
        if (threadId3D[0] >= MaxRange[0])
        {
          threadId3D[0] = 0;
          threadId3D[1]++;
          if (threadId3D[1] >= MaxRange[1])
          {
            threadId3D[1] = 0;
            threadId3D[2]++;
          }
        }
      }
    }

    FunctorType Functor;
    vtkm::exec::internal::ErrorMessageBuffer ErrorMessage;
    vtkm::Id BeginId;
    vtkm::Id EndId;
    vtkm::Id3 MaxRange;
  };

  template<typename KernelType>
  VTKM_CONT_EXPORT
  static void DoSchedule(KernelType kernel,
                         vtkm::Id numInstances)
  {
    if (numInstances < 1) { return; }

    const vtkm::Id MESSAGE_SIZE = 1024;
    char errorString[MESSAGE_SIZE];
    errorString[0] = '\0';
    vtkm::exec::internal::ErrorMessageBuffer errorMessage(errorString,
                                                          MESSAGE_SIZE);
    kernel.Functor.SetErrorMessageBuffer(errorMessage);
    kernel.ErrorMessage = errorMessage;

    vtkm::Id numThreads =
        static_cast<vtkm::Id>(std::thread::hardware_concurrency());
    if (numThreads > numInstances)
    {
      numThreads = numInstances;
    }
    vtkm::Id numInstancesPerThread = (numInstances+numThreads-1)/numThreads;

    std::thread *threadPool = new std::thread[numThreads];
    vtkm::Id beginId = 0;
    for (vtkm::Id threadIndex = 0; threadIndex < numThreads; threadIndex++)
    {
      vtkm::Id endId = std::min(beginId+numInstancesPerThread, numInstances);
      KernelType threadKernel = kernel;
      threadKernel.BeginId = beginId;
      threadKernel.EndId = endId;
      std::thread newThread(threadKernel);
      threadPool[threadIndex].swap(newThread);
      beginId = endId;
    }

    for (vtkm::Id threadIndex = 0; threadIndex < numThreads; threadIndex++)
    {
      threadPool[threadIndex].join();
    }

    delete[] threadPool;

    if (errorMessage.IsErrorRaised())
    {
      throw vtkm::cont::ErrorExecution(errorString);
    }
  }

public:
  template<typename FunctorType>
  VTKM_CONT_EXPORT
  static void Schedule(FunctorType functor, vtkm::Id numInstances)
  {
    DoSchedule(ScheduleKernel1D<FunctorType>(functor), numInstances);
  }

  template<typename FunctorType>
  VTKM_CONT_EXPORT
  static void Schedule(FunctorType functor, vtkm::Id3 maxRange)
  {
    vtkm::Id numInstances = maxRange[0]*maxRange[1]*maxRange[2];
    DoSchedule(ScheduleKernel3D<FunctorType>(functor, maxRange), numInstances);
  }

  VTKM_CONT_EXPORT
  static void Synchronize()
  {
    // Nothing to do. This device schedules all of its operations using a
    // split/join paradigm. This means that the if the control threaad is
    // calling this method, then nothing should be running in the execution
    // environment.
  }
};

}
} // namespace vtkm::cont
////
//// END-EXAMPLE DeviceAdapterAlgorithmCxx11Thread.h
////

////
//// BEGIN-EXAMPLE DeviceAdapterTimerImplementationCxx11Thread.h
////
#include <chrono>

namespace vtkm {
namespace cont {

template<>
class DeviceAdapterTimerImplementation<vtkm::cont::DeviceAdapterTagCxx11Thread>
{
public:
  VTKM_CONT_EXPORT
  DeviceAdapterTimerImplementation()
  {
    this->Reset();
  }

  VTKM_CONT_EXPORT
  void Reset()
  {
    vtkm::cont::DeviceAdapterAlgorithm<vtkm::cont::DeviceAdapterTagCxx11Thread>
        ::Synchronize();
    this->StartTime = std::chrono::high_resolution_clock::now();
  }

  VTKM_CONT_EXPORT
  vtkm::Float64 GetElapsedTime()
  {
    vtkm::cont::DeviceAdapterAlgorithm<vtkm::cont::DeviceAdapterTagCxx11Thread>
        ::Synchronize();
    std::chrono::high_resolution_clock::time_point endTime =
        std::chrono::high_resolution_clock::now();

    std::chrono::high_resolution_clock::duration elapsedTicks =
        endTime - this->StartTime;

    std::chrono::duration<vtkm::Float64> elapsedSeconds(elapsedTicks);

    return elapsedSeconds.count();
  }

private:
  std::chrono::high_resolution_clock::time_point StartTime;
};

}
} // namespace vtkm::cont
////
//// END-EXAMPLE DeviceAdapterTimerImplementationCxx11Thread.h
////

////
//// BEGIN-EXAMPLE UnitTestDeviceAdapterCxx11Thread.cxx
////
//// PAUSE-EXAMPLE
// We did not really put the device adapter components in separate header
// files, but for the purposes of an example we are pretending we are.
#if 0
//// RESUME-EXAMPLE
#include <vtkm/cont/cxx11/DeviceAdapterCxx11Thread.h>
//// PAUSE-EXAMPLE
#endif
//// RESUME-EXAMPLE

#include <vtkm/cont/testing/TestingDeviceAdapter.h>

int UnitTestDeviceAdapterCxx11Thread(int, char *[])
{
  return vtkm::cont::testing::TestingDeviceAdapter<
      vtkm::cont::DeviceAdapterTagCxx11Thread>::Run();
}
////
//// END-EXAMPLE UnitTestDeviceAdapterCxx11Thread.cxx
////

int CustomDeviceAdapter(int argc, char *argv[])
{
  return UnitTestDeviceAdapterCxx11Thread(argc, argv);
}
