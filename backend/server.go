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
			if Mode != "none" {
				fmt.Println("CXON ignored, turn off current mode.")
				continue
			}

			Mode = "flight"
			fmt.Println("Flight mode enabled.")
			SendPacket("CMD,1070,CX,ON\n")

		case "CXOFF":
			fmt.Println("CXOFF CALLED")
			if Mode != "flight" {
				fmt.Println("CXOFF ignored, turn on flight mode.")
				continue
			}

			Mode = "none"
			fmt.Println("No mode enabled.")
			SendPacket("CMD,1070,CX,OFF\n")

		case "STGPS":
			fmt.Println("STGPS CALLED")
			if !(Mode == "flight" || Mode == "sim") {
				fmt.Println("STGPS ignored, turn on flight or sim mode.")
				continue
			}

			SendPacket("CMD,1070,ST,GPS\n")

		case "SIME":
			fmt.Println("SIME CALLED")
			if Mode != "none" {
				fmt.Println("SIME ignored, turn off current mode.")
				continue
			}

			Mode = "sim"
			fmt.Println("Simulation mode enabled.")
			SendPacket("CMD,1070,SIM,ENABLE\n")

		case "SIMD":
			fmt.Println("SIMD CALLED")
			if Mode != "sim" {
				fmt.Println("SIMD ignored, turn on simulation mode.")
				continue
			}

			Mode = "none"
			SimActive = false
			lineIndex = 0
			fmt.Println("No mode enabled.")
			SendPacket("CMD,1070,SIM,DISABLE\n")

		case "SIMA":
			fmt.Println("SIMA CALLED")
			if Mode != "sim" {
				fmt.Println("SIMA ignored, turn on simulation mode.")
				continue
			}

			SimActive = true
			fmt.Println("Simulation mode activated.")
			SendPacket("CMD,1070,SIM,ACTIVATE\n")
			time.Sleep(time.Second*5)

		case "CAL":
			fmt.Println("CAL CALLED")
			if !(Mode == "flight" || Mode == "sim") {
				fmt.Println("CAL ignored, turn on flight or sim mode.")
				continue
			}

			SendPacket("CMD,1070,CAL\n")

		case "ACTFL0":
			fmt.Println("ACTFL0 CALLED")
			// if !(Mode == "flight" || Mode == "sim") {
			// 	fmt.Println("ACTFL0 ignored, turn on flight or sim mode.")
			// 	continue
			// }

			SendPacket("CMD,1070,ACT,FL0\n")

		case "ACTFL1":
			fmt.Println("ACTFL1 CALLED")
			// if !(Mode == "flight" || Mode == "sim") {
			// 	fmt.Println("ACTFL1 ignored, turn on flight or sim mode.")
			// 	continue
			// }

			SendPacket("CMD,1070,ACT,FL1\n")

		case "ACTRL0":
			fmt.Println("ACTRL0 CALLED")
			// if !(Mode == "flight" || Mode == "sim") {
			// 	fmt.Println("ACTRL0 ignored, turn on flight or sim mode.")
			// 	continue
			// }

			SendPacket("CMD,1070,ACT,RL0\n")

		case "ACTRL1":
			fmt.Println("ACTRL1 CALLED")
			// if !(Mode == "flight" || Mode == "sim") {
			// 	fmt.Println("ACTRL1 ignored, turn on flight or sim mode.")
			// 	continue
			// }

			SendPacket("CMD,1070,ACT,RL1\n")

		case "ACTRL2":
			fmt.Println("ACTRL2 CALLED")
			// if !(Mode == "flight" || Mode == "sim") {
			// 	fmt.Println("ACTRL2 ignored, turn on flight or sim mode.")
			// 	continue
			// }

			SendPacket("CMD,1070,ACT,RL2\n")
		
		case "ACTHS0":
			fmt.Println("ACTHS0 CALLED")
			// if !(Mode == "flight" || Mode == "sim") {
			// 	fmt.Println("ACTHS0 ignored, turn on flight or sim mode.")
			// 	continue
			// }

			SendPacket("CMD,1070,ACT,HS0\n")

		case "ACTHS1":
			fmt.Println("ACTHS1 CALLED")
			// if !(Mode == "flight" || Mode == "sim") {
			// 	fmt.Println("ACTHS1 ignored, turn on flight or sim mode.")
			// 	continue
			// }

			SendPacket("CMD,1070,ACT,HS1\n")

		case "ACTHS2":
			fmt.Println("ACTHS2 CALLED")
			// if !(Mode == "flight" || Mode == "sim") {
			// 	fmt.Println("ACTHS2 ignored, turn on flight or sim mode.")
			// 	continue
			// }

			SendPacket("CMD,1070,ACT,HS2\n")

		case "ACTRES":
			fmt.Println("ACTRES CALLED")
			// if !(Mode == "flight" || Mode == "sim") {
			// 	fmt.Println("ACTRES ignored, turn on flight or sim mode.")
			// 	continue
			// }

			SendPacket("CMD,1070,ACT,RES\n")

		case "ACTAB":
			fmt.Println("ACTAB CALLED")
			if !(Mode == "flight" || Mode == "sim") {
				fmt.Println("ACTAB ignored, turn on flight or sim mode.")
				continue
			}

			SendPacket("CMD,1070,ACT,AB\n")

		case "ACTLED":
			fmt.Println("ACTLED CALLED")
			if !(Mode == "flight" || Mode == "sim") {
				fmt.Println("ACTLED ignored, turn on flight or sim mode.")
				continue
			}

			SendPacket("CMD,1070,ACT,LED\n")

		case "PING":
			fmt.Println("PING CALLED")
			// if !(Mode == "flight" || Mode == "sim") {
			// 	fmt.Println("PING ignored, turn on flight or sim mode.")
			// 	continue
			// }

			start := time.Now()

			SendPacket("CMD,1070,PING\n")
			timeOut := make(chan time.Time)
			pingChannel := make(chan string, 1)
			flag := true
			go func() {
				p := GetPingPacket(PacketList)
				if (p != "Empty") {
					pingChannel <- p
				}
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
				case <- pingChannel:
					duration := time.Since(start)
					fmt.Println("Pong! " + strconv.FormatInt(duration.Milliseconds(), 5) + "ms")
					flag = false
				}
			}

	
		case "RESET":
			fmt.Println("RESET CALLED")
			Mode = "none"

			packetCount = 0
			mode = "S"
			state = "READY"
			altitude = 0.0
			hs_deployed = "N"
			pc_deployed = "N"
			mast_raised = "N"
			pressure = 1000.0
			gpsLat = 34.82
			gpsLong = -86.64
			gpsSats = 13
			tiltx = 0.0
			tilty = 0.0

			lineIndex = 0

			ClearQueue(PacketList)

		case "GEN":
			fmt.Println("GEN CALLED")
			if Mode != "none" {
				fmt.Println("GEN ignored, turn off current mode.")
				continue
			}
			Mode = "gen"
			fmt.Println("Generator mode enabled.")

		default:
			if string(p)[0:2] == "ST" {
				fmt.Println("STCUS ACTIVATED")
				if !(Mode == "flight" || Mode == "sim") {
					fmt.Println("STCUS ignored, turn on flight or sim mode.")
					continue
				}

				SendPacket("CMD,1070,ST," + string(p)[2:] + "\n")
			} else {
				fmt.Println("OTHER MESSAGE RECEIVED")
				fmt.Println(p)
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