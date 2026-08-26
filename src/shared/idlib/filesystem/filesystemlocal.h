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
  virtual idFile* OpenFileAppend(const char* path, fsPath_t basePath) = 0;
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
    const char* selectedInput = input != nullptr ? input : "";
    if (idStr::Icmpn(selectedInput, "../../", 6) == 0)
      selectedInput += 6;

    idStr fixedName;
    if (basePath != nullptr && basePath[0] != '\0') {
      fixedName = basePath;
      fixedName.AppendPath(selectedInput);
    } else {
      fixedName = selectedInput;
    }
    fixedName.SetFileExtension(extension != nullptr ? extension : "");

    idStr leafName;
    fixedName.ExtractFileName(leafName);
    if (leafName.Length() >= 38) {
      idStr filePath;
      fixedName.ExtractFilePath(filePath);
      leafName.StripFileExtension();
      fixedName = filePath;
      int runLength = 0;
      for (int index = 0; index < leafName.Length(); ++index) {
        if (runLength >= 30) {
          fixedName += '/';
          runLength = 0;
        }
        fixedName += leafName[index];
        ++runLength;
      }
      fixedName.SetFileExtension(extension != nullptr ? extension : "");
    }
    idStr::Copynz(output, fixedName.data, outputLength);
  }
  void FixLongFilename(const char* basePath, const char* extension,
                       const char* input, char* output, int outputLength,
                       int) const {
    FixLongFilename(basePath, extension, input, output, outputLength);
  }
  std::uint32_t GetTimestamp(const char* path, bool) const {
    return GetTimestamp(path);
  }
  bool FileExists(const char* path, bool) const { return FileExists(path); }
  bool RemoveFile(const char* path, fsPath_t) { return RemoveFile(path); }
  idFile* OpenFileRead(const char* path, bool, bool) {
    return OpenFileRead(path);
  }
  // NEEDS_WORK(shared/idlib/filesystem): retail exposes this renderer-facing
  // read/write opener.  Keep the exact call boundary here until the filesystem
  // compiland supplies its path policy and permanent-file implementation.
  virtual idFile* OpenFileReadWrite(const char* path, bool, fsPath_t) {
    return OpenFileWrite(path);
  }
  idFile* OpenFileWrite(const char* path, fsPath_t) {
    return OpenFileWrite(path);
  }
  // NEEDS_WORK(shared/idlib/filesystem): this is the renderer-facing retail
  // boundary. Recover the platform/path-selection implementation with the
  // filesystem module; the fallback preserves already-native full paths.
  virtual void RelativePathToOSPath(const char* relativePath, char* output,
                                    int outputLength, fsPath_t) const {
    idStr::Copynz(output, relativePath != nullptr ? relativePath : "",
                  outputLength);
  }
  // NEEDS_WORK(shared/idlib/filesystem): exact search-path selection belongs
  // to the filesystem recovery pass.
  virtual const char* GetBasePathStr(fsPath_t) const { return ""; }
  // NEEDS_WORK(shared/idlib/filesystem): exact numbering/search policy belongs
  // to filesystem. This boundary preserves the retail signature for renderer
  // screenshot commands until that compiland is recovered.
  virtual void GetNumberedFilename(int* lastNumber, const char* baseName,
                                   const char* extension, idStr* output,
                                   bool) const {
    const int number = lastNumber != nullptr ? ++*lastNumber : 1;
    output->Format("%s%05d.%s", baseName != nullptr ? baseName : "", number,
                   extension != nullptr ? extension : "");
  }
};

class idFileSystemLocal final : public idFileSystem {
public:
  void Init(const char* root) override;
  void Shutdown() override;
  idFile* OpenFileRead(const char* path) override;
  idFile* OpenFileWrite(const char* path) override;
  idFile* OpenFileAppend(const char* path, fsPath_t basePath) override;
  bool ReadFile(const char* path, std::vector<std::uint8_t>& data) override;
  bool WriteFile(const char* path, const void* data, std::size_t size) override;
  bool FileExists(const char* path) const override;
  std::uint32_t GetTimestamp(const char* path) const override;
  bool RemoveFile(const char* path) override;
  bool RenameFile(const char* from, const char* to);
  idFileList* ListFiles(const char* directory, const char* extension,
                        bool recursive = false) const override;
  idStr BuildOSPath(const char* path) const;
  const char* GetBasePathStr(fsPath_t) const override {
    return rootPath.data;
  }

private:
  idStr rootPath;
};

extern idFileSystemLocal fileSystemLocal;
extern idFileSystem* fileSystem;
