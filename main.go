package main

import (
	"fmt"

	server "github.com/Aeyu17/CanSat23-Obsidian/server"
)

func main() {
	fmt.Println("Starting Ground Control Station...")
	server.InitServer()
}
