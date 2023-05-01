package backend

import (
	"log"
	"fmt"
	"bufio"

	"github.com/tarm/serial"
)

const PORT = "COM7"
const BAUD = 9600
var (
	SerialPort = InitPort(PORT, BAUD)
	PacketReceiverChannel = make(chan string, 100)
	SerialConnected = true
)

func InitPort(port string, baud int) (SerialPort *serial.Port) {
	c := &serial.Config{Name: port, Baud: baud}
	SerialPort, err := serial.OpenPort(c)
	if err != nil {
		log.Println(err)
		SerialConnected = false
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