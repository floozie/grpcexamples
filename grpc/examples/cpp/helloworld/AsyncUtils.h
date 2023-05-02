// Copyright (c) 2022 TRUMPF Laser GmbH
// All Rights Reserved, see LICENSE.TXT for further details.

#pragma once

#include <condition_variable>
#include <memory>
#include <thread>
#include <shared_mutex>

namespace vlp::app_container {

  class Synchronizer {
  public:
    void wait() {
      std::unique_lock<std::mutex> lock(mutex_);
      cv_.wait(lock, [this] { return condition_; });
    }

    void notify() {
      std::unique_lock<std::mutex> lock(mutex_);
      condition_ = true;
      cv_.notify_one();
    }

  private:
    std::mutex mutex_;
    std::condition_variable cv_;
    bool condition_ = false;
  };

  class CancellationToken {
  public:
    CancellationToken() = default;
    CancellationToken(const CancellationToken&) = delete;
    ~CancellationToken() {
      {
        // lock the mutex_ before accessing shared data
        std::unique_lock<std::mutex> lock(mutex_);

        // set the cancellation flag and notify waiting threads
        is_cancellation_requested_ = true;
        condition_.notify_all();
      }
      {
        // lock the handlers_mutex_ before accessing shared data
        std::unique_lock<std::mutex> lock(handlers_mutex_);
        // remove all cancellation handlers
        for (auto& handler : cancellation_handlers_) {
          delete handler;
        }
      }
    }

    void RequestCancellation() {
      {
        std::lock_guard<std::mutex> guard{ mutex_ };
        if (is_cancellation_requested_) {
          return;
        }
        is_cancellation_requested_ = true;
      }
      condition_.notify_all();
      ExecuteCancellationHandlers();

    }

    bool IsCancellationRequested() {
      std::lock_guard<std::mutex> guard{ mutex_ };
      return is_cancellation_requested_;
    }

    void Wait() {
      std::unique_lock<std::mutex> lock{ mutex_ };
      condition_.wait(lock, [&] { return is_cancellation_requested_; });
    }

    void AddCancellationHandler(std::function<void()>* handler) {
      std::lock_guard<std::mutex> guard{ handlers_mutex_ };
      cancellation_handlers_.push_back(handler);
    }

    void RemoveCancellationHandler(std::function<void()>* handler) {
      std::lock_guard<std::mutex> guard{ handlers_mutex_ };
      auto it = std::find_if(cancellation_handlers_.begin(),
        cancellation_handlers_.end(),
        [&](const auto& f) { return f == handler; });
      if (it != cancellation_handlers_.end()) {
        cancellation_handlers_.erase(it);
        delete handler;
      }
    }

    /// <summary>
    /// </summary>
    /// <param name="duration"></param>
    /// <returns><c>true</c> if cancellation was requested withing
    /// <c>duration</c>, <c>false</c> if timeout was reached</returns>
    /// <remarks>Consider <c>using namespace
    /// std::chrono_literals</c></c></remarks>
    template <class _Rep, class _Period>
    bool WaitFor(const std::chrono::duration<_Rep, _Period>& duration) {
      std::unique_lock<std::mutex> lock{ mutex_ };
      return condition_.wait_until(lock,
        std::chrono::steady_clock::now() + duration,
        [&] { return is_cancellation_requested_; });
    }

  private:
    void ExecuteCancellationHandlers() {
      std::lock_guard<std::mutex> guard{ handlers_mutex_ };
      for (auto& handler_ptr : cancellation_handlers_) {
        (*handler_ptr)();
      }
    }

    std::mutex mutex_;
    std::mutex handlers_mutex_;
    std::condition_variable condition_;
    bool is_cancellation_requested_{ false };
    std::vector<std::function<void()>*> cancellation_handlers_;
  };

}  // namespace vlp::app_container
