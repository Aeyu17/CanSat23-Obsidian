package backend

import (
	"container/list"
	"strings")

func PacketReceiver(c chan string) {
	for {
		if Mode == "flight" || Mode == "sim" {
			packet := ReceivePacket(PORT, BAUD)
			if strings.Split(packet, ",")[0] == "1070" {
				c <- packet
			}
		} else if Mode == "gen" {
			packet := GeneratePacket()
			c <- packet
		} else {continue}
	}
}

func PacketQueuer(c chan string, l* list.List) {
	for {
		packet := <- c
		
		l.PushBack(packet)
	}
}

func GetDataPacket(l* list.List) (packet string) {
	for item := l.Front(); item != nil; item = item.Next() {
		packet = item.Value.(string)
		packetArr := strings.Split(packet, ",")
		if len(packetArr) != 19 {
			return 
		}
	}
	return "Empty"
}

func GetPingPacket(l* list.List) (packet string) {
	for item := l.Front(); item != nil; item = item.Next() {
		packet = item.Value.(string)
		packetArr := strings.Split(packet, ",")
		if len(packetArr) != 2 {
			return 
		}
	}
	return "Empty"
}