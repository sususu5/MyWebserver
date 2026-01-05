#include "heaptimer.h"

void HeapTimer::swapNode_(size_t i, size_t j) {
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
}

void HeapTimer::siftUp_(size_t i) {
    assert(i >= 0 && i < heap_.size());
    size_t parent = (i - 1) / 2;
    while (parent >= 0) {
        if (heap_[parent] > heap_[i]) {
            swapNode_(i, parent);
            i = parent;
            parent = (i - 1) / 2;
        } else {
            break;
        }
    }
}

bool HeapTimer::siftDown_(size_t index, size_t n) {
    assert(index >= 0 && index < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    auto i = index;
    auto child = 2 * i + 1;
    while (child < n) {
        if (child + 1 < n && heap_[child + 1] < heap_[child]) child++;
        if (heap_[i] < heap_[child]) break;
        swapNode_(i, child);
        i = child;
        child = 2 * i + 1;
    }
    return i > index;
}

void HeapTimer::del_(size_t index) {
    assert(index >= 0 && index < heap_.size());
    size_t tmp = index;
    size_t n = heap_.size();
    assert(tmp <= n);
    if (index < heap_.size() - 1) {
        // Move the node to be deleted to the end of the heap_
        swapNode_(index, heap_.size() - 1);
        if (!siftDown_(index, heap_.size() - 1)) {
            siftUp_(index);
        }
    }
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void HeapTimer::adjust(int id, int newExpires) {
    assert(!heap_.empty() && ref_.count(id) > 0);
    heap_[ref_[id]].expires = Clock::now() + MS(newExpires);
    siftDown_(ref_[id], heap_.size());
}

void HeapTimer::add(int id, int timeOut, const TimeoutCallBack& cb) {
    assert(id >= 0);
    if (ref_.count(id)) {
        // If the timer already exists, update the expiration time
        int tmp = ref_[id];
        heap_[tmp].expires = Clock::now() + MS(timeOut);
        heap_[tmp].cb = cb;
        if (!siftDown_(tmp, heap_.size())) {
            siftUp_(tmp);
        }
    } else {
        // If the timer does not exist, add a new timer
        size_t n = heap_.size();
        ref_[id] = n;
        heap_.push_back({id, Clock::now() + MS(timeOut), cb});
        siftUp_(n);
    }
}

// Delete the specified timer and trigger the callback function
void HeapTimer::doWork(int id) {
    if (heap_.empty() || ref_.count(id) == 0) return;
    size_t i = ref_[id];
    auto node = heap_[i];
    node.cb();
    del_(i);
}

// Clear the timer which has expired
void HeapTimer::tick() {
    if (heap_.empty()) return;
    while (!heap_.empty()) {
        TimerNode node = heap_.front();
        if (std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) break;
        node.cb();
        pop();
    }
}

void HeapTimer::pop() {
    assert(heap_.size() > 0);
    del_(0);
}

void HeapTimer::clear() {
    ref_.clear();
    heap_.clear();
}

int HeapTimer::getNextTick() {
    tick();
    size_t res = -1;
    if (!heap_.empty()) {
        res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
        if (res < 0) res = 0;
    }
    return res;
}