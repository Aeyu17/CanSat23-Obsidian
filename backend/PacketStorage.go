package backend

import (
	"container/list"
	"strings"
	"time"
	"fmt"
)

var PacketList *list.List = new(list.List)

func PacketReceiver(c chan string) {
	go ReceivePacket()
	for {
		var packet string;
		if Mode == "flight" || Mode == "sim" {
			packet = <- PacketReceiverChannel
			fmt.Println("Wahoo!")
			if strings.Split(packet, ",")[0] == "1070" {
				c <- packet
				fmt.Println("Packet Received")
			}
		} else if Mode == "gen" {
			time.Sleep(time.Second)
			packet = GeneratePacket()
			c <- packet
		} else {
			continue
		}
	}
}

func PacketEnqueuer(c chan string, l *list.List) {
	for {
		packet := <- c
		var packetArr = strings.Split(packet, ",")
		if packetArr[0] != "1070" || !(len(packetArr) == 2 || len(packetArr) == 20) {
			fmt.Println("Bad packet received.")
			continue
		}
		l.PushBack(packet)
		fmt.Println("Packet queued.")
	}
}

func GetDataPacket(l *list.List) (packet string) {
	// packet format:
	// 	TEAM_ID, MISSION_TIME, PACKET_COUNT, MODE, STATE, ALTITUDE, HS_DEPLOYED, PC_DEPLOYED, MAST_RAISED, TEMPERATURE,
	// 		PRESSURE, VOLTAGE, GPS_TIME, GPS_ALTITUDE, GPS_LATITUDE, GPS_LONGITUDE, GPS_SATS, TILT_X, TILT_Y, CMD_ECHO

	for item := l.Front(); item != nil; item = item.Next() {
		packet = item.Value.(string)
		packetArr := strings.Split(packet, ",")
		if len(packetArr) == 20 {
			l.Remove(item)
			return
		}
	}
	return "Empty"
}

func GetPingPacket(l *list.List) (packet string) {
	// packet format:
	// 	TEAM_ID, PING
	for item := l.Front(); item != nil; item = item.Next() {
		packet = item.Value.(string)
		packetArr := strings.Split(packet, ",")
		if len(packetArr) == 2 {
			l.Remove(item)
			return
		}
	}
	return "Empty"
}
