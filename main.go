package main

import (
	"fmt"

	backend "github.com/Aeyu17/CanSat23-Obsidian/backend"
	server "github.com/Aeyu17/CanSat23-Obsidian/server"
)

func main() {
	fmt.Println("Starting Ground Control Station...")
	server.InitServer()

	backend.WriteToCSV("hi")
	
	
    	rand.Seed(time.Now().UnixNano())    
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
    	//use the time.Since() wherever you repeat steps

    	//starting line for the text file
    	WriteToTXT("TEAM_ID, MISSION_TIME, PACKET_COUNT, MODE, STATE, ALTITUDE, HS_DEPLOYED, PS_DEPLOYED, MAST_RAISED, TEMPERATURE, PRESSURE, VOLTAGE, GPS_TIME, GPS_ALTITUDE, GPS_LATITUDE, GPS_LONGITUDE, GPS_SATS, TILT_X, TILT_Y, CMD_ECHO\n\n")

    	for FlightStatus != true{ 
        	PreviousTime = CurrentTime
			CurrentTime = float64(time.Since(Start)) //calculates time since the start of the timer
        	CurrentTime = math.Round(CurrentTime/10000000)/100 //rounds currentTime to the hundredths place

        	TestAlt = generateAlt(TestAlt, float64(FinalAlt), float64(TestTimeToApogee), TestMarginOfError, DescentSpeed1, 
                    	DescentSpeed2, ParachuteActivationHeight, HeatShieldActivationHeight, CurrentTime, PreviousTime)
        	Temperature = generateTemp(TestAlt, AverageTempAtGL)
        	Pressure = generatePressure(TestAlt, Temperature)
        
        	fmt.Println(fmt.Sprint(CurrentTime))
        	PacketCount += 1 

        	//Formation of CSV
        	WriteToCSV(fmt.Sprint(CurrentTime) + "s   :   " + fmt.Sprint(TestAlt) + "m\n")
        	WriteToTXT("1007," + fmt.Sprint(time.Now().Format("15:04:05.00")) + "," + fmt.Sprint(PacketCount) + "," + "S," + "STATE," + fmt.Sprint(TestAlt) + "m," + fmt.Sprint(HS_DEPLOYED) + "," + fmt.Sprint(PS_DEPLOYED) + "," + fmt.Sprint(MAST_RAISED) + "," + fmt.Sprint(Temperature) + "*F," + fmt.Sprint(Pressure) + "hPa," + "3.3," + "GPS_TIME," + "GPS_ALTITUDE," + "GPS_LATITUDE," + "GPS_LONGITUDE," + "GPS_SATS," + "TILT_X," + "TILT_Y," + "CMD_ECHO" + "\n")

        	//delay
        	time.Sleep(time.Second/3)
    	}
    	fmt.Println("Flight Complete")
}
