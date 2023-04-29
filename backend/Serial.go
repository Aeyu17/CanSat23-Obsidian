package backend

import (
	"log"
	"strings"

	"github.com/tarm/serial"
)

const PORT = "COM6"
const BAUD = 9600
var SerialPort = InitPort(PORT, BAUD)

func InitPort(port string, baud int) (SerialPort *serial.Port) {
	c := &serial.Config{Name: port, Baud: baud}
	SerialPort, err := serial.OpenPort(c)
	if err != nil {
		log.Fatal(err)
	}
	return
}

func ReceivePacket() (data string) {
	buf := make([]byte, 128)
	for {
		n, err := SerialPort.Read(buf)
		log.Println("hhhh")
		if err != nil {
			log.Println(err)
			return
		}
		// If we receive a newline stop reading
		if strings.Contains(string(buf[:n]), "\n") {
			data = string(buf[:n])
			break
		}
	}
	return
}

func SendPacket(data string) {
	_, err := SerialPort.Write([]byte(data))
	if err != nil {
		log.Println(err)
		return
	}
}