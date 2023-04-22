window.onload = function () {
    var gcsSocket = new WebSocket("ws://localhost:8080/ws");
 
    var startTime = "00:00:00";
    var dataLength = 500; // number of dataPoints visible at any point

    function updateData(packet) {
        // count is number of times loop runs to generate random dataPoints.
        let packetArr = packet.split(",");

        if (startTime == "00:00:00") {
            startTime = packetArr[1];
        }

        let startTimeArr = [];
        let timeArr = [];
        for (i = 0; i < 3; i++) {
            startTimeArr.push(parseInt(startTime.split(":")[i]));
            timeArr.push(parseInt(packetArr[1].split(":")[i]));
        }
        console.log(startTimeArr);
        console.log(timeArr);

        time = (timeArr[0] - startTimeArr[0])*3600 + (timeArr[1] - startTimeArr[1])*60 + (timeArr[2] - startTimeArr[2]);

        console.log(time);

        alts.push({
            x: time,
            y: parseFloat(packetArr[5])
        });
        temps.push({
            x: time,
            y: parseFloat(packetArr[9])
        });
        press.push({
            x: time,
            y: parseFloat(packetArr[11])
        });
        tilts.push({
            x: parseFloat(packetArr[17]),
            y: parseFloat(packetArr[18])
        });
        locs.push({
            x: parseFloat(packetArr[15]),
            y: parseFloat(packetArr[14])
        });
        volts.push({
            x: time,
            y: parseFloat(packetArr[10])
        });
        time++;
        if (alts.length > dataLength) {
            alts.shift();
        }
        if (temps.length > dataLength) {
            temps.shift();
        }
        if (press.length > dataLength) {
            press.shift();
        }
        if (tilts.length > dataLength) {
            tilts.shift();
        }
        if (locs.length > dataLength) {
            locs.shift();
        }
        if (volts.length > dataLength) {
            volts.shift();
        }
        altChart.render();
        tempChart.render();
        presChart.render();
        tiltChart.render();
        locChart.render();
        voltChart.render();

        // update labels
        document.getElementById("TimeLabel").textContent = "Time: " + packetArr[1];
        document.getElementById("PacketCount").textContent = "Packet Count: " + packetArr[2];
        document.getElementById("Mode").textContent = "Mode: " + packetArr[3];
        document.getElementById("State").textContent = "State: " + packetArr[4];
        document.getElementById("HSStatus").textContent = "Heat Shield Deployed: " + packetArr[6];
        document.getElementById("PCStatus").textContent = "Parachute Deployed: " + packetArr[7];
        document.getElementById("MastStatus").textContent = "Flag Raised: " + packetArr[8];
        document.getElementById("VoltageLabel").textContent = "Voltage: " + packetArr[10] +"V";
        document.getElementById("GPSTimeLabel").textContent = "GPS Time: " + packetArr[12];
        document.getElementById("GPSAltitude").textContent = "GPS Altitude: " + packetArr[13] + "m";
        document.getElementById("GPSSats").textContent = "SIV: " + packetArr[16];
        document.getElementById("CMDEcho").textContent = "Command Echo: " + packetArr[19];

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

    var alts = [];
    var temps = [];
    var press = [];
    var tilts = [];
    var locs = [];
    var volts = [];

    var altChart = new CanvasJS.Chart("altitude", {
        title :{
            text: "Altitude (m)"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: alts 
        }]
    });
    var tempChart = new CanvasJS.Chart("temperature", {
        title :{
            text: "Temperature (°C)"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: temps
        }]
    })
    var presChart = new CanvasJS.Chart("pressure", {
        title :{
            text: "Pressure (kPa)"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: press
        }]
    })
    var tiltChart = new CanvasJS.Chart("tilt", {
        title :{
            text: "Tilt (°)"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: tilts
        }]
    })
    var locChart = new CanvasJS.Chart("location", {
        title :{
            text: "Location (°)"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: locs
        }]
    })
    var voltChart = new CanvasJS.Chart("voltage", {
        title :{
            text: "Voltage (V)"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: volts
        }]
    })
}