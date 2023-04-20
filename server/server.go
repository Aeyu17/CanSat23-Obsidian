package server

import (
	"embed"
	"fmt"
	"io/fs"
	"log"
	"net/http"

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
			fmt.Println("CXON ACTIVATED")
		case "CXOFF":
			fmt.Println("CXOFF ACTIVATED")
		case "STGPS":
			fmt.Println("STGPS ACTIVATED")
		case "SIME":
			fmt.Println("SIME ACTIVATED")
		case "SIMD":
			fmt.Println("SIMD ACTIVATED")
		case "SIMA":
			fmt.Println("SIMA ACTIVATED")
		case "CAL":
			fmt.Println("CAL ACTIVATED")
		case "ACTMR":
			fmt.Println("ACTMR ACTIVATED")
		case "ACTHS":
			fmt.Println("ACTHS ACTIVATED")
		case "ACTPC":
			fmt.Println("ACTPC ACTIVATED")
		case "ACTAB":
			fmt.Println("ACTAB ACTIVATED")
		case "ACTLED":
			fmt.Println("ACTLED ACTIVATED")
		case "PING":
			fmt.Println("PING ACTIVATED")
		case "RESET":
			fmt.Println("RESET ACTIVATED")
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


