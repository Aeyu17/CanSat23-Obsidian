package main

import (
	"fmt"
	"time"

	backend "github.com/Aeyu17/CanSat23-Obsidian/backend"
	server "github.com/Aeyu17/CanSat23-Obsidian/server"
)

var mode = "none"
// can be flight, sim, and gen
/*
flight - proper flight mode (default)
sim - cansat designated sim mode
gen - ground station generator sim mode
none - 
*/

const PORT = "COM5"
const BAUD = 9600
// if you change this, you need to go to server.go and change that too

func packetTransceiver() {	
	fmt.Println("Running in " + mode + " mode.")
	for {
		// RECEIVER
		var packet string;
		switch mode {
		case "flight":
			packet = backend.GetPacket(PORT, BAUD)
			backend.WriteToCSV(packet, "flightlaunchdata.csv")

		case "sim":
			simpPacket := backend.ReadPressureCSV("cansat_2023_simp.csv")
			if len(simpPacket) == 0 {
				fmt.Println("Simulation complete.")
				mode = "flight"
				continue
			}
			backend.SendPacket(PORT, BAUD, simpPacket)

			packet = backend.GetPacket(PORT, BAUD)
			backend.WriteToCSV(packet, "simlaunchdata.csv")

		case "gen":
			packet = backend.GeneratePacket()
			backend.WriteToCSV(packet, "genlaunchdata.csv")
			time.Sleep(time.Second/2)

		default:
			continue
		}

		// TRANSMITTER
		server.ServerWrite(packet)
	}
}

func main() {
	go packetTransceiver()

	fmt.Println("Starting Ground Control Station...")
	server.InitServer() // should be the last thing run
	
	
}
