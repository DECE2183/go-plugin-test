package main

import (
  "fmt"
  "time"
)

func GetTime() string {
  return fmt.Sprintf("Current time: %s", time.Now())
}
