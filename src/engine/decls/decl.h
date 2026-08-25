#pragma once

#include "shared/idlib/containers/hashindex.h"
#include "shared/idlib/containers/list.h"
#include "shared/idlib/text/atomicstring.h"
#include "shared/idlib/text/parser.h"
#include "shared/idlib/text/str.h"
#include "shared/idlib/typeinfo/typeinfo.h"

#include <cstdint>

enum production_t {
  PROD_DEVELOPMENT = 0,
  PROD_BUILDING = 1,
  PROD_PRODUCTION = 2,
  PROD_LOADED = 3,
};

class idResource;
class idDecl;
class idDeclFile;
class idDeclInfo;
class idBitMsg;
struct idDeclSource;
struct idAutoComplete;

class idResourceList {
public:
  virtual ~idResourceList() = default;
  virtual const idResource* GetResourceByID(int resourceID) const;
  virtual idResource* Alloc(const char* name) const;
  virtual int AllocSize() const;
  idResource* Load(const char* name, bool load = true, bool forceReload = false);
  idResource* FindExisting(const char* name,
                           bool skipStaleCheck = false) const;
  idResource* Index(int index) const;
  void GetLoadedResources(idList<idResource*, 99>& resources) const;
  void Add(idResource* resource);
  void Remove(idResource* resource);
  const idResource* ReadResourceFromMsg(idBitMsg* message) const;
  void WriteResourceToMsg(const idResource* resource, idBitMsg* message) const;

  const char* resourceTypeName = "resource";
  idResourceList* nextResourceList = nullptr;
  idList<idResource*> resourceList;
  idHashIndex resourceIDHash;
};

class idResource {
public:
  idResource();
  virtual ~idResource();
  virtual void LoadResource();
  virtual bool ReloadIfStale();
  virtual void WriteResourceFile();
  virtual idResourceList* GetResourceList() const;
  virtual void Print();
  virtual void List();

  void SetName(const char* resourceName);
  const char* GetName() const;
  const char* GetResourceTypeName() const;
  void SetResourceError(const char* format, ...);

  idAtomicString name;
  idResourceList* resourceListPtr;
  const char* resourceError;
  int resourceID;
  int staleCount;
  std::uint8_t loadedResource : 1;
  std::uint8_t staticResource : 1;
  std::uint8_t insideRecursiveLoad : 1;
  std::uint8_t buildForceLoad : 1;
  std::uint8_t loadDeferred : 1;
};

template <typename T> class idTypedResourceList : public idResourceList {
public:
  explicit idTypedResourceList(const char* typeName = "resource") {
    resourceTypeName = typeName;
  }
  T* Alloc(const char* resourceName) const override {
    T* resource = new T;
    resource->resourceListPtr = const_cast<idTypedResourceList<T>*>(this);
    resource->SetName(resourceName);
    auto& mutableResources =
        const_cast<idTypedResourceList<T>*>(this)->resourceList;
    mutableResources.Append(resource);
    resource->resourceID = mutableResources.Num() - 1;
    return resource;
  }
  int AllocSize() const override { return static_cast<int>(sizeof(T)); }
};

struct idDeclSource {
  idAtomicString name;
  idDeclFile* sourceFile = nullptr;
  int sourceLine = 0;
  idDeclSource* nextInFile = nullptr;
  bool isStatic = false;
  int sourceTextOffset = 0;
  int sourceTextLength = 0;
  const char* GetFileName() const;
};

class idDeclFile {
public:
  idDeclFile();
  ~idDeclFile();
  bool SourceFileChanged(bool force) const;
  std::uint32_t LoadAndParse();
  void FreeDynamic();
  idDecl* AddDecl(idDeclInfo* type, const char* declName, int line, int textOffset,
                  int textLength, bool parseDecl = false);

  bool hadRedefinition = false;
  idAtomicString fileName;
  idDeclInfo* defaultType = nullptr;
  std::uint32_t timestamp = 0;
  std::uint32_t checksum = 0;
  int fileSize = 0;
  int numLines = 0;
  bool everReloaded = false;
  idDeclSource* firstDeclSourceInFile = nullptr;
  idList<int, 46> dependencies;
  int dependenciesDynamicStartIndex = 0;
  idList<idStr> includeFiles;
};

class idDeclInfo : public idResourceList {
public:
  idDeclInfo(const char* declName = "decl", const char* className = "idDecl",
             const char* extension = "decl", bool remapping = false);
  ~idDeclInfo() override;
  virtual idDecl* AllocDecl();
  const idResource* GetResourceByID(int resourceID) const override;
  idDeclSource* FindDeclSource(const char* declName) const;
  idDecl* FindResource(const char* declName, bool load = true) const;
  const char* RemapResourceName(char* buffer, int bufferSize, const char* resourceName) const;
  const idDecl* FindExisting(const char* resourceName, bool remap = true) const;
  idDecl* FindWithInheritance(const char* resourceName, bool makeDefault = true,
                              bool remap = true) const;
  const idResource* GetResourceByIDWithInheritance(int resourceID) const;
  void RemoveDeclSource(idDeclSource* source);
  void AddDeclSource(idDeclSource* source);
  void FreeDynamic();

  const char* name = "decl";
  const char* classname = "idDecl";
  const char* fileExtension = ".decl";
  bool usesRemapping = false;
  idAtomicString nameExtension;
  idAtomicString defaultFolder;
  idDeclInfo* parent = nullptr;
  void (*onReload)(const idDecl*) = nullptr;
  idList<idDeclSource*, 46> declSources;
  idHashIndex declSourceHash;
  int declSourceDynamicStartIndex = 0;
};

