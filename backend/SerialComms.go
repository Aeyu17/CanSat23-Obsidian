package main

import (
	"log"
	"serial-master"
	"w32"
)

// func GetPacket() {
// 	c := &serial.Config{Name: "COM7", Baud: 115200}
// 	s, err := serial.OpenPort(c)
// 	if err != nil {
// 		log.Fatal(err)
// 	}
// 	n, err := s.Write([]byte("test"))
// 	if err != nil {
// 		log.Fatal(err)
// 	}
// 	buf := make([]byte, 128)
// 	n, err = s.Read(buf)
// 	if err != nil {
// 		log.Fatal(err)
// 	}
// 	log.Print("%q", buf[:n])
// }

func main() {
	c := &serial.Config{Name: "COM7", Baud: 115200}
	s, err := serial.OpenPort(c)
	if err != nil {
		log.Fatal(err)
	}
	n, err := s.Write([]byte("test"))
	if err != nil {
		log.Fatal(err)
	}
	buf := make([]byte, 128)
	n, err = s.Read(buf)
	if err != nil {
		log.Fatal(err)
	}
	log.Print("%q", buf[:n])

	while(1) {
		while(w32.GetKeyState(w32.VK_LEFT)&0x8000 != 0) {
			n, err := s.Write([]byte("O"))
			if err != nil {
				log.Fatal(err)
			}
		}
		while(w32.GetKeyState(w32.VK_RIGHT)&0x8000 != 0) {
		    n, err := s.Write([]byte("I"))
			if err != nil {
				log.Fatal(err)
			}
		}
		if(w32.GetKeyState("Q")&0x8000 != 0) {
		    break;
		}
	}
	serial.ClosePort(c);
}