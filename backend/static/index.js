let missionTime = "00:00:00";
let packetCount = -1;
let mode = "F";
let state = "IDLE";
let altitudeArr = [];
let hsDeployed = "N";
let pcDeployed = "N";
let mastRaised = "N";
let temperatureArr = [];
let pressureArr = [];
let voltageArr = [];
let gpsTime = "00:00:00";
let gpsAltitude = 0.0;
let locationArr = [];
let gpsSatellites = -1;
let tiltArr = [];
let cmdEcho = "NONE";
let simMode = false;

let startTime = "00:00:00";

let fontFamily = "Helvetica";

window.onload = function () {
    let gcsSocket = new WebSocket("ws://localhost:8080/ws");
 
    let dataLength = 500; // number of dataPoints visible at any point

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

        if (startTime == "00:00:00" && typeof(missionTime) == typeof("")) {
            startTime = missionTime;
        }

        let startTimeArr = [];
        let timeArr = [];

        startTime.split(":").forEach(function (e){
            startTimeArr.push(parseInt(e));
        });

        missionTime.split(":").forEach(function (e){
            timeArr.push(parseInt(e));
        });

        time = (timeArr[0] - startTimeArr[0])*3600 + (timeArr[1] - startTimeArr[1])*60 + (timeArr[2] - startTimeArr[2]);

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

    let telemToggle = document.getElementById("telemetryToggle");
    telemToggle.addEventListener("change", function (e){
        if (telemToggle.checked){
            gcsSocket.send("CXON");
        } else {
            gcsSocket.send("CXOFF");
        }
        e.preventDefault();
    });

    let setgpsTimeBtn = document.getElementById("setGPSTimeBtn");
    setgpsTimeBtn.addEventListener("click", function (e){
        gcsSocket.send("STGPS");
        e.preventDefault();
    });

    let setCurTimeBtn = document.getElementById("setCurTimeBtn");
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

    let calBtn = document.getElementById("calBtn");
    calBtn.addEventListener("click", function (e){
        gcsSocket.send("CAL");
        e.preventDefault();
    });

    let mrBtn = document.getElementById("mrBtn");
    mrBtn.addEventListener("click", function (e){
        gcsSocket.send("ACTMR");
        e.preventDefault();
    });

    let hsBtn = document.getElementById("hsBtn");
    hsBtn.addEventListener("click", function (e){
        gcsSocket.send("ACTHS");
        e.preventDefault();
    });

    let pcBtn = document.getElementById("pcBtn");
    pcBtn.addEventListener("click", function (e){
        gcsSocket.send("ACTPC");
        e.preventDefault();
    });

    let audioToggle = document.getElementById("audioToggle");
    audioToggle.addEventListener("change", function (e){
        gcsSocket.send("ACTAB");
        e.preventDefault();
    });

    let ledToggle = document.getElementById("ledToggle");
    ledToggle.addEventListener("change", function (e){
        gcsSocket.send("ACTLED");
        e.preventDefault();
    });

    let pingBtn = document.getElementById("pingBtn");
    pingBtn.addEventListener("click", function (e){
        gcsSocket.send("PING");
        e.preventDefault();
    });

    let resetBtn = document.getElementById("resetBtn");
    resetBtn.addEventListener("click", function (e){
        gcsSocket.send("RESET");
        e.preventDefault();
    });

    let resetReleaseBtn = document.getElementById("relResetBtn");
    resetReleaseBtn.addEventListener("click", function (e){
        gcsSocket.send("RESREL");
        e.preventDefault();
    });

    let simBtn = document.getElementById("simBtn");
    simBtn.addEventListener("click", function (e){
        if (simMode) {
            gcsSocket.send("SIMA");
        } else {
            gcsSocket.send("GEN");
        }
        e.preventDefault();
    });

    let simToggle = document.getElementById("simulationToggle");
    simToggle.addEventListener("change", function (e){
        if (simToggle.checked){
            simMode = true;
            gcsSocket.send("SIME");
        } else {
            simMode = false;
            gcsSocket.send("SIMD");
        }
        e.preventDefault();
    });

    let altChart = new CanvasJS.Chart("altitude", {
        title:{
            text: "Altitude",
            fontFamily
        },
        axisX:{
            title: "Time (s)"
        },
        axisY:{
            title: "Altitude (m)"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: altitudeArr 
        }]
    });
    let tempChart = new CanvasJS.Chart("temperature", {
        title:{
            text: "Temperature",
            fontFamily
        },
        axisX:{
            title: "Time (s)"
        },
        axisY:{
            title: "Temperature (°C)"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: temperatureArr
        }]
    })
    let presChart = new CanvasJS.Chart("pressure", {
        title:{
            text: "Pressure",
            fontFamily
        },
        axisX:{
            title: "Time (s)"
        },
        axisY:{
            title: "Pressure (kPa)"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: pressureArr
        }]
    })
    let tiltChart = new CanvasJS.Chart("tilt", {
        title:{
            text: "Tilt",
            fontFamily
        },
        axisX:{
            title: "Tilt X (°)"
        },
        axisY:{
            title: "Tilt Y (°)"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: tiltArr
        }]
    })
    let locChart = new CanvasJS.Chart("location", {
        title:{
            text: "Location",
            fontFamily
        },
        axisX:{
            title: "Longitude (°)"
        },
        axisY:{
            title: "Latitude (°)"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: locationArr
        }]
    })
    let voltChart = new CanvasJS.Chart("voltage", {
        title:{
            text: "Voltage",
            fontFamily
        },
        axisX:{
            title: "Time (s)"
        },
        axisY:{
            title: "Voltage (V)"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: voltageArr
        }]
    })

    altChart.render();
    tempChart.render();
    presChart.render();
    tiltChart.render();
    locChart.render();
    voltChart.render();
}