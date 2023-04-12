package main

import (
	"fmt"
	"math"
	"math/rand"
    "time"

	backend "github.com/Aeyu17/CanSat23-Obsidian/backend"
	server "github.com/Aeyu17/CanSat23-Obsidian/server"
)

func main() {
	fmt.Println("Starting Ground Control Station...")
	server.InitServer()
	
	rand.Seed(time.Now().UnixNano())    
	//commented values are default
	TestMarginOfError := 0.5 //0.5
	TestTimeToApogee := 10 //30
	FinalAlt := 700 // 700
	DescentSpeed1 := 50 //20
	DescentSpeed2 := 35 //15
	HeatShieldActivationHeight := 500.0 //500.0
	ParachuteActivationHeight := 200.0 //200.0
	TestAlt := 0.0 //0.0
	AverageTempAtGL := 50.0 //50.0
	Temperature := 0.0 //0.0
	Pressure := 0.0 //0.0
	CurrentTime := 0.0 //0.0
	PreviousTime := CurrentTime //CurrentTime
	PacketCount := 0 //0

	Start := time.Now()

	//starting line for the text file
	backend.WriteToTXT("TEAM_ID, MISSION_TIME, PACKET_COUNT, MODE, STATE, ALTITUDE, HS_DEPLOYED, PS_DEPLOYED, MAST_RAISED, TEMPERATURE, PRESSURE, VOLTAGE, GPS_TIME, GPS_ALTITUDE, GPS_LATITUDE, GPS_LONGITUDE, GPS_SATS, TILT_X, TILT_Y, CMD_ECHO\n\n")

	for !backend.FlightStatus { 
		PreviousTime = CurrentTime
		CurrentTime = float64(time.Since(Start)) //calculates time since the start of the timer
		CurrentTime = math.Round(CurrentTime/10000000)/100 //rounds currentTime to the hundredths place

		TestAlt = backend.GenerateAlt(TestAlt, float64(FinalAlt), float64(TestTimeToApogee), TestMarginOfError, DescentSpeed1, 
					DescentSpeed2, ParachuteActivationHeight, HeatShieldActivationHeight, CurrentTime, PreviousTime)
		Temperature = backend.GenerateTemp(TestAlt, AverageTempAtGL)
		Pressure = backend.GeneratePressure(TestAlt, Temperature)
	
		fmt.Println(fmt.Sprint(CurrentTime))
		PacketCount += 1 

		//Formation of CSV & TXT
		data := "1007," + fmt.Sprint(time.Now().Format("15:04:05.00")) + "," + fmt.Sprint(PacketCount) + "," + 
				"S," + "STATE," + fmt.Sprint(TestAlt) + "m," + fmt.Sprint(backend.HS_DEPLOYED) + "," + 
				fmt.Sprint(backend.PS_DEPLOYED) + "," + fmt.Sprint(backend.MAST_RAISED) + "," + fmt.Sprint(Temperature) + 
				"*F," + fmt.Sprint(Pressure) + "hPa," + "3.3," + "GPS_TIME," + "GPS_ALTITUDE," + 
				"GPS_LATITUDE," + "GPS_LONGITUDE," + "GPS_SATS," + "TILT_X," + "TILT_Y," + "CMD_ECHO" + "\n"
		backend.WriteToCSV(data)
		backend.WriteToTXT(data)

		//XBEE OUT
		//packet := backend.ToPacket(data)
		//length := len(packet)
		//fmt.Println(length) //because the compiler will not let you run the program if there is an unused variable omg its so annoying i want to shit all over googles headquarters, like just make it a warning why does it have to be so complicated i want to run the program without having to comment out or delete the variable PLEASEQ!@@>@>>!>!>!?!?!
		//backend.SendPacket("COM13", packet, length)
		PacketCount+=1

		//delay
		time.Sleep(time.Second)
	}
	fmt.Println("Flight Complete")
}
