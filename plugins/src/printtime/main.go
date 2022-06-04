package main

import (
  "plugin/cinterface"
  "C"
)

//export GetInfo
func GetInfo() (name, version, inputType, outputType string) {
  return "Print Time", "1.0.0", "void", "void"
}

//export Exec
func Exec() {
  // fmt.Printf("Time: %s\n", time.Now())
  // fmt.Printf("Func addr in go: %d\n", C.get_host_cout())
  // C.host_cout(C.CString(time.Now().String()))
  cinterface.HostCout(GetTime())
}

func main() {
  Exec()
}
