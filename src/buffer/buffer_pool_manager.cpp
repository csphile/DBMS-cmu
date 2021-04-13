//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/buffer_pool_manager.h"

#include <list>
#include <unordered_map>

namespace bustub {

BufferPoolManager::BufferPoolManager(size_t pool_size, DiskManager *disk_manager, LogManager *log_manager)
    : pool_size_(pool_size), disk_manager_(disk_manager), log_manager_(log_manager) {
  // We allocate a consecutive memory space for the buffer pool.
  pages_ = new Page[pool_size_];
  replacer_ = new LRUReplacer(pool_size);

  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(static_cast<int>(i));
  }
}

BufferPoolManager::~BufferPoolManager() {
  delete[] pages_;
  delete replacer_;
}

Page *BufferPoolManager::FetchPageImpl(page_id_t page_id) {
  // 1.     Search the page table for the requested page (P).
  // 1.1    If P exists, pin it and return it immediately.
  // 1.2    If P does not exist, find a replacement page (R) from either the free list or the replacer.
  //        Note that pages are always found from the free list first.
  // 2.     If R is dirty, write it back to the disk.
  // 3.     Delete R from the page table and insert P.
  // 4.     Update P's metadata, read in the page content from disk, and then return a pointer to P.
  std::lock_guard<std::mutex> guard(latch_);
  // Step 1
  frame_id_t frame_id;
  auto it = page_table_.find(page_id);
  if (it != page_table_.end()) {
    pages_[it->second].pin_count_++;
    replacer_->Pin(it->second);
    return &pages_[it->second];
  }
  // 1.2
  if (!free_list_.empty()) {
    frame_id = free_list_.front();
    free_list_.pop_front();
  } else {
    if (!replacer_->Victim(&frame_id)) {
      return nullptr;
    }
    // 2.0 write it back to the disk;
    if (pages_[frame_id].is_dirty_) {
      disk_manager_->WritePage(pages_[frame_id].GetPageId(), pages_[frame_id].GetData());
    }
    // delete and insert P
    page_table_.erase(pages_[frame_id].GetPageId());
  }
  // 4 read in the page content
  pages_[frame_id].ResetMemory();
  disk_manager_->ReadPage(page_id, pages_[frame_id].GetData());

  pages_[frame_id].page_id_ = page_id;
  page_table_[page_id] = frame_id;
  // update P's metadata and read in the page content from disk
  pages_[frame_id].pin_count_++;
  replacer_->Pin(frame_id);
  pages_[frame_id].is_dirty_ = false;
  // return a pointer
  return &pages_[frame_id];
}

bool BufferPoolManager::UnpinPageImpl(page_id_t page_id, bool is_dirty) {
  std::lock_guard<std::mutex> guard(latch_);
  auto it = page_table_.find(page_id);
  if (it == page_table_.end() || pages_[it->second].pin_count_ <= 0) {
    return false;
  }
  pages_[it->second].pin_count_--;
  if (is_dirty) {
    pages_[it->second].is_dirty_ = true;
  }
  if (pages_[it->second].pin_count_ == 0) {
    replacer_->Unpin(it->second);
  }
  return true;
}

bool BufferPoolManager::FlushPageImpl(page_id_t page_id) {
  // Make sure you call DiskManager::WritePage!
  std::lock_guard<std::mutex> guard(latch_);
  auto it = page_table_.find(page_id);
  if (it == page_table_.end()) {
    return false;
  }
  if (pages_[it->second].is_dirty_) {
    disk_manager_->WritePage(page_id, pages_[it->second].GetData());
  }
  return true;
}

Page *BufferPoolManager::NewPageImpl(page_id_t *page_id) {
  // 0.   Make sure you call DiskManager::AllocatePage!
  // 1.   If all the pages in the buffer pool are pinned, return nullptr.
  // 2.   Pick a victim page P from either the free list or the replacer. Always pick from the free list first.
  // 3.   Update P's metadata, zero out memory and add P to the page table.
  // 4.   Set the page ID output parameter. Return a pointer to P.
  std::lock_guard<std::mutex> guard(latch_);
  frame_id_t frame_id;
  if (!free_list_.empty()) {
    frame_id = free_list_.front();
    free_list_.pop_front();
  } else {
    if (!replacer_->Victim(&frame_id)) {
      return nullptr;
    }
  }
  if (pages_[frame_id].is_dirty_) {
    disk_manager_->WritePage(pages_[frame_id].GetPageId(), pages_[frame_id].GetData());
  }
  page_table_.erase(pages_[frame_id].GetPageId());
  // update P's metadata
  *page_id = disk_manager_->AllocatePage();
  page_table_[*page_id] = frame_id;
  pages_[frame_id].ResetMemory();
  pages_[frame_id].page_id_ = *page_id;
  pages_[frame_id].pin_count_++;
  pages_[frame_id].is_dirty_ = false;
  replacer_->Pin(frame_id);
  return &pages_[frame_id];
}

bool BufferPoolManager::DeletePageImpl(page_id_t page_id) {
  // 0.   Make sure you call DiskManager::DeallocatePage!
  // 1.   Search the page table for the requested page (P).
  // 1.   If P does not exist, return true.
  // 2.   If P exists, but has a non-zero pin-count, return false. Someone is using the page.
  // 3.   Otherwise, P can be deleted. Remove P from the page table, reset its metadata and return it to the free list.
  std::lock_guard<std::mutex> guard(latch_);

  auto it = page_table_.find(page_id);
  if (it == page_table_.end()) {
    disk_manager_->DeallocatePage(page_id);
    return true;
  }
  if (pages_[it->second].GetPinCount() > 0) {
    return false;
  }
  Page *to_delete = &pages_[it->second];
  free_list_.emplace_back(it->second);
  page_table_.erase(it);
  to_delete->ResetMemory();
  to_delete->page_id_ = INVALID_PAGE_ID;
  to_delete->is_dirty_ = false;
  to_delete->pin_count_ = 0;
  return true;
}

void BufferPoolManager::FlushAllPagesImpl() {
  for (auto it : page_table_) {
    FlushPageImpl(it.first);
  }
}

}  // namespace bustub
