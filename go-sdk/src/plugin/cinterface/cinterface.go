package cinterface

/*
#ifndef _C_INTERFACE_
#define _C_INTERFACE_

typedef struct {
  void (*host_cout)(const char *str);
} funcs_decription_t;

static void (*host_cout_func)(const char *str);
static inline void HostCout(const char *str)
{
  if (host_cout_func == 0) return;
  host_cout_func(str);
}

static inline void InitFuncs(int *arg_ptr)
{
  funcs_decription_t *funcs = (funcs_decription_t *)arg_ptr;
  host_cout_func = funcs->host_cout;
}

#endif // _C_INTERFACE_
*/
import "C"

//export init_interface
func init_interface(addr *C.int) {
  C.InitFuncs(addr)
}


// Public funcs
func HostCout(str string) {
  C.HostCout(C.CString(str))
}
