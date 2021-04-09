//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_replacer.cpp
//
// Identification: src/buffer/lru_replacer.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_replacer.h"

namespace bustub {

LRUReplacer::LRUReplacer(size_t num_pages) : max_size(num_pages) {}

LRUReplacer::~LRUReplacer() = default;

bool LRUReplacer::Victim(frame_id_t *frame_id) {
  const std::lock_guard<std::mutex> lock(_lock_data);
  // remove the lru element from the replacer
  if (this->Size() == 0) {
    return false;
  }
  id2data.erase(_data.front());
  *frame_id = _data.front();
  _data.pop_front();
  return true;
}

void LRUReplacer::Pin(frame_id_t frame_id) {
  // if frame_id doesn't exist, do nothing
  // remove from the replacer
  const std::lock_guard<std::mutex> lock(_lock_data);
  auto it = id2data.find(frame_id);
  if (it != id2data.end()) {
    _data.erase(it->second);
    id2data.erase(it);
  }
}

void LRUReplacer::Unpin(frame_id_t frame_id) {
  // if frame_id already exists do nothing
  // otherwise add to the replacer
  const std::lock_guard<std::mutex> lock(_lock_data);
  auto it = id2data.find(frame_id);
  if (it == id2data.end()) {
    _data.push_back(frame_id);
    id2data[frame_id] = prev(_data.end());
  }
}

size_t LRUReplacer::Size() { return _data.size(); }

}  // namespace bustub
