/*
 * Copyright (C) 2019 Alibaba Inc. All rights reserved.
 * Author: Kraken Team.
 */

#include "performance.h"
#include "dart_methods.h"
#include <chrono>
#include <cmath>

namespace kraken::binding::jsc {

using namespace std::chrono;

#if ENABLE_PROFILE
#define PERF_WIDGET_CREATION_COST "widget_creation_cost"
#define PERF_CONTROLLER_PROPERTIES_INIT_COST "kraken_controller_properties_init_cost"
#define PERF_VIEW_CONTROLLER_PROPERTIES_INIT_COST "kraken_view_controller_properties_init_cost"
#define PERF_BRIDGE_INIT_COST "kraken_bridge_init_cost"
#define PERF_BRIDGE_REGISTER_DART_METHOD_COST "kraken_bridge_register_dart_method_cost"
#define PERF_CREATE_VIEWPORT_COST "kraken_create_viewport"
#define PERF_ELEMENT_MANAGER_INIT_COST "kraken_element_manager_init_cost"
#define PERF_ELEMENT_MANAGER_PROPERTIES_INIT_COST "kraken_element_manager_property_init_cost"
#define PERF_BODY_ELEMENT_INIT_COST "kraken_body_element_init_cost"
#define PERF_BODY_ELEMENT_PROPERTIES_INIT_COST "kraken_body_element_property_init_cost"
#define PERF_JS_CONTEXT_INIT_COST "js_context_init_cost"
#define PERF_JS_NATIVE_METHOD_INIT_COST "native_method_init_cost"
#define PERF_JS_POLYFILL_INIT_COST "polyfill_init_cost"

#define PERF_CONTROLLER_INIT_START "kraken_controller_init_start"
#define PERF_CONTROLLER_INIT_END "kraken_controller_init_end"
#define PERF_CONTROLLER_PROPERTY_INIT "kraken_controller_properties_init"
#define PERF_VIEW_CONTROLLER_INIT_START "kraken_view_controller_init_start"
#define PERF_VIEW_CONTROLLER_PROPERTY_INIT "kraken_view_controller_property_init"
#define PERF_BRIDGE_INIT_START "kraken_bridge_init_start"
#define PERF_BRIDGE_INIT_END "kraken_bridge_init_end"
#define PERF_BRIDGE_REGISTER_DART_METHOD_START "kraken_bridge_register_dart_method_start"
#define PERF_BRIDGE_REGISTER_DART_METHOD_END "kraken_bridge_register_dart_method_end"
#define PERF_CREATE_VIEWPORT_START "kraken_create_viewport_start"
#define PERF_CREATE_VIEWPORT_END "kraken_create_viewport_end"
#define PERF_ELEMENT_MANAGER_INIT_START "kraken_element_manager_init_start"
#define PERF_ELEMENT_MANAGER_INIT_END "kraken_element_manager_init_end"
#define PERF_ELEMENT_MANAGER_PROPERTY_INIT "kraken_element_manager_property_init"
#define PERF_BODY_ELEMENT_INIT_START "kraken_body_element_init_start"
#define PERF_BODY_ELEMENT_INIT_END "kraken_body_element_init_end"
#define PERF_BODY_ELEMENT_PROPERTY_INIT "kraken_body_element_property_init"
#define PERF_JS_CONTEXT_INIT_START "js_context_start"
#define PERF_JS_CONTEXT_INIT_END "js_context_end"
#define PERF_JS_NATIVE_METHOD_INIT_START "init_native_method_start"
#define PERF_JS_NATIVE_METHOD_INIT_END "init_native_method_end"
#define PERF_JS_POLYFILL_INIT_START "init_js_polyfill_start"
#define PERF_JS_POLYFILL_INIT_END "init_js_polyfill_end"
#endif

std::unordered_map<JSContext *, NativePerformance *> NativePerformance::instanceMap{};
NativePerformance *NativePerformance::instance(JSContext *context) {
  if (instanceMap.count(context) == 0) {
    instanceMap[context] = new NativePerformance();
  }

  return instanceMap[context];
}

void NativePerformance::disposeInstance(JSContext *context) {
  if (instanceMap.count(context) > 0) delete instanceMap[context];
}

void NativePerformance::mark(const std::string &markName) {
  double startTime = std::chrono::duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  auto *nativePerformanceEntry = new NativePerformanceEntry{markName, "mark", startTime, 0};
  entries.emplace_back(nativePerformanceEntry);
}

void NativePerformance::mark(const std::string &markName, double startTime) {
  auto *nativePerformanceEntry = new NativePerformanceEntry{markName, "mark", startTime, 0};
  entries.emplace_back(nativePerformanceEntry);
}

JSObjectRef buildPerformanceEntry(std::string &entryType, JSContext *context,
                                  NativePerformanceEntry *nativePerformanceEntry) {
  if (entryType == "mark") {
    auto *mark = new JSPerformanceMark(context, nativePerformanceEntry);
    return mark->jsObject;
  } else if (entryType == "measure") {
    auto *measure = new JSPerformanceMeasure(context, nativePerformanceEntry);
    return measure->jsObject;
  }

  return nullptr;
}

JSPerformanceEntry::JSPerformanceEntry(JSContext *context, NativePerformanceEntry *nativePerformanceEntry)
  : HostObject(context, "PerformanceEntry"), m_nativePerformanceEntry(nativePerformanceEntry) {}

JSValueRef JSPerformanceEntry::getProperty(std::string &name, JSValueRef *exception) {
  auto propertyMap = getPerformanceEntryPropertyMap();
  if (propertyMap.count(name) > 0) {
    auto property = propertyMap[name];
    switch (property) {
    case PerformanceEntryProperty::name: {
      JSStringRef nameValue = JSStringCreateWithUTF8CString(m_nativePerformanceEntry->name);
      return JSValueMakeString(ctx, nameValue);
    }
    case PerformanceEntryProperty::entryType: {
      JSStringRef entryValue = JSStringCreateWithUTF8CString(m_nativePerformanceEntry->entryType);
      return JSValueMakeString(ctx, entryValue);
    }
    case PerformanceEntryProperty::startTime:
      return JSValueMakeNumber(ctx, m_nativePerformanceEntry->startTime);
    case PerformanceEntryProperty::duration:
      return JSValueMakeNumber(ctx, m_nativePerformanceEntry->duration);
    }
  }
  return nullptr;
}

JSPerformanceMark::JSPerformanceMark(JSContext *context, std::string &name, double startTime)
  : JSPerformanceEntry(context, new NativePerformanceEntry(name, "mark", startTime, 0)) {}
JSPerformanceMark::JSPerformanceMark(JSContext *context, NativePerformanceEntry *nativePerformanceEntry)
  : JSPerformanceEntry(context, nativePerformanceEntry) {}

JSPerformanceMeasure::JSPerformanceMeasure(JSContext *context, std::string &name, double startTime, double duration)
  : JSPerformanceEntry(context, new NativePerformanceEntry(name, "measure", startTime, duration)) {}
JSPerformanceMeasure::JSPerformanceMeasure(JSContext *context, NativePerformanceEntry *nativePerformanceEntry)
  : JSPerformanceEntry(context, nativePerformanceEntry) {}

JSValueRef JSPerformance::getProperty(std::string &name, JSValueRef *exception) {
  auto propertyMap = getPerformancePropertyMap();

  if (propertyMap.count(name) > 0) {
    auto property = propertyMap[name];

    switch (property) {
    case PerformanceProperty::now: {
      return m_now.function();
    }
    case PerformanceProperty::timeOrigin: {
      double time =
        std::chrono::duration_cast<std::chrono::milliseconds>(context->timeOrigin.time_since_epoch()).count();
      return JSValueMakeNumber(ctx, time);
    }
    case PerformanceProperty::toJSON: {
      return m_toJSON.function();
    }
    case PerformanceProperty::clearMarks:
      return m_clearMarks.function();
    case PerformanceProperty::clearMeasures:
      return m_clearMeasures.function();
    case PerformanceProperty::getEntries:
      return m_getEntries.function();
    case PerformanceProperty::getEntriesByName:
      return m_getEntriesByName.function();
    case PerformanceProperty::getEntriesByType:
      return m_getEntriesByType.function();
    case PerformanceProperty::mark:
      return m_mark.function();
    case PerformanceProperty::measure:
      return m_measure.function();
#if ENABLE_PROFILE
    case PerformanceProperty::summary:
      return m_summary.function();
#endif
    }
  }

  return HostObject::getProperty(name, exception);
}

void JSPerformanceEntry::getPropertyNames(JSPropertyNameAccumulatorRef accumulator) {
  for (auto &property : getPerformanceEntryPropertyNames()) {
    JSPropertyNameAccumulatorAddName(accumulator, property);
  }
}

JSPerformance::~JSPerformance() {}

void JSPerformance::getPropertyNames(JSPropertyNameAccumulatorRef accumulator) {
  for (auto &property : getPerformancePropertyNames()) {
    JSPropertyNameAccumulatorAddName(accumulator, property);
  }
}

double JSPerformance::internalNow() {
  auto now = std::chrono::system_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - context->timeOrigin);
  auto reducedDuration = std::floor(duration / 1000us) * 1000us;
  return std::chrono::duration_cast<std::chrono::milliseconds>(reducedDuration).count();
}

