/*
 * Copyright (C) 2020 Alibaba Inc. All rights reserved.
 * Author: Kraken Team.
 */

#ifndef KRAKENBRIDGE_FOUNDATION_UI_COMMAND_BUFFER_H_
#define KRAKENBRIDGE_FOUNDATION_UI_COMMAND_BUFFER_H_

#include <cinttypes>
#include <vector>
#include "bindings/qjs/native_string_utils.h"
#include "native_value.h"

namespace kraken {

class ExecutingContext;

enum class UICommand {
  createElement,
  createTextNode,
  createComment,
  disposeEventTarget,
  addEvent,
  removeNode,
  insertAdjacentNode,
  setStyle,
  setAttribute,
  removeAttribute,
  cloneNode,
  removeEvent,
  createDocumentFragment,
};

struct UICommandItem {
  UICommandItem(int32_t id, int32_t type, const NativeString* args_01, const NativeString* args_02, void* nativePtr)
      : type(type),
        string_01(reinterpret_cast<int64_t>(new NativeString(args_01))),
        args_01_length(args_01->length()),
        string_02(reinterpret_cast<int64_t>(new NativeString(args_02))),
        args_02_length(args_02->length()),
        id(id),
        nativePtr(reinterpret_cast<int64_t>(nativePtr)){};
  UICommandItem(int32_t id, int32_t type, const NativeString* args_01, void* nativePtr)
      : type(type),
        string_01(reinterpret_cast<int64_t>(new NativeString(args_01))),
        args_01_length(args_01->length()),
        id(id),
        nativePtr(reinterpret_cast<int64_t>(nativePtr)){};
  UICommandItem(int32_t id, int32_t type, void* nativePtr)
      : type(type), id(id), nativePtr(reinterpret_cast<int64_t>(nativePtr)){};
  int32_t type;
  int32_t id;
  int32_t args_01_length{0};
  int32_t args_02_length{0};
  int64_t string_01{0};
  int64_t string_02{0};
  int64_t nativePtr{0};
};

class UICommandBuffer {
 public:
  UICommandBuffer() = delete;
  explicit UICommandBuffer(ExecutingContext* context);
  void addCommand(int32_t id, int32_t type, void* nativePtr, bool batchedUpdate);
  void addCommand(int32_t id, int32_t type, void* nativePtr);
  void addCommand(int32_t id, int32_t type, const std::unique_ptr<NativeString>& args_01, const std::unique_ptr<NativeString>& args_02, void* nativePtr);
  void addCommand(int32_t id, int32_t type, const std::unique_ptr<NativeString>& args_01, void* nativePtr);
  UICommandItem* data();
  int64_t size();
  void clear();

 private:
  ExecutingContext* m_context{nullptr};
  std::atomic<bool> update_batched{false};
  std::vector<UICommandItem> queue;
};

}  // namespace kraken

#endif  // KRAKENBRIDGE_FOUNDATION_UI_COMMAND_BUFFER_H_