class idDeclManager {
public:
  virtual ~idDeclManager() = default;
  virtual void Init() = 0;
  virtual void Shutdown() = 0;
  virtual void Reload(bool force) = 0;
  virtual void MergeDecls(const idStr& mapName, const idStr& mapFile) = 0;
  virtual void MarkStatic() = 0;
  virtual void FreeDynamic() = 0;
  virtual void RegisterDeclFolder(const char* folder, const char* extension,
                                  idDeclInfo* defaultType) = 0;
  virtual int GetNumDeclTypes() const = 0;
  virtual idDeclInfo* GetDeclType(const char* typeName) const = 0;
  virtual idDeclInfo* GetDeclType(int index) const = 0;
  virtual idDeclInfo* GetDeclTypeFromClassname(const char* className) const = 0;
  virtual idDecl* CreateNewDecl(idDeclInfo* type, const char* name, const char* fileName,
                                const char* sourceText) = 0;
  virtual void AddDependency(idDecl& decl, const char* fileName) = 0;
  virtual void AddDependency(idDecl& decl, const idDecl* dependency);
  virtual void GetDeclFilesDependentOnDeclFile(const idDeclFile& source,
                                                idList<idDeclFile*>& result) const = 0;
  virtual void ArgCompletion_DeclTypeName(idAutoComplete* completion) = 0;
  virtual void ArgCompletion_DeclName(idAutoComplete* completion, idDeclInfo* type) = 0;
  virtual void PrintMultiplayerDeclOverrides() = 0;
  void GetDeclTextHeader(const idDeclInfo& info, const char* name, idStr& header) const;
  virtual bool ShouldLogMissingDecl(const idDecl& decl) const;
  static void ArgCompletion_DeclTypeAndName(idAutoComplete* completion);
  template <typename T>
  static void ArgCompletion_Decl(idAutoComplete& completion);
};

enum scFileStatus_t { SCF_UNKNOWN, SCF_CHECKEDIN, SCF_CHECKEDOUT, SCF_NOTIMPORTED };
enum scFileType_t { SCT_NONE };

class idSourceControl {
public:
  virtual ~idSourceControl() = default;
  virtual scFileStatus_t GetFileStatus(const idStr& file) = 0;
  virtual std::uint32_t CheckOut(const idList<idStr>& files) = 0;
  virtual std::uint32_t Import(const idList<idStr>& files, bool keepCheckedOut, bool submit,
                               scFileType_t fileType) = 0;
  std::uint32_t CheckOut(const idStr& file);
  std::uint32_t Import(const idStr& file, bool keepCheckedOut, bool submit, scFileType_t fileType);
};

class idDecl : public idResource {
public:
  idDecl();
  ~idDecl() override;

  void LoadResource() override;
  bool ReloadIfStale() override;
  void WriteResourceFile() override;
  void List() override;
  virtual std::uint32_t GetDeclTimestamp();
  virtual void ReParse(bool fromDisk);
  virtual idDeclInfo* GetDeclInfo() = 0;
  virtual bool RebuildTextSource();
  virtual bool SetImplicitText();
  virtual const char* DefaultDefinition() const;
  virtual void LogMissingDecl();
  virtual void Parse(idParser& source) = 0;
  virtual void FreeData();
  virtual std::uint64_t Size() const;
  virtual bool IsInventoryDecl() const;

  int GetLineNum() const;
  int GetTextLength() const;
  bool SourceFileExists() const;
  bool EverReloaded() const;
  idDeclSource* DeclSource() const;
  bool IsImplicit() const;
  const char* GetFileName() const;
  void SetText(const char* text, int length = 0);
  void MakeDefault();
  void Revert(bool force = false);
  void DefineFromSource(idDeclSource* source);
  void DeleteUnusedText();
  char* GetText(bool force = false);
  bool ReplaceSourceFileText(bool useSourceControl);

  char* textSource;
  int textLength;
  std::uint32_t md5Checksum;
  bool programaticallyDefined;
  idDeclSource* declSource;
};

template <typename T> class idDeclInfoTemplate : public idDeclInfo {
public:
  using idDeclInfo::idDeclInfo;
  T* Alloc(const char* resourceName) const override {
    T* decl = new T;
    decl->resourceListPtr = const_cast<idDeclInfoTemplate<T>*>(this);
    decl->SetName(resourceName);
    return decl;
  }
  int AllocSize() const override { return static_cast<int>(sizeof(T)); }
  idDecl* AllocDecl() override {
    T* decl = new T;
    decl->resourceListPtr = this;
    return decl;
  }
};

class idDeclTypeInfo : public idDecl {
public:
  idDeclTypeInfo();
  ~idDeclTypeInfo() override;
  const char* DefaultDefinition() const override;
  idDeclInfo* GetDeclInfo() override;
  void Parse(idParser& source) override;
  bool RebuildTextSource() override;
  void SetState(idDeclTypeInfo* empty);

  idDeclTypeInfo* parent;
  bool hasBeenParsedAtLeastOnce;

  static idDeclInfoTemplate<idDeclTypeInfo> resourceList;
};

idStr ProductionFilename(const idDecl* resource);

extern idDeclManager* declManager;
extern void (*resourceLoadCallback)(idResource* resource);
extern idTypeInfoTools* declTypeInfoTools;

static_assert(sizeof(idResource) == 48, "idResource ABI drift");
static_assert(sizeof(idDecl) == 80, "idDecl ABI drift");
static_assert(sizeof(idDeclSource) == 48, "idDeclSource ABI drift");
