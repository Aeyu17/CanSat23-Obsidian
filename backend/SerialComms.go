package backend

import (
	"log"
	"serial-master"
)

func GetPacket(port string) (data int) {
	c := &serial.Config{Name: port, Baud: 115200}
	s, err := serial.OpenPort(c)
	if err != nil {
		log.Fatal(err)
	}
	buf := make([]byte, 128)
	data, err = s.Read(buf)
	if err != nil {
		log.Print("Could not read | Error message: ")
		log.Println(err)
	}
	log.Printf("%q", buf[:data])
	s.Close()
	return
}

func SendPacket(port string, data []byte, length int) {
	c := &serial.Config{Name: port, Baud: 115200}
	s, err := serial.OpenPort(c)
	if err != nil {
		log.Print("Could not write | Error message: ")
		log.Println(err)
	}


	//send start byte
	n, err := s.Write(data)
	if err != nil {
		log.Fatal(err)
	}
	buf := make([]byte, 128)
	log.Printf("%q", buf[:n])
	//send end byte

	s.Close()
}


func ToPacket(data string) (packet []byte) {
	packet = []byte(data)
	return
}

// while(1) {
// 	while(w32.GetKeyState(w32.VK_LEFT)&0x8000 != 0) {
// 		n, err := s.Write([]byte("O"))
// 		if err != nil {
// 			log.Fatal(err)
// 		}
// 	}
// 	while(w32.GetKeyState(w32.VK_RIGHT)&0x8000 != 0) {
// 		n, err := s.Write([]byte("I"))
// 		if err != nil {
// 			log.Fatal(err)
// 		}
// 	}
// 	if(w32.GetKeyState("Q")&0x8000 != 0) {
// 		break;
// 	}
// }
// serial.ClosePort(c);