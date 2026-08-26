#include <Windows.h>
#include <dia2.h>

#include <cstdio>

namespace {

void PrintBstr(BSTR value) {
  if (value != nullptr) {
    ::wprintf(L"%ls", value);
    SysFreeString(value);
  }
}

void DumpSymbol(IDiaSymbol* symbol, int indent) {
  DWORD tag = 0;
  ULONGLONG length = 0;
  LONG offset = 0;
  DWORD location = 0;
  DWORD dataKind = 0;
  BOOL isVirtual = FALSE;
  DWORD virtualBaseOffset = 0;
  DWORD rva = 0;
  ULONGLONG va = 0;
  BSTR name = nullptr;
  BSTR undecorated = nullptr;
  symbol->get_symTag(&tag);
  symbol->get_name(&name);
  symbol->get_undecoratedName(&undecorated);
  const HRESULT lengthResult = symbol->get_length(&length);
  const HRESULT offsetResult = symbol->get_offset(&offset);
  const HRESULT locationResult = symbol->get_locationType(&location);
  const HRESULT dataResult = symbol->get_dataKind(&dataKind);
  const HRESULT virtualResult = symbol->get_virtual(&isVirtual);
  const HRESULT virtualOffsetResult =
      symbol->get_virtualBaseOffset(&virtualBaseOffset);
  const HRESULT rvaResult = symbol->get_relativeVirtualAddress(&rva);
  const HRESULT vaResult = symbol->get_virtualAddress(&va);

  ::wprintf(L"%*ls[tag=%lu] ", indent, L"", tag);
  PrintBstr(name);
  if (undecorated != nullptr) {
    ::wprintf(L" | ");
    PrintBstr(undecorated);
  }
  if (SUCCEEDED(lengthResult)) ::wprintf(L" len=0x%llX", length);
  if (SUCCEEDED(offsetResult)) ::wprintf(L" off=0x%lX", offset);
  if (SUCCEEDED(locationResult)) ::wprintf(L" loc=%lu", location);
  if (SUCCEEDED(dataResult)) ::wprintf(L" data=%lu", dataKind);
  if (SUCCEEDED(virtualResult)) ::wprintf(L" virt=%d", isVirtual);
  if (SUCCEEDED(virtualOffsetResult))
    ::wprintf(L" vbaseoff=0x%lX", virtualBaseOffset);
  if (SUCCEEDED(rvaResult)) ::wprintf(L" rva=0x%lX", rva);
  if (SUCCEEDED(vaResult)) ::wprintf(L" va=0x%llX", va);

  IDiaSymbol* type = nullptr;
  if (SUCCEEDED(symbol->get_type(&type)) && type != nullptr) {
    BSTR typeName = nullptr;
    ULONGLONG typeLength = 0;
    DWORD typeTag = 0;
    type->get_name(&typeName);
    type->get_length(&typeLength);
    type->get_symTag(&typeTag);
    ::wprintf(L" type(tag=%lu,len=0x%llX,name=", typeTag, typeLength);
    PrintBstr(typeName);
    ::wprintf(L")");
    type->Release();
  }
  ::wprintf(L"\n");
}

void DumpType(IDiaSymbol* global, const wchar_t* name) {
  IDiaEnumSymbols* matches = nullptr;
  if (FAILED(global->findChildren(SymTagUDT, name, nsCaseSensitive,
                                  &matches)) ||
      matches == nullptr) {
    ::wprintf(L"TYPE NOT FOUND: %ls\n", name);
    return;
  }
  IDiaSymbol* type = nullptr;
  ULONG fetched = 0;
  while (matches->Next(1, &type, &fetched) == S_OK && fetched == 1) {
    ::wprintf(L"\n=== %ls ===\n", name);
    DumpSymbol(type, 0);
    IDiaEnumSymbols* children = nullptr;
    if (SUCCEEDED(type->findChildren(SymTagNull, nullptr, nsNone, &children)) &&
        children != nullptr) {
      IDiaSymbol* child = nullptr;
      while (children->Next(1, &child, &fetched) == S_OK && fetched == 1) {
        DumpSymbol(child, 2);
        child->Release();
      }
      children->Release();
    }
    type->Release();
  }
  matches->Release();
}

void DumpPublic(IDiaSymbol* global, const wchar_t* expression) {
  IDiaEnumSymbols* matches = nullptr;
  if (FAILED(global->findChildren(SymTagPublicSymbol, expression,
                                  nsfRegularExpression, &matches)) ||
      matches == nullptr) {
    return;
  }
  ::wprintf(L"\n=== public %ls ===\n", expression);
  IDiaSymbol* symbol = nullptr;
  ULONG fetched = 0;
  while (matches->Next(1, &symbol, &fetched) == S_OK && fetched == 1) {
    DumpSymbol(symbol, 0);
    symbol->Release();
  }
  matches->Release();
}

}  // namespace

int wmain(int argumentCount, wchar_t** arguments) {
  if (FAILED(CoInitialize(nullptr))) return 1;
  IDiaDataSource* source = nullptr;
  HRESULT result = CoCreateInstance(CLSID_DiaSource, nullptr,
                                    CLSCTX_INPROC_SERVER,
                                    __uuidof(IDiaDataSource),
                                    reinterpret_cast<void**>(&source));
  if (FAILED(result) || source == nullptr) {
    std::printf("CoCreateInstance failed: 0x%08lX\n", result);
    return 2;
  }
  result = source->loadDataFromPdb(
      L"E:\\projects\\wolftno\\retail\\WolfNewOrder_x64_rtm.pdb");
  if (FAILED(result)) {
    std::printf("loadDataFromPdb failed: 0x%08lX\n", result);
    return 3;
  }
  IDiaSession* session = nullptr;
  source->openSession(&session);
  IDiaSymbol* global = nullptr;
  session->get_globalScope(&global);

  if (argumentCount > 1) {
    for (int index = 1; index < argumentCount; ++index) {
      if (wcsncmp(arguments[index], L"public:", 7) == 0)
        DumpPublic(global, arguments[index] + 7);
      else
        DumpType(global, arguments[index]);
    }
  } else {
    DumpType(global, L"idAnimator_Base");
    DumpType(global, L"idAnimator_TreeSway");
  }

  global->Release();
  session->Release();
  source->Release();
  CoUninitialize();
  return 0;
}
