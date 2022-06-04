#pragma once

#include <string>
#include <filesystem>
#include <iostream>
//#include <cstdlib>
#include <stdlib.h>

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
    _interface(&interface_ptr), _compiled(false), _loaded(false), _name(name)
  {
    _path_to_source = plugins_src_path + path_sep + _name;
    _path_to_binary = plugins_bin_path + path_sep + _name + ".lib";
  }

  void compile()
  {
    if (!fs::exists(_path_to_source) || !fs::is_directory(_path_to_source))
      throw std::runtime_error("Plugin " + _name + " does not exists");

    setenv("GOROOT", go_sdk_path.c_str(), true);
    setenv("GOPATH", plugins_path.c_str(), true);

    std::string cmd = "cd " + _path_to_source + " && go build -o " + _path_to_binary + " -buildmode c-shared .";

    std::cout << "Building plugin \"" << _name << "\"..." << std::endl;
    std::cout << cmd << std::endl;

    if (system(cmd.c_str()) != 0)
      throw std::runtime_error("Compilation error");

    std::cout << "Build done." << std::endl;
    _compiled = true;
  }

  void load()
  {
    if (!_compiled)
      throw std::runtime_error("Plugin \"" + _name + "\" is not compiled");
    if (_loaded)
      throw std::runtime_error("Plugin \"" + _name + "\" already loaded");

#if _WIN32
    auto lib_hadle = LoadLibrary(_path_to_binary.c_str());
    if (lib_hadle == nullptr)
      throw std::runtime_error("Cant load plugin \"" + _name + "\"");

    _lib_ptr = lib_hadle;
    _init_interface = (void (*)(int *interface_addr)) GetProcAddress(lib_hadle, "init_interface");
    _get_info = (void (*)()) GetProcAddress(lib_hadle, "GetInfo");
    _execute = (void (*)()) GetProcAddress(lib_hadle, "Exec");
#else

#endif

    if (_init_interface == nullptr)
      throw std::runtime_error("Plugin \"" + _name + "\" has no propper interface");

    _init_interface((int *)_interface);

    if (_get_info == nullptr || _execute == nullptr)
      throw std::runtime_error("Cant get plugin \"" + _name + "\" interface");

    _loaded = true;
  }

  void unload()
  {
    if (!_loaded)
      throw std::runtime_error("Plugin \"" + _name + "\" not loaded");

#if _WIN32
    FreeLibrary((HINSTANCE) _lib_ptr);
#else

#endif
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

  void (*_init_interface)(int *interface_addr);
  void (*_get_info)();
  void (*_execute)();

  std::string _name;
  std::string _path_to_source;
  std::string _path_to_binary;

  bool _compiled;
  bool _loaded;

  const interface_t *_interface;
};
