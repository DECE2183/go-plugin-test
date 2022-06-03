package main

import (
  "fmt"
  "time"
  "C"
)

//export GetInfo
func GetInfo() (name, version, inputType, outputType string) {
  return "Print Time", "1.0.0", "void", "void"
}

//export Exec
func Exec() {
  fmt.Printf("Current time: %s\n", time.Now())
}

func main() {
  Exec()
}
