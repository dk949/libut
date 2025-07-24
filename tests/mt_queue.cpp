//  NOLINTBEGIN(readability-magic-numbers,google-build-using-namespace)
#include <ut/mt_queue/mt_queue.hpp>
#include <ut/overload/overload.hpp>

#include <barrier>
#include <chrono>
#include <numeric>
#include <thread>
#include <utility>
namespace chr = std::chrono;
using namespace std::chrono_literals;

#include <catch.hpp>
using namespace ut;

TEMPLATE_TEST_CASE("Single threaded", "[mt_queue]", std::queue<int>, std::deque<int>, std::priority_queue<int>) {
    MtQueue<int, TestType> q;
    SECTION("push, front, pop, empty") {
        // NOTE: pushing in order that will be preserved by the priority queueu
        q.push(30);
        q.push(20);
        q.push(10);

        REQUIRE(!q.empty());
        REQUIRE(q.front() == 30);
        REQUIRE(q.pop() == 30);
        REQUIRE(q.front() == 20);
        REQUIRE(q.pop() == 20);
        REQUIRE(q.front() == 10);
        REQUIRE(q.pop() == 10);
        REQUIRE(q.empty());
    }


    SECTION("tryPop") {
        REQUIRE(!q.tryPop().has_value());
        q.push(7);
        auto p = q.tryPop();
        REQUIRE(p.has_value());
        REQUIRE(*p == 7);
    }

    SECTION("tryFront") {
        REQUIRE(!q.tryFront().has_value());
        q.push(7);
        auto p = q.tryFront();
        REQUIRE(p.has_value());
        REQUIRE(*p == 7);
    }

    SECTION("tryPopFor") {
        auto now = chr::high_resolution_clock::now();
        REQUIRE(!q.tryPopFor(50ms).has_value());
        REQUIRE(chr::high_resolution_clock::now() - now >= 50ms);
        q.push(7);
        now = chr::high_resolution_clock::now();
        auto p = q.tryPopFor(50ms);
        REQUIRE(chr::high_resolution_clock::now() - now < 50ms);
        REQUIRE(p.has_value());
        REQUIRE(*p == 7);
    }

    SECTION("tryFrontFor") {
        auto now = chr::high_resolution_clock::now();
        REQUIRE(!q.tryFrontFor(50ms).has_value());
        REQUIRE(chr::high_resolution_clock::now() - now >= 50ms);
        q.push(7);
        now = chr::high_resolution_clock::now();
        auto p = q.tryFrontFor(50ms);
        REQUIRE(chr::high_resolution_clock::now() - now < 50ms);
        REQUIRE(p.has_value());
        REQUIRE(*p == 7);
    }

    SECTION("underLock") {
        q.underLock([](auto &inner) {
            ut::Overload {
                [&](std::queue<int> &inner_q) { inner_q.push(10); },
                [&](std::deque<int> &inner_q) { inner_q.push_back(10); },
                [&](std::priority_queue<int> &inner_q) { inner_q.push(10); },
            }(inner);
            auto back = ut::Overload {
                [&](std::queue<int> &inner_q) { return inner_q.back(); },
                [&](std::deque<int> &inner_q) { return inner_q.back(); },
                [&](std::priority_queue<int> &inner_q) { return inner_q.top(); },
            }(inner);
            REQUIRE(back == 10);
        });
        std::as_const(q).underLock([](auto &inner) { REQUIRE(!inner.empty()); });
    }
}

TEMPLATE_TEST_CASE("Multi-threaded", "[mt_queue]", std::queue<int>, std::deque<int>, std::priority_queue<int>) {
    MtQueue<int, TestType> q;

    SECTION("pop, front") {
        std::barrier b {2};
        auto producer = std::jthread([&q, &b]() {
            REQUIRE(q.empty());
            b.arrive_and_wait();
            std::this_thread::sleep_for(30ms);
            q.push(30);
            std::this_thread::sleep_for(30ms);
            q.push(20);
        });
        auto consumer = std::jthread([&q, &b]() {
            b.arrive_and_wait();
            auto now = chr::high_resolution_clock::now();
            REQUIRE(q.front() == 30);
            REQUIRE(!q.empty());
            REQUIRE(std::chrono::high_resolution_clock::now() - now >= 30ms);
            REQUIRE(q.pop() == 30);
            REQUIRE(q.pop() == 20);
            REQUIRE(std::chrono::high_resolution_clock::now() - now >= 60ms);
        });
    }
    SECTION("tryPop") {
        auto producer = std::jthread([&q]() {
            std::this_thread::sleep_for(30ms);
            q.push(30);
        });
        auto consumer = std::jthread([&q]() {
            REQUIRE(!q.tryFront().has_value());
            REQUIRE(!q.tryPop().has_value());
            std::this_thread::sleep_for(40ms);
            auto f = q.tryFront();
            auto p = q.tryPop();
            REQUIRE(f.has_value());
            REQUIRE(*f == 30);
            REQUIRE(p.has_value());
            REQUIRE(*p == 30);
        });
    }
}

