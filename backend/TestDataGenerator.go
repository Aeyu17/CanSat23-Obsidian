package backend
import (
    "fmt"
    "math"

    "log"
    "os"
)

var FlightStatus = false
var HS_DEPLOYED = "N"
var PS_DEPLOYED = "N"
var MAST_RAISED = "N"

//rounds an input to the specified number of decimal places
func roundTo(input float64, places int) (output float64){
    output = math.Round(input * math.Pow((10), float64(places))) / math.Pow(10, float64(places))
    return output
}

//generates an altitude
func generateAlt(alt float64, finalAlt float64, timeToApogee float64, marginOfError float64, descentSpeed1 int, descentSpeed2 int, 
    parachuteActivationHeight float64, heatShieldActivationHeight float64, currentTime float64, previousTime float64) (Alt float64){

    //formula for a parabola with defined x-ints
    //thx Khan Academy for making something easy for me to test with *https://www.khanacademy.org/computer-programming/approximating-a-parabola/5515380745306112*
    if(currentTime < float64(timeToApogee) || alt > heatShieldActivationHeight){
        Alt = (((math.Pow(currentTime-timeToApogee, 2)) / ( -(timeToApogee) / (finalAlt/timeToApogee))) + finalAlt)

    } else {
        if Alt >= parachuteActivationHeight {
            HS_DEPLOYED = "P"
            Alt = (( -(float64(descentSpeed1) * (currentTime-previousTime))) + alt)
        } else {
            PS_DEPLOYED = "C"
            Alt = (( -(float64(descentSpeed2) * (currentTime-previousTime))) + alt)
        }
    }
    
    //creates the random margin and then adds/subtracts it from a clean generated altitude 
    randMargin := (rand.Float64()*marginOfError)
    randMargin = roundTo(randMargin, 1)

    if(rand.Intn(2) == 0){
        Alt = Alt - randMargin
        Alt = roundTo(Alt, 1)
    } else {
        Alt = Alt + randMargin
        Alt = roundTo(Alt, 1)
    }
    
    if(currentTime > float64(timeToApogee) && Alt < 0){
        Alt = 0
        MAST_RAISED = "M"
        FlightStatus = true
    }
    return
}

//generates an temperature using the temperature lapse rate for a low altitude
func generateTemp(alt float64, currentTemp float64) (temp float64){
    //finds temp in fahrenheit
    temp = currentTemp - ((alt/1000) * 6.5)
    //f to c
    temp = (temp - 32) * 9/5
    temp = roundTo(temp, 1)
    return
}

//generates pressure using altitude and temperature
func generatePressure(alt float64, temp float64) (pressure float64){
    //Formula obtained from https://www.mide.com/air-pressure-at-altitude-calculator
    //Equation 2 was used as it was for altitudes below 11km and was altered for finding pressure
    pressure = 101900 * math.Pow((1 + (-0.0065 / temp + 273.15) * alt), (-9.80665 * 0.0289644) / (8.31432 * -0.0065))
    pressure = roundTo(pressure/100, 1)
    return
}

func generateVoltage() (Volts float64){
    return
}

//example for use/test
// func main(){
//     rand.Seed(time.Now().UnixNano())    
//     TestMarginOfError := 0.5 //0.5
//     TestTimeToApogee := 10 //30
//     FinalAlt := 700 // 700
//     DescentSpeed1 := 50 //20
//     DescentSpeed2 := 35 //15
//     HeatShieldActivationHeight := 500.0 //500.0
//     ParachuteActivationHeight := 200.0 //200.0
//     TestAlt := 0.0 //0.0
//     AverageTempAtGL := 50.0 //50.0
//     Temperature := 0.0 //0.0
//     Pressure := 0.0 //0.0
//     CurrentTime := 0.0 //0.0
//     PreviousTime := CurrentTime //CurrentTime
//     PacketCount := 0 //0

//     Start := time.Now()
//     //use the time.Since() wherever you repeat steps

//     //starting line for the text file
//     WriteToTXT("TEAM_ID, MISSION_TIME, PACKET_COUNT, MODE, STATE, ALTITUDE, HS_DEPLOYED, PS_DEPLOYED, MAST_RAISED, TEMPERATURE, PRESSURE, VOLTAGE, GPS_TIME, GPS_ALTITUDE, GPS_LATITUDE, GPS_LONGITUDE, GPS_SATS, TILT_X, TILT_Y, CMD_ECHO\n\n")

//     for FlightStatus != true{ 
//         PreviousTime = CurrentTime
// 		CurrentTime = float64(time.Since(Start)) //calculates time since the start of the timer
//         CurrentTime = math.Round(CurrentTime/10000000)/100 //rounds currentTime to the hundredths place

//         TestAlt = generateAlt(TestAlt, float64(FinalAlt), float64(TestTimeToApogee), TestMarginOfError, DescentSpeed1, 
//                     DescentSpeed2, ParachuteActivationHeight, HeatShieldActivationHeight, CurrentTime, PreviousTime)
//         Temperature = generateTemp(TestAlt, AverageTempAtGL)
//         Pressure = generatePressure(TestAlt, Temperature)
        
//         fmt.Println(fmt.Sprint(CurrentTime))
//         PacketCount += 1 

//         //Formation of CSV
//         WriteToCSV(fmt.Sprint(CurrentTime) + "s   :   " + fmt.Sprint(TestAlt) + "m\n")
//         WriteToTXT("1007," + fmt.Sprint(time.Now().Format("15:04:05.00")) + "," + fmt.Sprint(PacketCount) + "," + "S," + "STATE," + fmt.Sprint(TestAlt) + "m," + fmt.Sprint(HS_DEPLOYED) + "," + fmt.Sprint(PS_DEPLOYED) + "," + fmt.Sprint(MAST_RAISED) + "," + fmt.Sprint(Temperature) + "*F," + fmt.Sprint(Pressure) + "hPa," + "3.3," + "GPS_TIME," + "GPS_ALTITUDE," + "GPS_LATITUDE," + "GPS_LONGITUDE," + "GPS_SATS," + "TILT_X," + "TILT_Y," + "CMD_ECHO" + "\n")

//         //delay
//         time.Sleep(time.Second/3)
//     }
//     fmt.Println("Flight Complete")
// }