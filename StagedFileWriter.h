#pragma once

#include <Arduino.h>
#include <FS.h>

class StagedFileWriter {
 public:
  bool begin(fs::FS& storage, const String& target, const String& temporary,
             uint32_t limit, uint32_t freeBytes) {
    abort();
    storage_ = &storage;
    target_ = target;
    temporary_ = temporary;
    backup_ = target + ".bak";
    limit_ = limit;
    freeBytes_ = freeBytes;
    total_ = 0;
    File targetFile = storage.open(target, "r");
    targetSize_ = targetFile ? targetFile.size() : 0;
    if (targetFile) targetFile.close();
    storage.remove(temporary_);
    existing_ = storage.open(target_, "r");
    matching_ = (bool)existing_;
    changed_ = !matching_;
    good_ = true;
    error_ = "";
    if (changed_) good_ = openStage();
    return good_;
  }

  bool write(const uint8_t* data, size_t length) {
    if (!good_) return false;
    if (total_ + length > limit_) return fail("Payload too large.");

    if (matching_) {
      uint8_t compare[256];
      size_t offset = 0;
      bool equal = true;
      while (offset < length) {
        size_t amount = min(sizeof(compare), length - offset);
        if (existing_.read(compare, amount) != (int)amount ||
            memcmp(compare, data + offset, amount) != 0) {
          equal = false;
          break;
        }
        offset += amount;
      }
      if (equal) {
        total_ += length;
        return true;
      }
      matching_ = false;
      changed_ = true;
      existing_.close();
      if (!openStage() || !copyPrefix(total_)) return false;
    }

    if (!reserve(length) || stage_.write(data, length) != length) {
      return fail("Storage write failed.");
    }
    total_ += length;
    return true;
  }

  bool finishInput() {
    if (!good_) return false;
    if (!total_) return fail("File is empty.");
    if (matching_ && total_ != targetSize_) {
      matching_ = false;
      changed_ = true;
      existing_.close();
      if (!openStage() || !copyPrefix(total_)) return false;
    }
    if (existing_) existing_.close();
    if (stage_) stage_.close();
    return true;
  }

  bool commit() {
    if (!good_) return false;
    if (!changed_) return true;
    if (stage_) stage_.close();
    storage_->remove(backup_);
    bool hadTarget = storage_->exists(target_);
    if (hadTarget && !storage_->rename(target_, backup_)) {
      return fail("Could not stage existing file.");
    }
    if (!storage_->rename(temporary_, target_)) {
      if (hadTarget) storage_->rename(backup_, target_);
      return fail("Could not install staged file.");
    }
    storage_->remove(backup_);
    return true;
  }

  void abort() {
    if (existing_) existing_.close();
    if (stage_) stage_.close();
    if (storage_ && temporary_.length()) storage_->remove(temporary_);
    good_ = false;
  }

  bool good() const { return good_; }
  bool changed() const { return changed_; }
  uint32_t size() const { return total_; }
  const String& error() const { return error_; }
  String inputPath() const { return changed_ ? temporary_ : target_; }

  static void recover(fs::FS& storage, const String& target,
                      const String& temporary) {
    String backup = target + ".bak";
    storage.remove(temporary);
    if (!storage.exists(target) && storage.exists(backup)) {
      storage.rename(backup, target);
    } else if (storage.exists(target)) {
      storage.remove(backup);
    }
  }

 private:
  fs::FS* storage_ = nullptr;
  File existing_;
  File stage_;
  String target_;
  String temporary_;
  String backup_;
  String error_;
  uint32_t limit_ = 0;
  uint32_t total_ = 0;
  uint32_t targetSize_ = 0;
  uint32_t freeBytes_ = 0;
  bool matching_ = false;
  bool changed_ = false;
  bool good_ = false;

  bool openStage() {
    if (!freeBytes_) return fail("Storage is full.");
    stage_ = storage_->open(temporary_, "w");
    if (!stage_) return fail("Could not create staged file.");
    return true;
  }

  bool copyPrefix(uint32_t length) {
    File source = storage_->open(target_, "r");
    if (!source) return fail("Could not compare existing file.");
    uint8_t buffer[256];
    uint32_t copied = 0;
    while (copied < length) {
      size_t amount = min<uint32_t>(sizeof(buffer), length - copied);
      if (source.read(buffer, amount) != (int)amount || !reserve(amount) ||
          stage_.write(buffer, amount) != amount) {
        source.close();
        return fail("Could not stage unchanged prefix.");
      }
      copied += amount;
    }
    source.close();
    return true;
  }

  bool reserve(size_t amount) {
    if (amount > freeBytes_) return false;
    freeBytes_ -= amount;
    return true;
  }

  bool fail(const String& message) {
    error_ = message;
    good_ = false;
    if (existing_) existing_.close();
    if (stage_) stage_.close();
    if (storage_ && temporary_.length()) storage_->remove(temporary_);
    return false;
  }
};