JSValueRef JSPerformance::now(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount,
                              const JSValueRef *arguments, JSValueRef *exception) {
  auto instance = reinterpret_cast<JSPerformance *>(JSObjectGetPrivate(thisObject));
  double now = instance->internalNow();
  return JSValueMakeNumber(ctx, now);
}

JSValueRef JSPerformance::timeOrigin(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
                                     size_t argumentCount, JSValueRef const *arguments, JSValueRef *exception) {
  auto instance = reinterpret_cast<JSPerformance *>(JSObjectGetPrivate(thisObject));
  double time =
    std::chrono::duration_cast<std::chrono::milliseconds>(instance->context->timeOrigin.time_since_epoch()).count();
  return JSValueMakeNumber(ctx, time);
}

JSValueRef JSPerformance::toJSON(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount,
                                 const JSValueRef *arguments, JSValueRef *exception) {
  auto instance = reinterpret_cast<JSPerformance *>(JSObjectGetPrivate(thisObject));
  double now = instance->internalNow();
  double timeOrigin =
    std::chrono::duration_cast<std::chrono::milliseconds>(instance->context->timeOrigin.time_since_epoch()).count();

  auto context = instance->context;
  auto object = JSObjectMake(ctx, nullptr, exception);
  JSC_SET_STRING_PROPERTY(context, object, "now", JSValueMakeNumber(ctx, now));
  JSC_SET_STRING_PROPERTY(context, object, "timeOrigin", JSValueMakeNumber(ctx, timeOrigin));
  return object;
}

