package backend

import (
	"log"
	"fmt"
	"bufio"

	"github.com/tarm/serial"
	serialBug "go.bug.st/serial"
)
 
const BAUD = 9600
var (
	Port string;
	SerialPort = InitPort(BAUD)
	PacketReceiverChannel = make(chan string, 100)
	SerialConnected = true
)

func InitPort(baud int) (SerialPort *serial.Port) {
	portList, err := serialBug.GetPortsList()
	if err != nil {
		log.Fatal(err)
	}
	
	if len(portList) == 0 {
		fmt.Println("No COM ports found.")
		SerialConnected = false
		return nil
	} else if len(portList) > 1 {
		fmt.Println("Multiple COM ports found. Disconnect any extra devices.")
	}

	Port = portList[0]
	fmt.Println("Connected to", Port)
	c := &serial.Config{Name: Port, Baud: baud}
	SerialPort, err = serial.OpenPort(c)
	if err != nil {
		log.Fatal(err)
	}
	return
}

func ReceivePacket() {
	if !SerialConnected {
		fmt.Println("XBee not connected.")
		return
	}
	reader := bufio.NewReaderSize(SerialPort, 1024)
	for {
		data, err := reader.ReadString(10)
		if err != nil {
			log.Fatal(err)
		}
		fmt.Println(data)
		PacketReceiverChannel <- data
	}

	// scanner := bufio.NewScanner(SerialPort)
	// for scanner.Scan() {
	// 	data := scanner.Text()
	// 	if err := scanner.Err(); err != nil {
	// 		log.Fatal(err)
	// 	}
	// 	fmt.Println(data)
	// 	PacketReceiverChannel <- data
	// }
}

func SendPacket(data string) {
	if !SerialConnected {
		fmt.Println("XBee not connected.")
		return
	}
	_, err := SerialPort.Write([]byte(data))
	if err != nil {
		log.Println(err)
		return
	}
}