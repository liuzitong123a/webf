/*
 * Copyright (C) 2021 Alibaba Inc. All rights reserved.
 * Author: Kraken Team.
 */

#ifndef KRAKENBRIDGE_MODULE_MANAGER_H
#define KRAKENBRIDGE_MODULE_MANAGER_H

#include "executing_context.h"

namespace kraken {

struct ModuleContext {
  JSValue callback;
  ExecutionContext* context;
  list_head link;
};

void bindModuleManager(ExecutionContext* context);
void handleInvokeModuleUnexpectedCallback(void* callbackContext, int32_t contextId, NativeString* errmsg, NativeString* json);
}  // namespace kraken

#endif  // KRAKENBRIDGE_MODULE_MANAGER_H
