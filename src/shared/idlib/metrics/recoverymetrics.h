#pragma once

#include "shared/idlib/filesystem/file.h"
#include "shared/idlib/networking/amqp/mq.h"
#include "shared/idlib/networking/protocols/protocols.h"

#include <atomic>
#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <vector>

class idEventRecorderLight {
public:
  struct event_t {
    std::uint64_t start{}, end{};
    idStr name, category;
  };
  void StartCapture();
  void StopCapture();
  void Clear();
  std::size_t StartEventCopyStringInternal(const char* name, const char* category = "default");
  void StopEvent(std::size_t index);
  bool WriteReport(idFile* file) const;
  bool IsCapturing() const { return capturing; }
  const std::vector<event_t>& Events() const { return events; }

private:
  bool capturing{};
  std::uint64_t captureStart{};
  mutable std::mutex mutex;
  std::vector<event_t> events;
};

class idForgeMetricsClient {
public:
  struct metricEvent_t {
    std::string route, payload, message, payloadType;
    std::uint64_t timestamp{};
  };
  idForgeMetricsClient();
  ~idForgeMetricsClient();
  bool StartMessageSystem(const amqpEndpoint_t& endpoint = {});
  void StopMessageSystem();
  void PreRun();
  void OnThreadTerminate();
  void SetHeaderInfo(const char* runtimeSession, const char* match, const char* state);
  bool ReportMetric(const char* route, const idProtoMessage&);
  void ReportMetric(const idProtoMessage* metric, const char* message,
                    const char* metricType, const char* routingKey);
  bool ReportMetric(const char* route, const std::string&);
  int ThreadSlice();
  std::size_t Pending() const;
  const idmetrics::SessionHeader& Header() const { return header; }

private:
  mutable std::mutex mutex;
  std::deque<metricEvent_t> queue;
  idmetrics::SessionHeader header;
  idMQSessionManager sessions;
  std::shared_ptr<idMQSession> session;
  bool running{};
};
idForgeMetricsClient* GetForgeMetricsClient();

class idPLog {
public:
  struct entry_t {
    idStr name;
    std::uint64_t start{}, ticks{};
    int depth{};
  };
  idPLog();
  ~idPLog();
  void Clear();
  void Free();
  void EnsureEntries(std::size_t);
  std::size_t Begin(const char* name, int depth = 0);
  void End(std::size_t);
  std::uint64_t GetOverHeadTicks() const;
  idStr ShowUniqueEntries() const;
  idStr BuildLogData() const;
  idStr ShowCallGraph() const;
  void Enable(std::uint64_t mask = ~0ull) {
    enabled = true;
    groupMask = mask;
  }
  void Disable() { enabled = false; }

private:
  bool enabled{true};
  std::uint64_t groupMask{~0ull};
  std::vector<entry_t> entries;
};

class idPSyncLog {
public:
  struct sync_t {
    std::uint64_t time;
    std::uint32_t thread;
    idStr label;
  };
  idPSyncLog();
  ~idPSyncLog();
  void Add(const char*);
  void Clear();
  std::vector<sync_t> Snapshot() const;

private:
  mutable std::mutex mutex;
  std::vector<sync_t> events;
};
