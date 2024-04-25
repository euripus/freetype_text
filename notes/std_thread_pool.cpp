// https://dev.to/ish4n10/making-a-thread-pool-in-c-from-scratch-bnm

#include <vector>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <queue>
#include <iostream>
#include <functional>
#include <memory>

class ThreadPool
{
private:
    std::vector<std::thread>          workers;
    std::mutex                        mutex;
    std::condition_variable           cv;
    std::queue<std::function<void()>> queue;
    std::atomic<bool>                 stop;

    void worker();

public:
    ThreadPool(std::size_t nr_threads = std::max(1, std::thread::hardware_concurrency() - 1));
    ~ThreadPool();

    template<typename F, typename... Args>
    auto enqueue(F && f, Args &&... args) -> std::future<decltype(f(args...))>;

    ThreadPool(ThreadPool &)                   = delete;
    ThreadPool(ThreadPool const &)             = delete;
    ThreadPool & operator=(ThreadPool &&)      = delete;
    ThreadPool & operator=(ThreadPool const &) = delete;
};

ThreadPool::ThreadPool(std::size_t nr_workers)
{
    stop = false;
    for(auto i{0}; i < nr_workers; i++)
    {
        workers.emplace_back(&ThreadPool::worker, this);
    }
}

void ThreadPool::worker()
{
    for(;;)
    {
        std::function<void()> cur_task;
        {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [this]() { return stop || !queue.empty(); });

            if(stop && queue.empty())
                break;
            if(queue.empty())
                continue;

            cur_task = queue.front();
            queue.pop();
            // grab the fx from queue
        }
        cur_task();
    }
}

template<typename F, typename... Args>
inline auto ThreadPool::enqueue(F && f, Args &&... args) -> std::future<decltype(f(args...))>
{
    auto func             = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    auto encapsulated_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

    std::future<std::result_of_t<F(Args...)>> future_object = encapsulated_ptr->get_future();
    {
        std::unique_lock<std::mutex> lock(mutex);
        queue.emplace([encapsulated_ptr]() {
            (*encapsulated_ptr)();   // execute the fx
        });
    }
    cv.notify_one();
    return future_object;
}

ThreadPool::~ThreadPool()
{
    stop = true;

    cv.notify_all();
    for(auto & worker: workers)
    {
        worker.join();
    }
}

int main()
{
    ThreadPool                    pool(4);
    std::vector<std::future<int>> results;

    for(int i = 0; i < 8; ++i)
    {
        auto future = pool.enqueue([i] { return i + i; });
        results.emplace_back(std::move(future));
    }

    for(auto & result: results)
        std::cout << result.get() << ' ';
    std::cout << std::endl;
}
