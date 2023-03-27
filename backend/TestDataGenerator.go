package backend
import (
    "math"
    "math/rand"
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
func GenerateAlt(alt float64, finalAlt float64, timeToApogee float64, marginOfError float64, descentSpeed1 int, descentSpeed2 int, 
    parachuteActivationHeight float64, heatShieldActivationHeight float64, currentTime float64, previousTime float64) (Alt float64){

    //formula for a parabola with defined x-ints
    //*https://www.khanacademy.org/computer-programming/approximating-a-parabola/5515380745306112*
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
func GenerateTemp(alt float64, currentTemp float64) (temp float64){
    //finds temp in fahrenheit
    temp = currentTemp - ((alt/1000) * 6.5)
    //f to c
    temp = (temp - 32) * 9/5
    temp = roundTo(temp, 1)
    return
}

//generates pressure using altitude and temperature
func GeneratePressure(alt float64, temp float64) (pressure float64){
    //Formula obtained from https://www.mide.com/air-pressure-at-altitude-calculator
    //Equation 2 was used as it was for altitudes below 11km and was altered for finding pressure
    pressure = 101900 * math.Pow((1 + (-0.0065 / temp + 273.15) * alt), (-9.80665 * 0.0289644) / (8.31432 * -0.0065))
    pressure = roundTo(pressure/100, 1)
    return
}

func GenerateVoltage() (Volts float64){
    return
}