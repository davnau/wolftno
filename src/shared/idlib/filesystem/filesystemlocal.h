#pragma once

#include "shared/idlib/filesystem/file.h"
#include "shared/idlib/containers/list.h"

#include <cstdint>
#include <memory>
#include <vector>

struct idFileList {
  idStr basePath;
  idList<idStr> list;
};

class idFileSystem {
public:
  virtual ~idFileSystem() = default;
  virtual void Init(const char* root) = 0;
  virtual void Shutdown() = 0;
  virtual void ClearZipCache() {}
  virtual idFile* OpenFileRead(const char* path) = 0;
  virtual idFile* OpenFileWrite(const char* path) = 0;
  virtual bool ReadFile(const char* path, std::vector<std::uint8_t>& data) = 0;
  virtual bool WriteFile(const char* path, const void* data, std::size_t size) = 0;
  virtual bool FileExists(const char* path) const = 0;
  virtual std::uint32_t GetTimestamp(const char* path) const = 0;
  virtual bool RemoveFile(const char* path) = 0;
  virtual idFileList* ListFiles(const char* directory, const char* extension,
                                bool recursive = false) const = 0;
  virtual void FreeFileList(idFileList* list) const { delete list; }
  // NEEDS_WORK(shared/idlib/filesystem): renderer-facing portions of the
  // retail interface; the full vtable will be recovered with filesystem.
  virtual void FixLongFilename(const char* basePath, const char* extension,
                               const char* input, char* output,
                               int outputLength) const {
    idStr::snPrintf(output, outputLength, "%s%s.%s", basePath ? basePath : "",
                    input ? input : "", extension ? extension : "");
  }
  std::uint32_t GetTimestamp(const char* path, bool) const {
    return GetTimestamp(path);
  }
  bool FileExists(const char* path, bool) const { return FileExists(path); }
  bool RemoveFile(const char* path, fsPath_t) { return RemoveFile(path); }
};

class idFileSystemLocal final : public idFileSystem {
public:
  void Init(const char* root) override;
  void Shutdown() override;
  idFile* OpenFileRead(const char* path) override;
  idFile* OpenFileWrite(const char* path) override;
  bool ReadFile(const char* path, std::vector<std::uint8_t>& data) override;
  bool WriteFile(const char* path, const void* data, std::size_t size) override;
  bool FileExists(const char* path) const override;
  std::uint32_t GetTimestamp(const char* path) const override;
  bool RemoveFile(const char* path) override;
  bool RenameFile(const char* from, const char* to);
  idFileList* ListFiles(const char* directory, const char* extension,
                        bool recursive = false) const override;
  idStr BuildOSPath(const char* path) const;

private:
  idStr rootPath;
};

extern idFileSystemLocal fileSystemLocal;
extern idFileSystem* fileSystem;
