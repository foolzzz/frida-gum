/*
 * Copyright (C) 2015-2021 Ole André Vadla Ravnås <oleavr@nowsecure.com>
 *
 * Licence: wxWindows Library Licence, Version 3.1
 */

#ifndef __GUM_V8_PLATFORM_H__
#define __GUM_V8_PLATFORM_H__

#include "gumv8bundle.h"
#include "gumscriptscheduler.h"

#include <functional>
#include <map>
#include <unordered_set>
#include <v8/v8-platform.h>

class GumV8Operation;
class GumV8MainContextOperation;
class GumV8ThreadPoolOperation;
class GumV8DelayedThreadPoolOperation;
class GumV8PlatformLocker;
class GumV8PlatformUnlocker;

class GumV8Platform : public v8::Platform
{
public:
  GumV8Platform ();
  GumV8Platform (const GumV8Platform &) = delete;
  GumV8Platform & operator= (const GumV8Platform &) = delete;
  ~GumV8Platform ();

  v8::Isolate * GetIsolate () const { return shared_isolate; }
  GumV8Bundle * GetRuntimeBundle () const { return runtime_bundle; }
  const gchar * GetRuntimeSourceMap () const;
#ifdef HAVE_OBJC_BRIDGE
  GumV8Bundle * GetObjCBundle ();
  const gchar * GetObjCSourceMap () const;
#endif
#ifdef HAVE_SWIFT_BRIDGE
  GumV8Bundle * GetSwiftBundle ();
  const gchar * GetSwiftSourceMap () const;
#endif
#ifdef HAVE_JAVA_BRIDGE
  GumV8Bundle * GetJavaBundle ();
  const gchar * GetJavaSourceMap () const;
#endif
  GumScriptScheduler * GetScheduler () const { return scheduler; }
  std::shared_ptr<GumV8Operation> ScheduleOnJSThread (std::function<void ()> f);
  std::shared_ptr<GumV8Operation> ScheduleOnJSThread (gint priority,
      std::function<void ()> f);
  std::shared_ptr<GumV8Operation> ScheduleOnJSThreadDelayed (
      guint delay_in_milliseconds, std::function<void ()> f);
  std::shared_ptr<GumV8Operation> ScheduleOnJSThreadDelayed (
      guint delay_in_milliseconds, gint priority, std::function<void ()> f);
  void PerformOnJSThread (std::function<void ()> f);
  void PerformOnJSThread (gint priority, std::function<void ()> f);
  std::shared_ptr<GumV8Operation> ScheduleOnThreadPool (
      std::function<void ()> f);
  std::shared_ptr<GumV8Operation> ScheduleOnThreadPoolDelayed (
      guint delay_in_milliseconds, std::function<void ()> f);

  v8::PageAllocator * GetPageAllocator () override;
  int NumberOfWorkerThreads () override;
  std::shared_ptr<v8::TaskRunner> GetForegroundTaskRunner (
      v8::Isolate * isolate) override;
  void CallOnWorkerThread (std::unique_ptr<v8::Task> task) override;
  void CallDelayedOnWorkerThread (std::unique_ptr<v8::Task> task,
      double delay_in_seconds) override;
  bool IdleTasksEnabled (v8::Isolate * isolate) override;
  std::unique_ptr<v8::JobHandle> PostJob (v8::TaskPriority priority,
      std::unique_ptr<v8::JobTask> job_task) override;
  double MonotonicallyIncreasingTime () override;
  double CurrentClockTimeMillis () override;
  v8::ThreadingBackend * GetThreadingBackend () override;
  v8::TracingController * GetTracingController () override;

private:
  void InitRuntime ();
  void Dispose ();
  void CancelPendingOperations ();
  static void OnFatalError (const char * location, const char * message);

  static gboolean PerformMainContextOperation (gpointer data);
  static void ReleaseMainContextOperation (gpointer data);
  static void ReleaseSynchronousMainContextOperation (gpointer data);
  static void PerformThreadPoolOperation (gpointer data);
  static void ReleaseThreadPoolOperation (gpointer data);
  static gboolean StartDelayedThreadPoolOperation (gpointer data);
  static void PerformDelayedThreadPoolOperation (gpointer data);
  static void ReleaseDelayedThreadPoolOperation (gpointer data);

  GMutex mutex;
  v8::Isolate * shared_isolate;
  GumV8Bundle * runtime_bundle;
#ifdef HAVE_OBJC_BRIDGE
  GumV8Bundle * objc_bundle;
#endif
#ifdef HAVE_SWIFT_BRIDGE
  GumV8Bundle * swift_bundle;
#endif
#ifdef HAVE_JAVA_BRIDGE
  GumV8Bundle * java_bundle;
#endif
  GumScriptScheduler * scheduler;
  std::unordered_set<std::shared_ptr<GumV8Operation>> js_ops;
  std::unordered_set<std::shared_ptr<GumV8Operation>> pool_ops;
  std::map<v8::Isolate *, std::shared_ptr<v8::TaskRunner>> foreground_runners;
  std::unique_ptr<v8::PageAllocator> page_allocator;
  std::unique_ptr<v8::ArrayBuffer::Allocator> array_buffer_allocator;
  std::unique_ptr<v8::ThreadingBackend> threading_backend;
  std::unique_ptr<v8::TracingController> tracing_controller;

  friend class GumV8MainContextOperation;
  friend class GumV8ThreadPoolOperation;
  friend class GumV8DelayedThreadPoolOperation;
  friend class GumV8PlatformLocker;
  friend class GumV8PlatformUnlocker;
};

class GumV8Operation
{
public:
  GumV8Operation () = default;
  GumV8Operation (const GumV8Operation &) = delete;
  GumV8Operation & operator= (const GumV8Operation &) = delete;
  virtual ~GumV8Operation () = default;

  virtual void Cancel () = 0;
  virtual void Await () = 0;
};

#endif
