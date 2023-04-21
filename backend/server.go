package backend

import (
	"embed"
	"fmt"
	"io/fs"
	"log"
	"net/http"
	"time"
	"strconv"

	"github.com/gorilla/websocket"
)

//go:embed static/*
var staticFiles embed.FS
var upgrader = websocket.Upgrader{
	ReadBufferSize:		1024,
	WriteBufferSize:	1024,
	CheckOrigin:		func (r *http.Request) bool {return true},
}

var ws *websocket.Conn;
var Mode string;
var SimActive bool;

func ServerWrite(message string) {
	if err := ws.WriteMessage(1, []byte(message)); err != nil {
		log.Println(err)
	}
}

func reader(conn *websocket.Conn) {
	for {
		messageType, p, err := conn.ReadMessage()
		if err != nil {
			log.Println(err)
			return
		}

		log.Println(string(p))

		switch string(p) {
		case "CXON":
			fmt.Println("CXON CALLED")
			Mode = "flight"
			SendPacket(PORT, BAUD, "CMD,1070,CX,ON")

		case "CXOFF":
			fmt.Println("CXOFF CALLED")
			Mode = "none"
			SendPacket(PORT, BAUD, "CMD,1070,CX,OFF")

		case "STGPS":
			fmt.Println("STGPS CALLED")
			SendPacket(PORT, BAUD, "CMD,1070,ST,GPS")

		case "SIME":
			fmt.Println("SIME CALLED")
			Mode = "sim"
			SendPacket(PORT, BAUD, "CMD,1070,SIM,ENABLE")

		case "SIMD":
			fmt.Println("SIMD CALLED")
			Mode = "flight"
			SimActive = false
			SendPacket(PORT, BAUD, "CMD,1070,SIM,DISABLE")

		case "SIMA":
			fmt.Println("SIMA CALLED")
			SimActive = true
			SendPacket(PORT, BAUD, "CMD,1070,SIM,ACTIVATE")

		case "CAL":
			fmt.Println("CAL CALLED")
			SendPacket(PORT, BAUD, "CMD,1070,CAL")

		case "ACTMR":
			fmt.Println("ACTMR CALLED")
			SendPacket(PORT, BAUD, "CMD,1070,ACT,MR")
			
		case "ACTHS":
			fmt.Println("ACTHS CALLED")
			SendPacket(PORT, BAUD, "CMD,1070,ACT,HS")

		case "ACTPC":
			fmt.Println("ACTPC CALLED")
			SendPacket(PORT, BAUD, "CMD,1070,ACT,PC")

		case "ACTAB":
			fmt.Println("ACTAB CALLED")
			SendPacket(PORT, BAUD, "CMD,1070,ACT,AB")

		case "ACTLED":
			fmt.Println("ACTLED CALLED")
			SendPacket(PORT, BAUD, "CMD,1070,ACT,LED")

		case "PING":
			fmt.Println("PING CALLED")
			start := time.Now()

			SendPacket(PORT, BAUD, "CMD,1070,PING")
			GetPingPacket(PacketList)

			duration := time.Since(start)
			fmt.Println("Pong! " + strconv.FormatInt(duration.Milliseconds(), 5))

		case "RESET":
			fmt.Println("RESET CALLED")

		case "GEN":
			Mode = "gen"

		default:
			if string(p)[0:2] == "ST" {
				fmt.Println("STCUS ACTIVATED")
			} else {
				fmt.Println("OTHER MESSAGE RECEIVED")
			}
		}

		if err := conn.WriteMessage(messageType, p); err != nil {
			log.Println(err)
			return
		}
	}
}

func wsEndpoint(w http.ResponseWriter, r *http.Request) {
	ws1, err := upgrader.Upgrade(w, r, nil)
	ws = ws1
	if err != nil {
		log.Println(err)
	}

	log.Println("Client connected.")
	err = ws.WriteMessage(1, []byte("Hi Client!"))
	if err != nil {
		log.Println(err)
	}

	reader(ws)
}

func InitServer() {
	staticContent, err := fs.Sub(staticFiles, "static")
	if err != nil {
		panic(err)
	}

	fileServer := http.FileServer(http.FS(staticContent))
	http.Handle("/", fileServer)
	http.HandleFunc("/ws", wsEndpoint)

	fmt.Println("Starting server at port 8080")
	if err := http.ListenAndServe(":8080", nil); err != nil {
		log.Fatal(err)
	}
}