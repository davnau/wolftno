#include "shared/idlib/metrics/recoverymetrics.h"
#include "shared/idlib/sys/sys_time.h"
void idEventRecorderLight::StartCapture() {
  std::lock_guard<std::mutex> lock(mutex);
  events.clear();
  captureStart = static_cast<std::uint64_t>(Sys_Microseconds());
  capturing = true;
}
void idEventRecorderLight::StopCapture() {
  capturing = false;
}
void idEventRecorderLight::Clear() {
  std::lock_guard<std::mutex> lock(mutex);
  events.clear();
}
std::size_t idEventRecorderLight::StartEventCopyStringInternal(const char* n, const char* c) {
  std::lock_guard<std::mutex> lock(mutex);
  if (!capturing)
    return static_cast<std::size_t>(-1);
  events.push_back(
      {static_cast<std::uint64_t>(Sys_Microseconds()), 0, idStr(n ? n : ""), idStr(c ? c : "")});
  return events.size() - 1;
}
void idEventRecorderLight::StopEvent(std::size_t i) {
  std::lock_guard<std::mutex> lock(mutex);
  if (i < events.size())
    events[i].end = static_cast<std::uint64_t>(Sys_Microseconds());
}
bool idEventRecorderLight::WriteReport(idFile* f) const {
  if (!f)
    return false;
  std::lock_guard<std::mutex> lock(mutex);
  f->Printf("{\"captureStart\":%llu,\"events\":[", static_cast<unsigned long long>(captureStart));
  for (std::size_t i = 0; i < events.size(); ++i) {
    if (i)
      f->Printf(",");
    const auto& e = events[i];
    f->Printf("{\"name\":\"%s\",\"category\":\"%s\",\"start\":%llu,\"end\":%llu}", e.name.data,
              e.category.data, static_cast<unsigned long long>(e.start),
              static_cast<unsigned long long>(e.end));
  }
  f->Printf("]}");
  return true;
}
