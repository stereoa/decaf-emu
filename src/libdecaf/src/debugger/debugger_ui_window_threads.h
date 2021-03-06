#pragma once
#include "debugger_ui_window.h"
#include "modules/coreinit/coreinit_thread.h"

#include <chrono>

namespace debugger
{

namespace ui
{

struct ThreadInfo
{
   coreinit::OSThread *thread;
   uint32_t id;
   std::string name;
   coreinit::OSThreadState state;
   int32_t coreId;
   uint64_t coreTimeNs;
   int32_t priority;
   int32_t basePriority;
   uint32_t affinity;
};

class ThreadsWindow : public Window
{
public:
   ThreadsWindow(const std::string &name);
   virtual ~ThreadsWindow() = default;

   virtual void
   draw();

   void
   update();

private:
   std::vector<ThreadInfo> mThreadsCache;
};

} // namespace ui

} // namespace debugger