JSValueRef JSPerformance::clearMarks(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
                                     size_t argumentCount, const JSValueRef *arguments, JSValueRef *exception) {
  JSValueRef targetMark = nullptr;
  if (argumentCount == 1) {
    targetMark = arguments[0];
  }

  auto performance = reinterpret_cast<JSPerformance *>(JSObjectGetPrivate(thisObject));
  auto entries = performance->nativePerformance->entries;
  auto it = std::begin(entries);

  while (it != entries.end()) {
    std::string entryType = (*it)->entryType;
    if (entryType == "mark") {
      if (targetMark == nullptr) {
        entries.erase(it);
      } else {
        std::string entryName = (*it)->name;
        std::string targetName = JSStringToStdString(JSValueToStringCopy(ctx, targetMark, exception));
        if (entryName == targetName) {
          entries.erase(it);
        } else {
          it++;
        };
      }
    } else {
      it++;
    }
  }

  return nullptr;
}

JSValueRef JSPerformance::clearMeasures(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
                                        size_t argumentCount, const JSValueRef *arguments, JSValueRef *exception) {
  JSValueRef targetMark = nullptr;
  if (argumentCount == 1) {
    targetMark = arguments[0];
  }

  auto performance = reinterpret_cast<JSPerformance *>(JSObjectGetPrivate(thisObject));
  auto entries = performance->nativePerformance->entries;
  auto it = std::begin(entries);

  while (it != entries.end()) {
    std::string entryType = (*it)->entryType;
    if (entryType == "measure") {
      if (targetMark == nullptr) {
        entries.erase(it);
      } else {
        std::string entryName = (*it)->name;
        std::string targetName = JSStringToStdString(JSValueToStringCopy(ctx, targetMark, exception));
        if (entryName == targetName) {
          entries.erase(it);
        } else {
          it++;
        }
      }
    } else {
      it++;
    }
  }

  return nullptr;
}

