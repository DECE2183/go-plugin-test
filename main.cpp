#include <iostream>

#include "plugin.hpp"

int main()
{
  plugin pl_printtime("printtime");
  try
  {
    pl_printtime.compile();
  }
  catch (const std::exception &e)
  {
    std::cout << e.what() << std::endl;
    return 4;
  }

  try
  {
    pl_printtime.load();
  }
  catch (const std::exception &e)
  {
    std::cout << e.what() << std::endl;
    return 6;
  }

  try
  {
    pl_printtime.execute();
  }
  catch (const std::exception &e)
  {
    std::cout << e.what() << std::endl;
    return 8;
  }

  pl_printtime.unload();

  return 0;
}
