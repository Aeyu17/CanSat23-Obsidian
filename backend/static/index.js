var missionTime = "00:00:00";
var packetCount = -1;
var mode = "F";
var state = "IDLE";
var altitudeArr = [];
var hsDeployed = "N";
var pcDeployed = "N";
var mastRaised = "N";
var temperatureArr = [];
var pressureArr = [];
var voltageArr = [];
var gpsTime = "00:00:00";
var gpsAltitude = 0.0;
var locationArr = [];
var gpsSatellites = -1;
var tiltArr = [];
var cmdEcho = "NONE";

var startTime = "00:00:00";

window.onload = function () {
    var gcsSocket = new WebSocket("ws://localhost:8080/ws");
 
    var dataLength = 500; // number of dataPoints visible at any point

    function updateData(packet) {
        // count is number of times loop runs to generate random dataPoints.
        let packetArr = packet.split(",");

        missionTime = packetArr[1];
        packetCount = parseInt(packetArr[2]);
        mode = packetArr[3];
        state = packetArr[4];
        let alt = parseFloat(packetArr[5]);
        hsDeployed = packetArr[6];
        pcDeployed = packetArr[7];
        mastRaised = packetArr[8];
        let temp = parseFloat(packetArr[9]);
        let pressure = parseFloat(packetArr[10]);
        let volt = parseFloat(packetArr[11]);
        gpsTime = packetArr[12];
        gpsAltitude = parseFloat(packetArr[13]);
        let lat = parseFloat(packetArr[14]);
        let long = parseFloat(packetArr[15]);
        gpsSatellites = parseInt(packetArr[16]);
        let tiltx = parseFloat(packetArr[17]);
        let tilty = parseFloat(packetArr[18]);
        cmdEcho = packetArr[19];

        if (startTime == "00:00:00" && typeof(missionTime) == typeof(" ")) {
            startTime = missionTime;
        }

        let startTimeArr = [];
        let timeArr = [];

        console.log(startTime);
        console.log(typeof(startTime));
        console.log(startTime.split(":"));

        startTime.split(":").forEach(function (e){
            console.log(e);
            startTimeArr.push(parseInt(e));
        });

        missionTime.split(":").forEach(function (e){
            timeArr.push(parseInt(e));
        });

        console.log(startTimeArr);
        console.log(timeArr);

        time = (timeArr[0] - startTimeArr[0])*3600 + (timeArr[1] - startTimeArr[1])*60 + (timeArr[2] - startTimeArr[2]);

        console.log(time);

        altitudeArr.push({
            x: time,
            y: alt
        });
        temperatureArr.push({
            x: time,
            y: temp
        });
        pressureArr.push({
            x: time,
            y: pressure
        });
        tiltArr.push({
            x: tiltx,
            y: tilty
        });
        locationArr.push({
            x: long,
            y: lat
        });
        voltageArr.push({
            x: time,
            y: volt
        });

        console.log(pressureArr);

        if (altitudeArr.length > dataLength) {
            altitudeArr.shift();
        }
        if (temperatureArr.length > dataLength) {
            temperatureArr.shift();
        }
        if (pressureArr.length > dataLength) {
            pressureArr.shift();
        }
        if (tiltArr.length > dataLength) {
            tiltArr.shift();
        }
        if (locationArr.length > dataLength) {
            locationArr.shift();
        }
        if (voltageArr.length > dataLength) {
            voltageArr.shift();
        }
        altChart.render();
        tempChart.render();
        presChart.render();
        tiltChart.render();
        locChart.render();
        voltChart.render();

        // update labels
        document.getElementById("TimeLabel").textContent = "Time: " + missionTime;
        document.getElementById("PacketCount").textContent = "Packet Count: " + packetCount;
        document.getElementById("Mode").textContent = "Mode: " + mode;
        document.getElementById("State").textContent = "State: " + state;
        document.getElementById("HSStatus").textContent = "Heat Shield Deployed: " + hsDeployed;
        document.getElementById("PCStatus").textContent = "Parachute Deployed: " + pcDeployed;
        document.getElementById("MastStatus").textContent = "Flag Raised: " + mastRaised;
        document.getElementById("VoltageLabel").textContent = "Voltage: " + volt + "V";
        document.getElementById("GPSTimeLabel").textContent = "GPS Time: " + gpsTime;
        document.getElementById("GPSAltitude").textContent = "GPS Altitude: " + gpsAltitude + "m";
        document.getElementById("GPSSats").textContent = "SIV: " + gpsSatellites;
        document.getElementById("CMDEcho").textContent = "Command Echo: " + cmdEcho;

    }

    gcsSocket.onmessage = function (event) {
        console.log(event.data);

        if (event.data == "1070,PING") {
            console.log("Pong!");
        } else {
            updateData(event.data);
        }
    };

    var testBtn = document.getElementById("testBtn");
    testBtn.addEventListener("click", function (e) {
        gcsSocket.send("Test Button Clicked");
        e.preventDefault();
    });

    var telemToggle = document.getElementById("telemetryToggle");
    telemToggle.addEventListener("change", function (e){
        if (telemToggle.checked){
            gcsSocket.send("CXON");
        } else {
            gcsSocket.send("CXOFF");
        }
        e.preventDefault();
    });

    var setgpsTimeBtn = document.getElementById("setGPSTimeBtn");
    setgpsTimeBtn.addEventListener("click", function (e){
        gcsSocket.send("STGPS");
        e.preventDefault();
    });

    var setCurTimeBtn = document.getElementById("setCurTimeBtn");
    setCurTimeBtn.addEventListener("click", function(e){
        const d = new Date();
        let hour = d.getUTCHours().toString();
        let minute = d.getUTCMinutes().toString();
        let second = d.getUTCSeconds().toString();
        if (hour.length == 1){
            hour = "0" + hour;
        }
        if (minute.length == 1){
            minute = "0" + minute;
        }
        if (second.length == 1){
            second = "0" + second;
        }
        gcsSocket.send("ST" + hour + ":" + minute + ":" + second);
        e.preventDefault();
    });

    var calBtn = document.getElementById("calBtn");
    calBtn.addEventListener("click", function (e){
        gcsSocket.send("CAL");
        e.preventDefault();
    });

    var mrBtn = document.getElementById("mrBtn");
    mrBtn.addEventListener("click", function (e){
        gcsSocket.send("ACTMR");
        e.preventDefault();
    });

    var hsBtn = document.getElementById("hsBtn");
    hsBtn.addEventListener("click", function (e){
        gcsSocket.send("ACTHS");
        e.preventDefault();
    });

    var pcBtn = document.getElementById("pcBtn");
    pcBtn.addEventListener("click", function (e){
        gcsSocket.send("ACTPC");
        e.preventDefault();
    });

    var audioToggle = document.getElementById("audioToggle");
    audioToggle.addEventListener("change", function (e){
        gcsSocket.send("ACTAB");
        e.preventDefault();
    });

    var ledToggle = document.getElementById("ledToggle");
    ledToggle.addEventListener("change", function (e){
        gcsSocket.send("ACTLED");
        e.preventDefault();
    });

    var pingBtn = document.getElementById("pingBtn");
    pingBtn.addEventListener("click", function (e){
        gcsSocket.send("PING");
        e.preventDefault();
    });

    var resetBtn = document.getElementById("resetBtn");
    resetBtn.addEventListener("click", function (e){
        gcsSocket.send("RESET");
        e.preventDefault();
    });

    var altChart = new CanvasJS.Chart("altitude", {
        title :{
            text: "Altitude (m)"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: altitudeArr 
        }]
    });
    var tempChart = new CanvasJS.Chart("temperature", {
        title :{
            text: "Temperature (°C)"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: temperatureArr
        }]
    })
    var presChart = new CanvasJS.Chart("pressure", {
        title :{
            text: "pressure (kPa)"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: pressureArr
        }]
    })
    var tiltChart = new CanvasJS.Chart("tilt", {
        title :{
            text: "Tilt (°)"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: tiltArr
        }]
    })
    var locChart = new CanvasJS.Chart("location", {
        title :{
            text: "Location (°)"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: locationArr
        }]
    })
    var voltChart = new CanvasJS.Chart("voltage", {
        title :{
            text: "Voltage (V)"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: voltageArr
        }]
    })
}