JSValueRef JSPerformance::getEntries(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
                                     size_t argumentCount, const JSValueRef *arguments, JSValueRef *exception) {
  auto performance = reinterpret_cast<JSPerformance *>(JSObjectGetPrivate(thisObject));
  auto entries = performance->getFullEntries();

  size_t entriesSize = entries.size();
  JSValueRef args[entriesSize];

  for (size_t i = 0; i < entriesSize; i++) {
    auto &entry = entries[i];
    auto entryType = std::string(entry->entryType);
    args[i] = buildPerformanceEntry(entryType, performance->context, entry);
  }

  JSObjectRef entriesArray = JSObjectMakeArray(ctx, entriesSize, args, exception);
  return entriesArray;
}

JSValueRef JSPerformance::getEntriesByName(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
                                           size_t argumentCount, const JSValueRef *arguments, JSValueRef *exception) {
  if (argumentCount == 0) {
    JSC_THROW_ERROR(ctx,
                    "Failed to execute 'getEntriesByName' on 'Performance': 1 argument required, but only 0 present.",
                    exception);
    return nullptr;
  }

  JSStringRef targetNameStrRef = JSValueToStringCopy(ctx, arguments[0], exception);
  std::string targetName = JSStringToStdString(targetNameStrRef);

  auto performance = reinterpret_cast<JSPerformance *>(JSObjectGetPrivate(thisObject));
  std::vector<JSObjectRef> targetEntries;
  auto entries = performance->getFullEntries();

  for (auto &m_entries : entries) {
    if (m_entries->name == targetName) {
      std::string entryType = std::string(m_entries->entryType);
      auto performanceEntry = buildPerformanceEntry(entryType, performance->context, m_entries);
      targetEntries.emplace_back(performanceEntry);
    }
  }

  return JSObjectMakeArray(ctx, targetEntries.size(), targetEntries.data(), exception);
}

JSValueRef JSPerformance::getEntriesByType(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
                                           size_t argumentCount, const JSValueRef *arguments, JSValueRef *exception) {
  if (argumentCount == 0) {
    JSC_THROW_ERROR(ctx,
                    "Failed to execute 'getEntriesByName' on 'Performance': 1 argument required, but only 0 present.",
                    exception);
    return nullptr;
  }

  JSStringRef entryTypeStrRef = JSValueToStringCopy(ctx, arguments[0], exception);
  std::string entryType = JSStringToStdString(entryTypeStrRef);

  auto performance = reinterpret_cast<JSPerformance *>(JSObjectGetPrivate(thisObject));
  std::vector<JSObjectRef> targetEntries;
  auto entries = performance->getFullEntries();

  for (auto &m_entries : entries) {
    if (m_entries->entryType == entryType) {
      auto performanceEntry = buildPerformanceEntry(entryType, performance->context, m_entries);
      targetEntries.emplace_back(performanceEntry);
    }
  }

  return JSObjectMakeArray(ctx, targetEntries.size(), targetEntries.data(), exception);
}

JSValueRef JSPerformance::mark(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount,
                               const JSValueRef *arguments, JSValueRef *exception) {
  if (argumentCount != 1) {
    JSC_THROW_ERROR(ctx, "Failed to execute 'mark' on 'Performance': 1 argument required, but only 0 present.",
                    exception);
    return nullptr;
  }

  auto performance = reinterpret_cast<JSPerformance *>(JSObjectGetPrivate(thisObject));
  JSStringRef markNameRef = JSValueToStringCopy(ctx, arguments[0], exception);
  std::string markName = JSStringToStdString(markNameRef);

  performance->nativePerformance->mark(markName);

  return nullptr;
}

