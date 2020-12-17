/*
 * Copyright (C) 2019 Alibaba Inc. All rights reserved.
 * Author: Kraken Team.
 */

#include "bindings/jsc/host_object.h"
#include "bindings/jsc/js_context.h"
#include "bindings/jsc/macros.h"
#include <unordered_map>
#include <vector>

namespace kraken::binding::jsc {

#define JSPerformanceName "Performance"

void bindPerformance(std::unique_ptr<JSContext> &context);

struct NativePerformanceEntry {
  NativePerformanceEntry(const std::string &name, const std::string &entryType, double startTime, double duration)
    : startTime(startTime), duration(duration) {
    this->name = new char[name.size() + 1];
    this->entryType = new char[entryType.size() + 1];
    strcpy(this->name, name.data());
    strcpy(this->entryType, entryType.data());
  };
  char *name;
  char *entryType;
  double startTime;
  double duration;
};

class JSPerformance;

class JSPerformanceEntry : public HostObject {
public:
  DEFINE_OBJECT_PROPERTY(PerformanceEntry, 4, name, entryType, startTime, duration)

  JSPerformanceEntry() = delete;
  explicit JSPerformanceEntry(JSContext *context, NativePerformanceEntry *nativePerformanceEntry);

  JSValueRef getProperty(std::string &name, JSValueRef *exception) override;
  void getPropertyNames(JSPropertyNameAccumulatorRef accumulator) override;

private:
  friend JSPerformance;
  NativePerformanceEntry *m_nativePerformanceEntry;
};

class JSPerformanceMark : public JSPerformanceEntry {
public:
  JSPerformanceMark() = delete;
  explicit JSPerformanceMark(JSContext *context, std::string &name, double startTime);
  explicit JSPerformanceMark(JSContext *context, NativePerformanceEntry *nativePerformanceEntry);

private:
};

class JSPerformanceMeasure : public JSPerformanceEntry {
public:
  JSPerformanceMeasure() = delete;
  explicit JSPerformanceMeasure(JSContext *context, std::string &name, double startTime, double duration);
  explicit JSPerformanceMeasure(JSContext *context, NativePerformanceEntry *nativePerformanceEntry);
};

class NativePerformance {
public:
  static std::unordered_map<JSContext *, NativePerformance *> instanceMap;
  static NativePerformance *instance(JSContext *context);
  static void disposeInstance(JSContext *context);

  void mark(const std::string &markName);
  void mark(const std::string &markName, double startTime);
  std::vector<NativePerformanceEntry *> entries;
};

class JSPerformance : public HostObject {
public:
  DEFINE_OBJECT_PROPERTY(Performance, 11, now, timeOrigin, toJSON, clearMarks, clearMeasures, getEntries,
                         getEntriesByName, getEntriesByType, mark, measure, summary)

  static JSValueRef now(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount,
                        const JSValueRef arguments[], JSValueRef *exception);

  static JSValueRef timeOrigin(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount,
                               const JSValueRef arguments[], JSValueRef *exception);

  static JSValueRef toJSON(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount,
                           const JSValueRef arguments[], JSValueRef *exception);
  static JSValueRef clearMarks(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount,
                               const JSValueRef arguments[], JSValueRef *exception);

  static JSValueRef clearMeasures(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount,
                                  const JSValueRef arguments[], JSValueRef *exception);

  static JSValueRef getEntries(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount,
                               const JSValueRef arguments[], JSValueRef *exception);

  static JSValueRef getEntriesByName(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
                                     size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception);

  static JSValueRef getEntriesByType(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
                                     size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception);

  static JSValueRef mark(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount,
                         const JSValueRef arguments[], JSValueRef *exception);

  static JSValueRef measure(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount,
                            const JSValueRef arguments[], JSValueRef *exception);

#if ENABLE_PROFILE
  static JSValueRef summary(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount,
                            const JSValueRef arguments[], JSValueRef *exception);
#endif

  JSPerformance(JSContext *context, NativePerformance *nativePerformance)
    : HostObject(context, JSPerformanceName), nativePerformance(nativePerformance) {}
  ~JSPerformance() override;
  JSValueRef getProperty(std::string &name, JSValueRef *exception) override;
  void getPropertyNames(JSPropertyNameAccumulatorRef accumulator) override;

private:
  friend JSPerformanceEntry;
  JSFunctionHolder m_now{context, "now", now};
  JSFunctionHolder m_toJSON{context, "toJSON", toJSON};
  JSFunctionHolder m_clearMarks{context, "clearMarks", clearMarks};
  JSFunctionHolder m_clearMeasures{context, "clearMeasures", clearMeasures};
  JSFunctionHolder m_getEntries{context, "getEntries", getEntries};
  JSFunctionHolder m_getEntriesByName{context, "getEntriesByName", getEntriesByName};
  JSFunctionHolder m_getEntriesByType{context, "getEntriesByType", getEntriesByType};
  JSFunctionHolder m_mark{context, "mark", mark};
  JSFunctionHolder m_measure{context, "measure", measure};

#if ENABLE_PROFILE
  JSFunctionHolder m_summary{context, "summary", summary};
  void measureSummary(JSValueRef *exception);
#endif
  void internalMeasure(const std::string &name, const std::string &startMark, const std::string &endMark,
                       JSValueRef *exception);
  double internalNow();
  std::vector<NativePerformanceEntry*> getFullEntries();
  NativePerformance *nativePerformance{nullptr};
};

} // namespace kraken::binding::jsc
