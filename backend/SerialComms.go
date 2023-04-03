// package backend

// import (
// 	"log"
// 	"serial-master"
// 	"strings"
// )

// func GetPacket(port string, baud int) (data string) {
// 	c := &serial.Config{Name: port, Baud: 9600}
// 	s, err := serial.OpenPort(c)
// 	if err != nil {
// 		log.Fatal(err)
// 	}
// 	buf := make([]byte, 128)
// 	for {
// 		n, err := s.Read(buf)
// 		if err != nil {
// 			log.Fatal(err)
// 		}
// 		// If we receive a newline stop reading
// 		if strings.Contains(string(buf[:n]), "\n") {
// 			data = string(buf[:n])
// 			break
// 		}
// 	}
// 	s.Close()
// 	return
// }

// func SendPacket(port string, baud int,  data string) {
// 	c := &serial.Config{Name: port, Baud: baud}
// 	s, err := serial.OpenPort(c)
// 	if err != nil {
// 		log.Fatal(err)
// 	}
// 	n, err := s.Write([]byte(data))
// 	if err != nil {
// 		log.Fatal(err)
// 		if (n == 1) {} // unused variable I am currently unsure how to handle
// 	}
// 	s.Close()
// }