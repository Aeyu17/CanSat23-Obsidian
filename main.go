package main

import (
	"fmt"
	"time"

	backend "github.com/Aeyu17/CanSat23-Obsidian/backend"
	server "github.com/Aeyu17/CanSat23-Obsidian/server"
)

var mode = "flight"
// can be flight, sim, and gen
/*
flight - proper flight mode (default)
sim - cansat designated sim mode
gen - ground station generator sim mode
*/

const port = "COM5"
const baud = 9600

func packetTransceiver() {
	fmt.Println("Running in " + mode + " mode.")
	for {
		// RECEIVER
		var packet string;
		switch mode {
		case "flight":
			packet = backend.GetPacket(port, baud)
			backend.WriteToCSV(packet, "flightlaunchdata.csv")

		case "sim":
			/*
			read from cansat_2023_simp.csv
			send it to the cansat
			wait for a packet back
			*/
			
			packet = backend.GetPacket(port, baud)
			backend.WriteToCSV(packet, "simlaunchdata.csv")

		case "gen":
			packet = backend.GeneratePacket()
			backend.WriteToCSV(packet, "genlaunchdata.csv")
			time.Sleep(time.Second/2)

		default:
			packet = backend.GeneratePacket()
			backend.WriteToCSV(packet, "genlaunchdata.csv")
			time.Sleep(time.Second/2)
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
