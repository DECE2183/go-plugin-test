#pragma once

#include <string>
#include <filesystem>
#include <iostream>
#include <cstdlib>

#if _WIN32
#include <windows.h>
const char path_sep = '\\';
#else
#include <dlfcn.h>
const char path_sep = '/';
#endif

namespace fs = std::filesystem;

const std::string working_path = fs::current_path().string();
const std::string plugins_path = working_path + path_sep + "plugins";
const std::string plugins_bin_path = plugins_path + path_sep + "bin";

class plugin
{
public:
  plugin(const std::string &name) : _compiled(false), _loaded(false), _name(name)
  {
    _path_to_source = plugins_path + path_sep + _name + ".go";
    _path_to_binary = plugins_bin_path + path_sep + _name + ".lib";
  }

  void compile()
  {
    if (fs::is_directory(_path_to_source) || !fs::exists(_path_to_source))
      throw std::runtime_error("Plugin " + _name + " does not exists");

    std::string cmd = "go build -o " + _path_to_binary + " -buildmode c-shared " + _path_to_source;
    std::cout << "Building plugin \"" << _name << "\"..." << std::endl;

    if (std::system(cmd.c_str()) != 0)
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
    _get_info = (void (*)()) GetProcAddress(lib_hadle, "GetInfo");
    _execute = (void (*)()) GetProcAddress(lib_hadle, "Exec");

    if (_get_info == nullptr || _execute == nullptr)
      throw std::runtime_error("Cant get plugin \"" + _name + "\" interface");
#else

#endif

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

  void (*_get_info)();
  void (*_execute)();

  std::string _name;
  std::string _path_to_source;
  std::string _path_to_binary;

  bool _compiled;
  bool _loaded;
};
