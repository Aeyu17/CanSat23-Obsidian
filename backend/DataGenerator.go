package backend

import (
	"math"
	"math/rand"
	"strconv"
	"time"
)

const team_id = "1070"
var packetCount = 0
var mode = "S"
var state = "READY"
var altitude = 0.0
var hs_deployed = "N"
var pc_deployed = "N"
var mast_raised = "N"
var pressure = 1000.0
var gpsLat = 34.82
var gpsLong = -86.64
var gpsSats = 13
var tiltx = 0.0
var tilty = 0.0
const cmd_echo = "CXON"


func GeneratePacket() (packet string) {
	currentTime := strconv.Itoa(time.Now().UTC().Hour()) + ":" + strconv.Itoa(time.Now().UTC().Minute()) + ":" + strconv.Itoa(time.Now().UTC().Second())
	packetCount++
	temperature := 25.0 + rand.Float64() - 0.5
	pressure += rand.Float64()
	voltage := 6.0 + rand.Float64() - 0.5

	switch state {
	case "READY":
		altitude += rand.Float64()
		if altitude >= 5 {
			state = "ASCENDING"
		}

	case "ASCENDING":
		altitude += 30.0 * rand.Float64()
		if altitude >= 700 {
			state = "DESCENDING"
		}

	case "DESCENDING":
		altitude -= 15.0 * rand.Float64()
		if altitude <= 500 {
			state = "HSDEPLOYED"
			hs_deployed = "P"
		}

	case "HSDEPLOYED":
		altitude -= 20.0 * rand.Float64()
		if altitude <= 200 {
			state = "PCDEPLOYED"
			pc_deployed = "C"
		}

	case "PCDEPLOYED":
		altitude -= 5.0 * rand.Float64()
		if altitude <= 1 {
			state = "LANDED"
			mast_raised = "M"
		}

	case "LANDED":
		altitude = 0.25 * rand.Float64() - 0.125

	}
	altitude = math.Round(altitude*10)/10

	gpsTime := currentTime
	gpsAlt := altitude
	gpsLat -= 0.01 * rand.Float64()
	gpsLong += 0.01 * rand.Float64()

	tiltx += 0.1 * rand.Float64() - 0.05
	tilty += 0.1 * rand.Float64() - 0.05

	packet = team_id + "," + currentTime + "," + strconv.Itoa(packetCount) + "," + mode + "," + state + "," + strconv.FormatFloat(altitude, 'f', -1, 64) + "," + hs_deployed + "," + pc_deployed + "," + mast_raised + "," + strconv.FormatFloat(temperature, 'f', -1, 64) + "," + strconv.FormatFloat(pressure, 'f', -1, 64) + "," + strconv.FormatFloat(voltage, 'f', -1, 64) + "," + gpsTime + "," + strconv.FormatFloat(gpsAlt, 'f', -1, 64) + "," + strconv.FormatFloat(gpsLat, 'f', -1, 64) + "," + strconv.FormatFloat(gpsLong, 'f', -1, 64) + "," + strconv.Itoa(gpsSats) + "," + strconv.FormatFloat(tiltx, 'f', -1, 64) + "," + strconv.FormatFloat(tilty, 'f', -1, 64) + "," + cmd_echo + "\n"
	return packet
}