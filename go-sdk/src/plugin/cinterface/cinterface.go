package cinterface

/*
typedef struct {
  void (*host_cout)(const char *str);
} interface_t;

static void (*host_cout_func)(const char *str);
static inline void HostCout(const char *str)
{
  if (host_cout_func == 0) return;
  host_cout_func(str);
}

static inline void InitInterface(int *arg_ptr)
{
  interface_t *interface = (interface_t *)arg_ptr;
  host_cout_func = interface->host_cout;
}
*/
import "C"

import (
  "strconv"
)

var Hashsum string = "0"

//export initInterface
func initInterface(addr *C.int) {
  C.InitInterface(addr)
}

//export getHashsum
func getHashsum() C.ulonglong {
  sum, _ := strconv.ParseUint(Hashsum, 10, 64)
  return C.ulonglong(sum)
}


// Public funcs
func HostCout(str string) {
  C.HostCout(C.CString(str))
}
