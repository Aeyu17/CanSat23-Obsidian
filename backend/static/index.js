window.onload = function () {
    var gcsSocket = new WebSocket("ws://localhost:8080/ws");

    gcsSocket.onmessage = function (event) {
        console.log(event.data)
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
    
    var time = 0;
    var alt = 0;
    var temp = 30;
    var pres = 1000;
    var tiltx = 0;
    var tilty = 0;
    var locx = 0;
    var locy = 0
    var volt = 6;
    var updateInterval = 1000;
    var dataLength = 50; // number of dataPoints visible at any point
    
    var updateChart = function (count, packet) {
        count = count || 1;
        // count is number of times loop runs to generate random dataPoints.
        for (var j = 0; j < count; j++) {	
            alt += -5 * Math.random() + 5;
            temp += Math.random() - 0.5;
            pres += Math.random() - 0.5;
            tiltx += 2 * Math.random() - 1;
            tilty += 2 * Math.random() - 1;
            locx += 2 * Math.random() - 1;
            locy += 2 * Math.random() - 1;
            volt = 5.75 + 0.5 * Math.random();

            alts.push({
                x: time,
                y: alt
            });
            temps.push({
                x: time,
                y: temp
            });
            press.push({
                x: time,
                y: pres
            });
            tilts.push({
                x: tiltx,
                y: tilty
            });
            locs.push({
                x: locx,
                y: locy
            });
            volts.push({
                x: time,
                y: volt
            });
            time++;
        }
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
        document.getElementById("TimeLabel").textContent = "Time: " + time + " s";
        document.getElementById("VoltageLabel").textContent = "Voltage: " + Math.round(100 * volt) / 100 + "V";

    }; 
    setInterval(function(){ updateChart() }, updateInterval); 
    
}