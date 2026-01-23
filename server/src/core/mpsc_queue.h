#pragma once

#include <atomic>
#include <optional>
#include <utility>

template <typename T>
class MPSCQueue {
public:
    MPSCQueue() {
        Node* dummy = new Node();
        head_.store(dummy, std::memory_order_relaxed);
        tail_.store(dummy, std::memory_order_relaxed);
    };

    ~MPSCQueue() {
        while (dequeue().has_value());
        delete head_.load(std::memory_order_relaxed);
    };

    void enqueue(T val) {
        Node* new_node = new Node(std::move(val));
        while (true) {
            Node* old_tail = tail_.load(std::memory_order_acquire);
            Node* next = old_tail->next.load(std::memory_order_acquire);
            if (old_tail == tail_.load(std::memory_order_acquire)) {
                if (next == nullptr) {
                    if (old_tail->next.compare_exchange_weak(next, new_node, std::memory_order_release,
                                                             std::memory_order_relaxed)) {
                        tail_.compare_exchange_strong(old_tail, new_node, std::memory_order_release,
                                                      std::memory_order_relaxed);
                        return;
                    }
                } else {
                    tail_.compare_exchange_strong(old_tail, next, std::memory_order_release, std::memory_order_relaxed);
                }
            }
        }
    }

    std::optional<T> dequeue() {
        Node* head = head_.load(std::memory_order_acquire);
        Node* next = head->next.load(std::memory_order_acquire);

        if (next == nullptr) {
            return std::nullopt;
        }

        std::optional<T> result = std::move(next->data);
        head_.store(next, std::memory_order_release);
        delete head;
        return result;
    }

    template <typename OutputIt>
    size_t dequeue_bulk(OutputIt out, size_t max_count) {
        size_t count = 0;
        while (count < max_count) {
            auto res = dequeue();
            if (!res) break;
            *out++ = std::move(res.value());
            count++;
        }
        return count;
    }

    bool empty() const {
        Node* head = head_.load(std::memory_order_acquire);
        Node* next = head->next.load(std::memory_order_acquire);
        return next == nullptr;
    };

private:
    struct Node {
        T data;
        std::atomic<Node*> next;

        Node() : next(nullptr) {}
        explicit Node(T val) : data(std::move(val)), next(nullptr) {}
    };

    alignas(64) std::atomic<Node*> head_;
    alignas(64) std::atomic<Node*> tail_;
};