#if ENABLE_PROFILE
JSValueRef JSPerformance::summary(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount,
                                  const JSValueRef *arguments, JSValueRef *exception) {
  auto performance = reinterpret_cast<JSPerformance *>(JSObjectGetPrivate(thisObject));
  performance->measureSummary(exception);
  return nullptr;
}

void JSPerformance::measureSummary(JSValueRef *exception) {
  internalMeasure(PERF_WIDGET_CREATION_COST, PERF_CONTROLLER_INIT_START, PERF_CONTROLLER_INIT_END, exception);
  internalMeasure(PERF_CONTROLLER_PROPERTIES_INIT_COST, PERF_CONTROLLER_INIT_START, PERF_CONTROLLER_PROPERTY_INIT, exception);
  internalMeasure(PERF_VIEW_CONTROLLER_PROPERTIES_INIT_COST, PERF_VIEW_CONTROLLER_INIT_START, PERF_VIEW_CONTROLLER_PROPERTY_INIT, exception);
  internalMeasure(PERF_BRIDGE_INIT_COST, PERF_BRIDGE_INIT_START, PERF_BRIDGE_INIT_END, exception);
  internalMeasure(PERF_BRIDGE_REGISTER_DART_METHOD_COST, PERF_BRIDGE_REGISTER_DART_METHOD_START, PERF_BRIDGE_REGISTER_DART_METHOD_END, exception);
  internalMeasure(PERF_CREATE_VIEWPORT_COST, PERF_CREATE_VIEWPORT_START, PERF_CREATE_VIEWPORT_END, exception);
  internalMeasure(PERF_ELEMENT_MANAGER_INIT_COST, PERF_ELEMENT_MANAGER_INIT_START, PERF_ELEMENT_MANAGER_INIT_END, exception);
  internalMeasure(PERF_ELEMENT_MANAGER_PROPERTIES_INIT_COST, PERF_ELEMENT_MANAGER_INIT_START, PERF_ELEMENT_MANAGER_PROPERTY_INIT, exception);
  internalMeasure(PERF_BODY_ELEMENT_INIT_COST, PERF_BODY_ELEMENT_INIT_START, PERF_BODY_ELEMENT_INIT_END, exception);
  internalMeasure(PERF_BODY_ELEMENT_PROPERTIES_INIT_COST, PERF_BODY_ELEMENT_INIT_START, PERF_BODY_ELEMENT_PROPERTY_INIT, exception);
  internalMeasure(PERF_JS_CONTEXT_INIT_COST, PERF_JS_CONTEXT_INIT_START, PERF_JS_CONTEXT_INIT_END, exception);
  internalMeasure(PERF_JS_NATIVE_METHOD_INIT_COST, PERF_JS_NATIVE_METHOD_INIT_START, PERF_JS_NATIVE_METHOD_INIT_END, exception);
  internalMeasure(PERF_JS_POLYFILL_INIT_COST, PERF_JS_POLYFILL_INIT_START, PERF_JS_POLYFILL_INIT_END, exception);
}

#endif

JSValueRef JSPerformance::measure(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount,
                                  const JSValueRef *arguments, JSValueRef *exception) {
  if (argumentCount == 0) {
    JSC_THROW_ERROR(ctx, "Failed to execute 'measure' on 'Performance': 1 argument required, but only 0 present.",
                    exception);
    return nullptr;
  }

  JSStringRef nameStrRef = JSValueToStringCopy(ctx, arguments[0], exception);
  std::string name = JSStringToStdString(nameStrRef);
  std::string startMark;
  std::string endMark;

  if (argumentCount > 1) {
    bool isStartMarkUndefined = JSValueIsUndefined(ctx, arguments[1]);
    if (!isStartMarkUndefined) {
      JSStringRef startMarkStringRef = JSValueToStringCopy(ctx, arguments[1], exception);
      startMark = JSStringToStdString(startMarkStringRef);
    }
  }

  if (argumentCount > 2) {
    JSStringRef endMarkStringRef = JSValueToStringCopy(ctx, arguments[2], exception);
    endMark = JSStringToStdString(endMarkStringRef);
  }

  auto performance = reinterpret_cast<JSPerformance *>(JSObjectGetPrivate(thisObject));
  performance->internalMeasure(name, startMark, endMark, exception);

  return nullptr;
}

