package backend

import (
	"log"
	"fmt"
	"bufio"

	"github.com/tarm/serial"
)

const PORT = "COM7"
const BAUD = 9600
var SerialPort = InitPort(PORT, BAUD)
var PacketReceiverChannel = make(chan string, 100)

func InitPort(port string, baud int) (SerialPort *serial.Port) {
	c := &serial.Config{Name: port, Baud: baud}
	SerialPort, err := serial.OpenPort(c)
	if err != nil {
		log.Fatal(err)
	}
	return
}

func ReceivePacket() {
	fmt.Println("FWJAF")
	scanner := bufio.NewScanner(SerialPort)
	for scanner.Scan() {
		data := scanner.Text()
		if err := scanner.Err(); err != nil {
			log.Fatal(err)
		}
		fmt.Println(data)
		PacketReceiverChannel <- data
	}
}

func SendPacket(data string) {
	_, err := SerialPort.Write([]byte(data))
	if err != nil {
		log.Println(err)
		return
	}
}