TEMPLATE_TEST_CASE("2 threaded stress test", "[mt_queue]", std::queue<int>, std::deque<int>, std::priority_queue<int>) {
    MtQueue<int, TestType> q;

    auto producer = std::jthread([&q]() {
        for (int i = 10000; i > 0; --i) {
            q.push(i);
        }
    });
    auto consumer = std::jthread([&q]() {
        for (int i = 10000; i > 0; --i) {
            REQUIRE(q.front() == i);
            REQUIRE(q.pop() == i);
        }
    });
}

TEMPLATE_TEST_CASE("multi producer single consumer stress test", "[mt_queue]", std::queue<int>, std::deque<int>
    // , std::priority_queue<int>
) {
    MtQueue<int, TestType> q;


    std::stop_source stop;

    auto producer = std::jthread([&q, &stop]() {
        for (int i = 10000; i > 0; --i) {
            q.push(i);
        }
        stop.request_stop();
    });
    std::vector<std::jthread> consumers;
    std::vector<std::vector<int>> consumer_fronts {10};
    std::vector<std::vector<int>> consumer_pops {10};
    for (size_t i = 0; i < 10; ++i) {
        consumers.emplace_back([&q, &pops = consumer_pops[i], &fronts = consumer_fronts[i], token = stop.get_token()]() {
            while (!(token.stop_requested() && q.empty())) {
                auto const front = q.tryFront();
                if (!front.has_value()) continue;
                fronts.push_back(*front);
                auto const pop = q.tryPop();
                if (!pop.has_value()) continue;
                pops.push_back(*pop);
            }
        });
    }

    producer.join();
    for (auto &c : consumers) {
        c.join();
    }

    REQUIRE(q.empty());
    REQUIRE(consumer_fronts.size() == consumer_pops.size());
    for (size_t i = 0; i < consumer_fronts.size(); ++i) {
        auto const &fronts = consumer_fronts[i];
        auto const &pops = consumer_pops[i];
        for (size_t j = 0; j < std::min(fronts.size(), pops.size()); ++j) {
            REQUIRE(fronts[j] >= pops[j]);
        }
    }


    auto const sum_pops =
        std::accumulate(consumer_pops.begin(), consumer_pops.end(), std::size_t {}, [](size_t sum, auto const &v) {
        return sum + v.size();
    });

    // Not checking this for fronts. sum of all fronts sizes can be greater than 10000 if a successful tryFront is
    // followed by an unsuccessful tryPop and then another successful tryFront.
    REQUIRE(sum_pops == 10000);
}

TEMPLATE_TEST_CASE("Single trheaded using unique_ptr",
    "[mt_queue]",
    std::queue<std::unique_ptr<int>>,
    std::deque<std::unique_ptr<int>>) {
    MtQueue<std::unique_ptr<int>, TestType> q;

    q.push(std::make_unique<int>(30));
    q.push(std::make_unique<int>(20));
    q.push(std::make_unique<int>(10));
    REQUIRE(*q.pop() == 30);
    REQUIRE(*q.pop() == 20);
    REQUIRE(*q.pop() == 10);
}

TEMPLATE_TEST_CASE("Single trheaded using shared_ptr",
    "[mt_queue]",
    std::queue<std::shared_ptr<int>>,
    std::deque<std::shared_ptr<int>>,
    std::priority_queue<std::shared_ptr<int>>) {

    MtQueue<std::shared_ptr<int>, TestType> q;

    q.push(std::make_shared<int>(30));
    auto front = q.front();
    REQUIRE(*q.pop() == 30);
    REQUIRE(q.empty());
    // pointer still valid after queue is empty
    REQUIRE(*front == 30);
}

//  NOLINTEND(readability-magic-numbers,google-build-using-namespace)
