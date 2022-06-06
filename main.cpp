#include <iostream>

#include "plugin.hpp"

void _host_cout(const char *str)
{
  std::cout << str << std::endl;
}

interface_t init_interface = {
  .host_cout = _host_cout
};


int main()
{
  plugin pl_printtime(init_interface, "printtime");
  try
  {
    // pl_printtime.compile();
    // pl_printtime.load();
    pl_printtime.autoload();
    pl_printtime.execute();
  }
  catch (const std::exception &e)
  {
    std::cout << e.what() << std::endl;
    return 1;
  }

  std::cout << "One more time" << std::endl;
  try
  {
    // pl_printtime.unload();
    // pl_printtime.autoload();
    pl_printtime.execute();
  }
  catch (const std::exception &e)
  {
    std::cout << e.what() << std::endl;
    return 1;
  }

  return 0;
}
