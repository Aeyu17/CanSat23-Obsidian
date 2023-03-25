package main

import (
	"fmt"
	"math/rand"
    "time"

	backend "github.com/Aeyu17/CanSat23-Obsidian/backend"
	server "github.com/Aeyu17/CanSat23-Obsidian/server"
)

func main() {
	fmt.Println("Starting Ground Control Station...")
	server.InitServer()

	backend.WriteToCSV("hi")
}
