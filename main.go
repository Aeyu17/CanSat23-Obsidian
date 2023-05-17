package main

import (
	"container/list"
	"fmt"
	"sync"
	"time"
	"bufio"

	backend "github.com/Aeyu17/CanSat23-Obsidian/backend"
)

var mu sync.Mutex;

// can be flight, sim, and gen
/*
flight - proper flight mode (default)
sim - cansat designated sim mode
gen - ground station generator sim mode
none - no mode active
*/

func packetTransceiver(l *list.List) {
	fmt.Println("Running in " + backend.Mode + " mode.")
	for {
		// RECEIVER
		var packet string
		switch backend.Mode {
		case "flight":
			
			packet = backend.GetDataPacket(l)
			if packet == "Empty" {
				continue
			}
			backend.WriteToCSV(packet, "flightlaunchdata.csv")

		case "sim":
			if !backend.SimActive {
				continue
			}
			simpPacket := backend.ReadPressureCSV("cansat_2023_simp.csv")
			if len(simpPacket) == 0 {
				fmt.Println("Simulation complete.")
				backend.Mode = "none"
				continue
			}
			backend.SendPacket(simpPacket + "\n")

			packet = backend.GetDataPacket(l)
			if packet == "Empty" {
				continue
			}
			backend.WriteToCSV(packet, "simlaunchdata.csv")

		case "gen":
			packet = backend.GetDataPacket(l)
			if packet == "Empty" {
				continue
			}
			backend.WriteToCSV(packet, "genlaunchdata.csv")

		default:
			fmt.Println("No mode enabled.")
			time.Sleep(time.Second)
			continue
		}

		// TRANSMITTER
		fmt.Println(packet)
		mu.Lock()
		backend.ServerWrite(packet)
		mu.Unlock()
	}
}

func main() {
	defer backend.SerialPort.Close()

	var packetChannel = make(chan string, 100)

	go func(){
		for {
			if !backend.SerialConnected {
				fmt.Println("Trying to reconnect the port...")
				backend.SerialPort = backend.InitPort(backend.BAUD)
				backend.SerialReader = bufio.NewReaderSize(backend.SerialPort, 1024)
			}
			time.Sleep(time.Second)
		}
	}()

	go backend.PacketReceiver(packetChannel)
	go backend.PacketEnqueuer(packetChannel, backend.PacketList)

	go packetTransceiver(backend.PacketList)

	fmt.Println("Starting Ground Control Station...")
	backend.InitServer() // should be the last thing run
}
