#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <boost/log/trivial.hpp>
#include <functional>
#include <future>
#include <queue>
#include <vector>

namespace tcs {

// A thread pool implementation from
// https://github.com/jhasse/ThreadPool/
class Executor {
 public:
  Executor(size_t);

  template <class F, class... Args>
  void Submit(F&& f, Args&&... args);

  void Exit();

  ~Executor();

 private:
  // need to keep track of threads so we can join them
  std::vector<std::thread> workers_;
  // the task queue
  std::queue<std::packaged_task<void()> > tasks_;
  // synchronization
  mutable std::mutex queue_mutex_;
  std::condition_variable condition_;
  bool stop_;
};

// the constructor just launches some amount of workers
inline Executor::Executor(size_t threads) : stop_{false} {
  for (size_t i = 0; i < threads; ++i)
    workers_.emplace_back([this] {
      for (;;) {
        std::packaged_task<void()> task;

        {
          std::unique_lock<std::mutex> lock(this->queue_mutex_);
          this->condition_.wait(
              lock, [this] { return this->stop_ || !this->tasks_.empty(); });
          if (this->stop_ && this->tasks_.empty()) return;
          task = std::move(this->tasks_.front());
          this->tasks_.pop();
        }

        task();
      }
    });
}

// add new work item to the pool
template <class F, class... Args>
void Executor::Submit(F&& f, Args&&... args) {
  using return_type = std::invoke_result_t<F, Args...>;

  std::packaged_task<return_type()> task(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));

  // std::future<return_type> res = task.get_future();
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);

    // don't allow enqueueing after stopping the pool
    if (stop_) {
      BOOST_LOG_TRIVIAL(info) << "Executor stopped. Discard new tasks.";
      return;
    }

    tasks_.emplace(std::move(task));
  }
  condition_.notify_one();
}

inline void Executor::Exit() {
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    if (stop_) return;
    stop_ = true;
  }
  condition_.notify_all();
  for (std::thread& worker : workers_) {
    worker.join();
  }
}

// the destructor joins all threads
inline Executor::~Executor() { Exit(); }

}  // namespace tcs

#endif /* EXECUTOR_H */
