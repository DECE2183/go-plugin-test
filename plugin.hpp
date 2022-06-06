#pragma once

#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>

#include "crc64.hpp"

#if _WIN32
#include <windows.h>
const char path_sep = '\\';
int setenv(const char *name, const char *value, int overwrite)
{
    int errcode = 0;
    if(!overwrite) {
        size_t envsize = 0;
        errcode = getenv_s(&envsize, NULL, 0, name);
        if(errcode || envsize) return errcode;
    }
    return _putenv_s(name, value);
}
#else
#include <dlfcn.h>
const char path_sep = '/';
#endif

namespace fs = std::filesystem;

const std::string working_path = fs::current_path().string();
const std::string go_sdk_path = working_path + path_sep + "go-sdk";
const std::string plugins_path = working_path + path_sep + "plugins";
const std::string plugins_src_path = plugins_path + path_sep + "src";
const std::string plugins_bin_path = plugins_path + path_sep + "bin";

typedef struct {
  void (*host_cout)(const char *str);
} interface_t;

class plugin
{
public:
  plugin(const interface_t &interface_ptr, const std::string &name) :
    _interface(&interface_ptr), _loaded(false), _name(name)
  {
    _path_to_source = plugins_src_path + path_sep + _name;
    _path_to_binary = plugins_bin_path + path_sep + _name + ".lib";
    crc64::generate();
  }

  ~plugin()
  {
    unload();
  }

  void compile()
  {
    if (!fs::exists(_path_to_source) || !fs::is_directory(_path_to_source))
      throw std::runtime_error("Plugin " + _name + " does not exists");

    setenv("GOROOT", go_sdk_path.c_str(), true);
    setenv("GOPATH", plugins_path.c_str(), true);

    uint64_t hashsum = calc_hash(_path_to_source);

    std::string cmd = "cd \"" + _path_to_source + "\" && go build -o \"" +
        _path_to_binary + "\" -ldflags=\"-X \'plugin/cinterface.Hashsum=" +
        std::to_string(hashsum) + "\'\" -buildmode c-shared . ";

    std::cout << "Building plugin \"" << _name << "\"..." << std::endl;
    std::cout << cmd << std::endl;

    if (system(cmd.c_str()) != 0)
      throw std::runtime_error("Compilation error");

    std::cout << "Build done." << std::endl;
  }

  void autoload()
  {
    if (!fs::exists(_path_to_binary))
    {
      try
      {
        compile();
        load();
      }
      catch (std::runtime_error &e)
      {
        throw e;
      }
    }
    else
    {
      load();
      uint64_t pl_hash = _get_hashsum();
      uint64_t src_hash = calc_hash(_path_to_source, 0);
      if (pl_hash != src_hash)
      {
        try
        {
          compile();
          unload();
          load();
        }
        catch (std::runtime_error &e)
        {
          std::cout << "Error: ";
          throw e;
        }
      }
    }
  }

  void load()
  {
    if (!fs::exists(_path_to_binary) || fs::is_directory(_path_to_binary))
      throw std::runtime_error("Plugin \"" + _name + "\" is not compiled");
    if (_loaded)
      throw std::runtime_error("Plugin \"" + _name + "\" already loaded");

#if _WIN32
    _lib_ptr = LoadLibrary(_path_to_binary.c_str());
    if (_lib_ptr == nullptr)
      throw std::runtime_error("Cant load plugin \"" + _name + "\"");

    _init_interface = (void (*)(int *interface_addr)) GetProcAddress((HMODULE)_lib_ptr, "initInterface");
    _get_hashsum = (uint64_t (*)()) GetProcAddress((HMODULE)_lib_ptr, "getHashsum");
    _get_info = (void (*)()) GetProcAddress((HMODULE)_lib_ptr, "GetInfo");
    _execute = (void (*)()) GetProcAddress((HMODULE)_lib_ptr, "Exec");
#else
    _lib_ptr = dlopen(_path_to_binary.c_str(), RTLD_LAZY);
    if (_lib_ptr == nullptr)
      throw std::runtime_error("Cant load plugin \"" + _name + "\": " + dlerror());

    _init_interface = (void (*)(int *interface_addr)) dlsym(_lib_ptr, "initInterface");
    _get_hashsum = (uint64_t (*)()) dlsym(_lib_ptr, "getHashsum");
    _get_info = (void (*)()) dlsym(_lib_ptr, "GetInfo");
    _execute = (void (*)()) dlsym(_lib_ptr, "Exec");
#endif

    if (_init_interface == nullptr || _get_hashsum == nullptr)
      throw std::runtime_error("Plugin \"" + _name + "\" has no propper interface");

    _init_interface((int *)_interface);

    if (_get_info == nullptr || _execute == nullptr)
      throw std::runtime_error("Cant get plugin \"" + _name + "\" base methods");

    _loaded = true;
  }

  void unload()
  {
    if (!_loaded)
      throw std::runtime_error("Plugin \"" + _name + "\" not loaded");

#if _WIN32
    bool free_succ = FreeLibrary((HMODULE)_lib_ptr);
#else
    dlclose(_lib_ptr);
#endif

    _lib_ptr = nullptr;
    _init_interface = nullptr;
    _get_hashsum = nullptr;
    _get_info = nullptr;
    _execute = nullptr;

    _loaded = false;
  }

  void execute()
  {
    if (!_loaded)
      throw std::runtime_error("Plugin \"" + _name + "\" not loaded");

    try
    {
      _execute();
    }
    catch (const std::exception &e)
    {
      throw e;
    }
  }

private:
  void *_lib_ptr;

  void     (*_init_interface)(int *interface_addr);
  uint64_t (*_get_hashsum)();
  void     (*_get_info)();
  void     (*_execute)();

  std::string _name;
  std::string _path_to_source;
  std::string _path_to_binary;

  bool _loaded;

  const interface_t *_interface;

  uint64_t calc_hash(const std::string &path, uint64_t sum = 0)
  {
    if (fs::is_directory(path))
    {
      for (const auto &entry : fs::directory_iterator(path))
      {
        sum = calc_hash(entry.path().string(), sum);
      }
    }
    else
    {
      std::ifstream r_file(path);
      std::stringstream buffer;
      buffer << r_file.rdbuf();
      r_file.close();
      sum = crc64::calc(buffer.str(), sum);
    }

    return sum;
  }
};
