package backend

import (
	"log"
	"fmt"
	"bufio"
	"time"

	"github.com/tarm/serial"
	"go.bug.st/serial/enumerator"
)
 
const BAUD = 9600
var (
	Port string;
	SerialPort = InitPort(BAUD)
	PacketReceiverChannel = make(chan string, 100)
	SerialConnected = true
	serialSem = make(chan int, 1)
	SerialReader = bufio.NewReaderSize(SerialPort, 1024)

)

func InitPort(baud int) (SerialPort *serial.Port) {
	serialSem <- 1
	portList, err := enumerator.GetDetailedPortsList()
	if err != nil {
		log.Fatal(err)
	}

	if len(portList) == 0 {
		fmt.Println("No COM ports found.")
		SerialConnected = false
		<- serialSem
		return nil
	}
	/*
	fmt.Println("Port Name:", portList[0].Name)
	fmt.Println("Is USB:", portList[0].IsUSB)
	fmt.Println("VID:", portList[0].VID)
	fmt.Println("PID:", portList[0].PID)
	fmt.Println("Serial Number:", portList[0].SerialNumber)
	fmt.Println("Product:", portList[0].Product)
	*/

	for _, p := range portList {
		if p.IsUSB && p.PID == "6015" && p.VID == "0403" {
			Port = p.Name
			break
		}
	}
	if Port == "" {
		fmt.Println("No valid COM ports found.")
		SerialConnected = false
		<- serialSem
		return nil
	}

	if SerialPort != nil {
		SerialConnected = true
		<- serialSem
		return SerialPort
	}
	
	fmt.Println("Connecting to", Port)
	c := &serial.Config{Name: Port, Baud: baud}
	SerialPort, err = serial.OpenPort(c)
	if err != nil {
		log.Println(err)
		SerialConnected = false
		<- serialSem
		return nil
	}
	fmt.Println("Connected to", Port)
	SerialConnected = true
	<- serialSem
	return
}

func ReceivePacket() {
	for {
		if !SerialConnected {
			fmt.Println("XBee not connected.")
			time.Sleep(time.Second/4)
		} else {
			data, err := SerialReader.ReadString(10)
			if err != nil {
				serialSem <- 2
				if err.Error() == "Invalid port on read" {
					SerialReader = bufio.NewReaderSize(SerialPort, 1024)
					log.Println("Reconnecting SerialReader...")
				} else {
					log.Println(err)
					SerialConnected = false
				}
				<- serialSem
			}
			fmt.Println(data)
			PacketReceiverChannel <- data
		}
	}
}

func SendPacket(data string) {
	if !SerialConnected {
		fmt.Println("XBee not connected.")
		return
	}
	serialSem <- 3
	fmt.Println("Sending: " + data)
	_, err := SerialPort.Write([]byte(data))
	if err != nil {
		log.Println(err)
		<- serialSem
		return
	}
	time.Sleep(time.Second)
	<- serialSem
}