std::vector<NativePerformanceEntry *> JSPerformance::getFullEntries() {
  auto bridgeEntries = nativePerformance->entries;
#if ENABLE_PROFILE
  auto dartEntryList = getDartMethod()->getPerformanceEntries(context->getContextId());
  auto dartEntryPtr = reinterpret_cast<NativePerformanceEntry **>(dartEntryList->entries);
  std::vector<NativePerformanceEntry *> dartEntries{dartEntryPtr, dartEntryPtr + dartEntryList->length};
#endif

  std::vector<NativePerformanceEntry *> mergedEntries;

  mergedEntries.insert(mergedEntries.begin(), bridgeEntries.begin(), bridgeEntries.end());
#if ENABLE_PROFILE
  mergedEntries.insert(mergedEntries.begin(), dartEntries.begin(), dartEntries.end());
  delete[] dartEntryPtr;
  delete dartEntryList;
  std::sort(mergedEntries.begin(), mergedEntries.end(),
            [](NativePerformanceEntry *left, NativePerformanceEntry *right) -> bool {
              return left->startTime < right->startTime;
            });
#endif

  return mergedEntries;
}

void JSPerformance::internalMeasure(const std::string &name, const std::string &startMark, const std::string &endMark,
                                    JSValueRef *exception) {
  auto entries = getFullEntries();

  double duration;
  auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

  if (!startMark.empty() && !endMark.empty()) {
    auto startEntry = std::find_if(entries.begin(), entries.end(),
                                   [&startMark](auto entry) -> bool { return startMark == entry->name; });
    auto endEntry =
      std::find_if(entries.begin(), entries.end(), [&endMark](auto entry) -> bool { return endMark == entry->name; });
    if (startEntry == entries.end()) {
      JSC_THROW_ERROR(
        ctx, ("Failed to execute 'measure' on 'Performance': The mark " + startMark + " does not exist.").c_str(),
        exception);
      return;
    }
    if (endEntry == entries.end()) {
      JSC_THROW_ERROR(
        ctx, ("Failed to execute 'measure' on 'Performance': The mark " + endMark + " does not exist.").c_str(),
        exception);
      return;
    }

    duration = (*endEntry)->startTime - (*startEntry)->startTime;
  } else if (!startMark.empty()) {
    auto startEntry = std::find_if(entries.begin(), entries.end(),
                                   [&startMark](auto entry) -> bool { return startMark == entry->name; });
    if (startEntry == entries.end()) {
      JSC_THROW_ERROR(
        ctx, ("Failed to execute 'measure' on 'Performance': The mark " + startMark + " does not exist.").c_str(),
        exception);
      return;
    }

    duration = now - (*startEntry)->startTime;
  } else if (!endMark.empty()) {
    auto endEntry =
      std::find_if(entries.begin(), entries.end(), [&endMark](auto entry) -> bool { return endMark == entry->name; });
    duration = (*endEntry)->startTime - duration_cast<milliseconds>(context->timeOrigin.time_since_epoch()).count();
  } else {
    duration = internalNow();
  }

  double startTime = std::chrono::duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  auto *nativePerformanceEntry = new NativePerformanceEntry{name, "measure", startTime, duration};
  nativePerformance->entries.emplace_back(nativePerformanceEntry);
}

void bindPerformance(std::unique_ptr<JSContext> &context) {
  auto performance = new JSPerformance(context.get(), NativePerformance::instance(context.get()));
  JSC_GLOBAL_BINDING_HOST_OBJECT(context, "performance", performance);
}
} // namespace kraken::binding::jsc
