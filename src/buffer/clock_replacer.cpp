//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// clock_replacer.cpp
//
// Identification: src/buffer/clock_replacer.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/clock_replacer.h"

namespace bustub {

ClockReplacer::ClockReplacer(size_t num_pages):sz_(num_pages){}

ClockReplacer::~ClockReplacer() = default;

bool ClockReplacer::Victim(frame_id_t *frame_id) {
  if (frames.empty()) {
    return false;
  }
  while (frames.front().second != 0) {
    auto p = frames.front();
    frames.pop_front();
    p.second = 0;
    frames.push_back(p);
    // maintain pointers
    mp[p.first] = prev(frames.end());
  }
  auto p = frames.front();
  frames.pop_front();
  *frame_id = p.first;
  mp.erase(p.first);
  return true;
}

void ClockReplacer::Pin(frame_id_t frame_id) {
  auto it = mp.find(frame_id);
  if (it == mp.end()) {
    return;
  }
  frames.erase(it->second);
  mp.erase(frame_id);
}

void ClockReplacer::Unpin(frame_id_t frame_id) {
  if (mp.count(frame_id) != 0U) {
    return ;
}
  frames.emplace_back(frame_id, 1);
  mp[frame_id] = prev(frames.end());
}

size_t ClockReplacer::Size() { return frames.size(); }

}  // namespace bustub
