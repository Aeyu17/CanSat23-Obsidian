package main

import (
	"fmt"
	"container/list"

	backend "github.com/Aeyu17/CanSat23-Obsidian/backend"
)

var mode = "none"
// can be flight, sim, and gen
/*
flight - proper flight mode (default)
sim - cansat designated sim mode
gen - ground station generator sim mode
none - no mode active
*/

func packetTransceiver(l *list.List) {	
	fmt.Println("Running in " + mode + " mode.")
	for {
		// RECEIVER
		var packet string;
		switch mode {
		case "flight":
			packet = backend.GetDataPacket(l)
			backend.WriteToCSV(packet, "flightlaunchdata.csv")

		case "sim":
			simpPacket := backend.ReadPressureCSV("cansat_2023_simp.csv")
			if len(simpPacket) == 0 {
				fmt.Println("Simulation complete.")
				mode = "flight"
				continue
			}
			backend.SendPacket(backend.PORT, backend.BAUD, simpPacket)

			packet = backend.GetDataPacket(l)
			backend.WriteToCSV(packet, "simlaunchdata.csv")

		case "gen":
			packet = backend.GetDataPacket(l)
			backend.WriteToCSV(packet, "genlaunchdata.csv")

		default:
			continue
		}

		// TRANSMITTER
		backend.ServerWrite(packet)
	}
}

func main() {
	var packetChannel chan string;

	go backend.PacketReceiver(packetChannel)
	go backend.PacketQueuer(packetChannel, backend.PacketList)

	go packetTransceiver(backend.PacketList)

	fmt.Println("Starting Ground Control Station...")
	backend.InitServer() // should be the last thing run
	
	
}
