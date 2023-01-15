package backend

import (
	"math"
	"math/rand"
	/*
	   used by the example
	   "fmt"
	   "time"
	*/)

// generates an altitude based on the parameters given
func generateCleanAlt(finalAlt float64, timeToApogee float64, currentTime float64) (cleanAlt float64) {
	//formula for a parabola with defined x-ints and apogee
	//thx Khan Academy for making something easy for me to test with *https://www.khanacademy.org/computer-programming/approximating-a-parabola/5515380745306112*
	cleanAlt = (math.Round(((math.Pow(currentTime-timeToApogee, 2))/(-(timeToApogee)/(finalAlt/timeToApogee)))+finalAlt) * 100) / 100
	return
}

// takes the clean altitude generated and adds or substracts the random margin of error
func GenerateRandAlt(marginOfError float64, timeToApogee int, finalAlt int, currentTime float64) (randAlt float64) {
	randMargin := rand.Float64() * marginOfError
	randMargin = math.Round(randMargin*100) / 100

	cleanAlt := generateCleanAlt(float64(finalAlt), float64(timeToApogee), currentTime)

	if rand.Intn(2) == 0 {
		randAlt = cleanAlt - randMargin
	} else {
		randAlt = cleanAlt + randMargin
	}

	if randAlt < 0 {
		randAlt = 0
	}
	return
}

// generates a altitude from a certain descent speed
func GenerateDescent(testAlt float64, descentSpeed1 int, descentSpeed2 int, currentTime float64, previousTime float64) (position float64) {
	if testAlt >= 200 {
		position = (math.Round(((-(float64(descentSpeed1) * (currentTime - previousTime))) + testAlt) * 100)) / 100
	} else {
		position = (math.Round(((-(float64(descentSpeed2) * (currentTime - previousTime))) + testAlt) * 100)) / 100
	}
	if position < 0 {
		position = 0
		flightStatus := true
	}
	return
}

/* example for use
func main(){
    rand.Seed(time.Now().UnixNano())
    TestMarginOfError := 0.15
    TestTimeToApogee := 15
    FinalAlt := 700
    DescentSpeed1 := 20
    DescentSpeed2 := 15
    ParachuteActivationHeight := 500.0
    TestAlt := 0.0
    CurrentTime := 0.0

    Start := time.Now()

    for FlightStatus != true{
        PreviousTime := CurrentTime
		CurrentTime = float64(time.Since(Start)) //calculates time since the start of the timer
        CurrentTime = (math.Round(CurrentTime/10000000))/100 //rounds currentTime to the hundredths place

        if(CurrentTime < float64(TestTimeToApogee) || TestAlt > ParachuteActivationHeight){
            TestAlt = GenerateRandAlt(TestMarginOfError, TestTimeToApogee, FinalAlt, CurrentTime)
        } else {
            TestAlt = GenerateDescent(TestAlt, DescentSpeed1, DescentSpeed2, CurrentTime, PreviousTime)
        }
        fmt.Println(fmt.Sprint(CurrentTime))

        WriteToCSV(fmt.Sprint(CurrentTime) + "s   :   " + fmt.Sprint(TestAlt) + "m\n")
        time.Sleep(time.Second/3) //delay
    }
    fmt.Println("Flight Complete")
}
*/
