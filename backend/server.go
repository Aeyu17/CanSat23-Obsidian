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

var Mode = "none";
var SimActive bool;
var wsChan = make(chan *websocket.Conn)
var wsActive bool;

func ServerWrite(message string) {
	ws := <- wsChan
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
			fmt.Println("Flight mode enabled.")
			SendPacket("CMD,1070,CX,ON\n")

		case "CXOFF":
			fmt.Println("CXOFF CALLED")
			Mode = "none"
			fmt.Println("No mode enabled.")
			SendPacket("CMD,1070,CX,OFF\n")

		case "STGPS":
			fmt.Println("STGPS CALLED")
			SendPacket("CMD,1070,ST,GPS\n")

		case "SIME":
			fmt.Println("SIME CALLED")
			Mode = "sim"
			fmt.Println("Simulation mode enabled.")
			SendPacket("CMD,1070,SIM,ENABLE\n")

		case "SIMD":
			fmt.Println("SIMD CALLED")
			Mode = "none"
			SimActive = false
			fmt.Println("No mode enabled.")
			SendPacket("CMD,1070,SIM,DISABLE\n")

		case "SIMA":
			fmt.Println("SIMA CALLED")
			SimActive = true
			fmt.Println("Simulation mode activated.")
			SendPacket("CMD,1070,SIM,ACTIVATE\n")

		case "CAL":
			fmt.Println("CAL CALLED")
			SendPacket("CMD,1070,CAL\n")

		case "ACTMR":
			fmt.Println("ACTMR CALLED")
			SendPacket("CMD,1070,ACT,MR\n")
			
		case "ACTHS":
			fmt.Println("ACTHS CALLED")
			SendPacket("CMD,1070,ACT,HS\n")

		case "ACTPC":
			fmt.Println("ACTPC CALLED")
			SendPacket("CMD,1070,ACT,PC\n")

		case "ACTAB":
			fmt.Println("ACTAB CALLED")
			SendPacket("CMD,1070,ACT,AB\n")

		case "ACTLED":
			fmt.Println("ACTLED CALLED")
			SendPacket("CMD,1070,ACT,LED\n")

		case "PING":
			fmt.Println("PING CALLED")
			start := time.Now()

			SendPacket("CMD,1070,PING\n")
			timeOut := make(chan time.Time)
			pingChannel := make(chan string, 1)
			flag := true
			go func() {
				pingChannel <- GetPingPacket(PacketList)
			}()
			go func() {
				// goofy ahh
				t := <- time.After(3 * time.Second)
				timeOut <- t
			}()
			for flag {
				select {
				case <- timeOut:
					fmt.Println("Ping timed out. Please check connection.")
					flag = false
				case p := <- pingChannel:
					if p != "Empty" {
						duration := time.Since(start)
						fmt.Println("Pong! " + strconv.FormatInt(duration.Milliseconds(), 5))
						flag = false
					}
				}
			}

	
		case "RESET":
			fmt.Println("RESET CALLED")

		case "GEN":
			Mode = "gen"
			fmt.Println("Generator mode enabled.")

		default:
			if string(p)[0:2] == "ST" {
				fmt.Println("STCUS ACTIVATED")
				SendPacket("CMD,1070,ST," + string(p)[2:] + "\n")
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
	ws, err := upgrader.Upgrade(w, r, nil)
	wsActive = true
	go func() {
		for wsActive {
			wsChan <- ws
		}
	}()
	if err != nil {
		log.Println(err)
	}

	log.Println("Client connected.")
	// err = ws.WriteMessage(1, []byte("Hi Client!"))
	if err != nil {
		log.Println(err)
	}

	reader(ws)

	wsActive = false
	log.Println("Page was closed.")
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