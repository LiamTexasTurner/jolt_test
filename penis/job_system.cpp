#include "job_system.h"

#include <algorithm>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <sstream>
#include <cassert>
#include <functional>
#include <mutex>
#include <vector>

#ifdef _WIN32

#define NOMINMAX
#include <Windows.h>

#endif

template<typename T, size_t capacity>
class ThreadSafeRingBuffer
{
public:
      bool push_back(const T& item)
      {
            std::lock_guard<std::mutex> guard(lock);

            size_t next = (head + 1) % capacity;

            if (next == tail)
            {
                  return false;
            }

            data[head] = item;
            head = next;

            return true;
      }

      bool pop_front(T& item)
      {
            std::lock_guard<std::mutex> guard(lock);

            if (tail == head)
            {
                  return false;
            }

            item = std::move(data[tail]);
            tail = (tail + 1) % capacity;

            return true;
      }

private:
      T data[capacity];
      size_t head = 0;
      size_t tail = 0;
      std::mutex lock;
};

struct ThreadContext
{
      Arena arena;
};

namespace pJobSystem
{
      std::vector<ThreadContext> thread_context;
      std::vector<std::thread> worker_threads;

      uint32_t numThreads = 0;

      ThreadSafeRingBuffer<std::function<void(Arena&)>, 256> jobPool;

      std::condition_variable wakeCondition;
      std::mutex wakeMutex;

      uint64_t currentLabel = 0;
      std::atomic<uint64_t> finishedLabel{0};

      size_t queuedJobs = 0;
      bool running = false;

      void Initialize()
      {
            if (running)
            {
                  return;
            }

            finishedLabel.store(0);
            currentLabel = 0;
            queuedJobs = 0;
            running = true;

            uint32_t numCores = std::thread::hardware_concurrency();

            if (numCores == 0)
            {
                  numCores = 1;
            }

            numThreads = std::max(1u, numCores);

            thread_context.resize(numThreads);
            worker_threads.reserve(numThreads);

            for (uint32_t threadID = 0; threadID < numThreads; ++threadID)
            {
                  worker_threads.emplace_back(
                        [threadID]()
                        {
                              Arena& arena = thread_context[threadID].arena;

                              while (true)
                              {
                                    std::function<void(Arena&)> job;

                                    {
                                          std::unique_lock<std::mutex> lock(wakeMutex);

                                          wakeCondition.wait(
                                                lock,
                                                []()
                                                {
                                                      return !running || queuedJobs > 0;
                                                }
                                          );

                                          if (!running && queuedJobs == 0)
                                          {
                                                break;
                                          }

                                          if (!jobPool.pop_front(job))
                                          {
                                                continue;
                                          }

                                          --queuedJobs;
                                    }

                                    arena_reset(&arena);
                                    job(arena);

                                    finishedLabel.fetch_add(
                                          1,
                                          std::memory_order_release
                                    );
                              }
                        }
                  );

#ifdef _WIN32

                  std::thread& worker = worker_threads.back();
                  HANDLE handle = reinterpret_cast<HANDLE>(worker.native_handle());

                  if (threadID < sizeof(DWORD_PTR) * 8)
                  {
                        DWORD_PTR affinityMask = DWORD_PTR{1} << threadID;
                        DWORD_PTR affinityResult = SetThreadAffinityMask(handle, affinityMask);

                        assert(affinityResult > 0);
                  }

                  std::wstringstream name;
                  name << L"P_JobSystem_" << threadID;

                  HRESULT result = SetThreadDescription(handle, name.str().c_str());

                  assert(SUCCEEDED(result));

#endif
            }
      }

      void poll()
      {
            wakeCondition.notify_one();
            std::this_thread::yield();
      }

      void Execute(const std::function<void(Arena&)>& job)
      {
            assert(running);

            ++currentLabel;

            while (true)
            {
                  bool pushed = false;

                  {
                        std::lock_guard<std::mutex> lock(wakeMutex);

                        pushed = jobPool.push_back(job);

                        if (pushed)
                        {
                              ++queuedJobs;
                        }
                  }

                  if (pushed)
                  {
                        wakeCondition.notify_one();
                        return;
                  }

                  poll();
            }
      }

      bool IsBusy()
      {
            return finishedLabel.load(std::memory_order_acquire) < currentLabel;
      }

      void Wait()
      {
            while (IsBusy())
            {
                  poll();
            }
      }

      void Dispatch(
            uint32_t jobCount,
            uint32_t groupSize,
            const std::function<void(JobDispatchArgs, Arena&)>& job)
      {
            assert(running);

            if (jobCount == 0 || groupSize == 0)
            {
                  return;
            }

            uint32_t groupCount =
                  (jobCount + groupSize - 1) / groupSize;

            currentLabel += groupCount;

            for (uint32_t groupIndex = 0; groupIndex < groupCount; ++groupIndex)
            {
                  std::function<void(Arena&)> jobGroup =
                        [jobCount, groupSize, job, groupIndex](Arena& arena)
                        {
                              uint32_t groupJobOffset =
                                    groupIndex * groupSize;

                              uint32_t groupJobEnd =
                                    std::min(
                                          groupJobOffset + groupSize,
                                          jobCount
                                    );

                              JobDispatchArgs args;
                              args.groupIndex = groupIndex;

                              for (uint32_t i = groupJobOffset; i < groupJobEnd; ++i)
                              {
                                    args.jobIndex = i;
                                    job(args, arena);
                              }
                        };

                  while (true)
                  {
                        bool pushed = false;

                        {
                              std::lock_guard<std::mutex> lock(wakeMutex);

                              pushed = jobPool.push_back(jobGroup);

                              if (pushed)
                              {
                                    ++queuedJobs;
                              }
                        }

                        if (pushed)
                        {
                              wakeCondition.notify_one();
                              break;
                        }

                        poll();
                  }
            }
      }

      void Shutdown()
      {
            if (!running)
            {
                  return;
            }

            Wait();

            {
                  std::lock_guard<std::mutex> lock(wakeMutex);
                  running = false;
            }

            wakeCondition.notify_all();

            for (std::thread& worker : worker_threads)
            {
                  if (worker.joinable())
                  {
                        worker.join();
                  }
            }

            worker_threads.clear();
            thread_context.clear();

            numThreads = 0;
            queuedJobs = 0;
            currentLabel = 0;
            finishedLabel.store(0);
      }
}
