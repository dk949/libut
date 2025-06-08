#ifndef UT_MT_QUEUE_HPP
#define UT_MT_QUEUE_HPP

#if __cplusplus < 202'002L
#    error this file has to be compiled with at least C++20
#endif

#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <queue>

namespace ut {
template<typename T>
struct is_queue_like : std::false_type { };

template<typename T, typename Container>
struct is_queue_like<std::queue<T, Container>> : std::true_type { };

template<typename T, typename Container, typename Compare>
struct is_queue_like<std::priority_queue<T, Container, Compare>> : std::true_type { };

template<typename T, typename Allocator>
struct is_queue_like<std::deque<T, Allocator>> : std::true_type { };

template<typename T>
inline constexpr bool is_queue_like_v = is_queue_like<T>::value;

template<typename T>
concept QueueLike = is_queue_like_v<T>;

/**
 * A Multi-threading safe queue.
 *
 * All public member functions are safe to call from multiple different threads.
 *
 * *Not* lock free and generally *not* suitable for high performance applications.
 *
 * All inputs and outputs are by value, to simplify synchronisation.
 *
 * By default uses `std::queue` as the underlying container.
 * Can also use `std::priority_queue` or `std::deque`.
 *
 * NOTE: If the queue is a priority_queue, it cannot be used with move-only objects.
 *
 *       Move-only objects also disable all front* methods, as they require copying the element.
 *
 *       std::shared_ptr can be used to avoid this issue, but it is not recommended. (also note that the
 *       default comparitor for a shared_ptr in a priority_queue is not very useful)
 */

template<typename T, QueueLike Q = std::queue<T>>
class MtQueue {
public:
    /**
     * Push an item to the back of the queue
     */
    void push(T t) {
        {
            std::lock_guard<std::mutex> g {m_mu};
            pushImpl(std::move(t));
        }
        m_cv.notify_one();
    }

    /**
     * Removes an item from the front of the queue and returns it.
     *
     * If the queue is empty, blocks until it is not empty.
     *
     * Moves the item if this is supported (i.e. not when using a `priority_queue`)
     */
    T pop() {
        std::unique_lock<std::mutex> g {m_mu};
        m_cv.wait(g, [this] { return !m_q.empty(); });
        return popImpl();
    }

    /**
     * Just like `pop`, but if the queue is empty, returns `std::nullopt`
     */
    std::optional<T> tryPop() {
        std::lock_guard<std::mutex> g {m_mu};
        if (m_q.empty()) return std::nullopt;
        return popImpl();
    }

    /**
     * Like `tryPop`, but waits for `duration` before returning `std::nullopt`
     */
    template<class Rep, class Period>
    std::optional<T> tryPopFor(std::chrono::duration<Rep, Period> const &duration) {
        std::unique_lock<std::mutex> g {m_mu};
        if (m_cv.wait_for(g, duration, [this]() { return !m_q.empty(); })) return popImpl();
        return std::nullopt;
    }

    /**
     * Take a copy of the front of the queue.
     *
     * Blocks until queue is not empty
     *
     * Does not remove item from the queue.
     */
    [[nodiscard]]
    T front() {
        std::unique_lock<std::mutex> g {m_mu};
        m_cv.wait(g, [this] { return !m_q.empty(); });
        return frontImpl();
    }

    /**
     * Take a copy of the front of the queue.
     *
     * Blocks until queue is not empty
     *
     * Does not remove item from the queue
     */
    [[nodiscard]]
    T front() const {
        std::unique_lock<std::mutex> g {m_mu};
        m_cv.wait(g, [this] { return !m_q.empty(); });
        return frontImpl();
    }

    /**
     * Like `front`, but if the queue is empty, returns `std::nullopt`
     */
    [[nodiscard]]
    std::optional<T> tryFront() {
        std::lock_guard<std::mutex> g {m_mu};
        if (m_q.empty()) return std::nullopt;
        return frontImpl();
    }

    /**
     * Like `front`, but if the queue is empty, returns `std::nullopt`
     */
    [[nodiscard]]
    std::optional<T> tryFront() const {
        std::lock_guard<std::mutex> g {m_mu};
        if (m_q.empty()) return std::nullopt;
        return frontImpl();
    }

    /**
     * Like `tryFront`, but waits for `duration` before returning `std::nullopt`
     */
    template<class Rep, class Period>
    [[nodiscard]]
    std::optional<T> tryFrontFor(std::chrono::duration<Rep, Period> dur) {
        std::unique_lock<std::mutex> g {m_mu};
        if (m_cv.wait_for(g, dur, [this]() { return !m_q.empty(); })) return frontImpl();
        return std::nullopt;
    }

    /**
     * Like `tryFront`, but waits for `duration` before returning `std::nullopt`
     */
    template<class Rep, class Period>
    [[nodiscard]]
    std::optional<T> tryFrontFor(std::chrono::duration<Rep, Period> dur) const {
        std::unique_lock<std::mutex> g {m_mu};
        if (m_cv.wait_for(g, dur, [this]() { return !m_q.empty(); })) return frontImpl();
        return std::nullopt;
    }

    /**
     * Is the queue empty.
     */
    [[nodiscard]]
    bool empty() const {
        std::lock_guard<std::mutex> g {m_mu};
        return m_q.empty();
    }
private:
    mutable std::mutex m_mu;
    mutable std::condition_variable m_cv;
    Q m_q;
    static constexpr bool has_top = requires {
        { m_q.top() } -> std::same_as<T const &>;
    };
    static constexpr bool has_front = requires {
        { m_q.front() } -> std::same_as<T const &>;
    }
    || requires {
        { m_q.front() } -> std::same_as<T &>;
    };
    static constexpr bool has_push = requires(T t) {
        m_q.push(t);
    };
    static constexpr bool has_push_back = requires(T t) {
        m_q.push_back(t);
    };
    static constexpr bool has_pop = requires {
        m_q.pop();
    };
    static constexpr bool has_pop_front = requires {
        m_q.pop_front();
    };

    static_assert(has_top != has_front, "Queue has to have either top or front, but not both");
    static_assert(has_push != has_push_back, "Queue has to have either push or push_back, but not both");
    static_assert(has_pop != has_pop_front, "Queue has to have either pop or pop_front, but not both");

    [[nodiscard]]
    decltype(auto) frontImpl() requires(has_top) {
        return m_q.top();
    }

    [[nodiscard]]
    decltype(auto) frontImpl() const requires(has_top) {
        return m_q.top();
    }

    [[nodiscard]]
    decltype(auto) frontImpl() requires(has_front) {
        return m_q.front();
    }

    [[nodiscard]]
    decltype(auto) frontImpl() const requires(has_front) {
        return m_q.front();
    }

    [[nodiscard]]
    T popImpl() requires(has_pop) {
        auto t = std::move(frontImpl());
        m_q.pop();
        return t;
    }

    [[nodiscard]]
    T popImpl() requires(has_pop_front) {
        auto t = std::move(frontImpl());
        m_q.pop_front();
        return t;
    }

    void pushImpl(T &&t) requires(has_push) {
        m_q.push(std::move(t));
    }

    void pushImpl(T &&t) requires(has_push_back) {
        m_q.push_back(std::move(t));
    }
};
}  // namespace ut

#endif  // UT_MT_QUEUE_HPP
