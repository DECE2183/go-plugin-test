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
    pl_printtime.compile();
    pl_printtime.load();
    pl_printtime.execute();
  }
  catch (const std::exception &e)
  {
    std::cout << e.what() << std::endl;
    return 1;
  }

  pl_printtime.unload();

  return 0;
}
