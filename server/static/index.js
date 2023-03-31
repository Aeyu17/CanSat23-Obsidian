window.onload = function () {

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
    
    var updateChart = function (count) {
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
    
    updateChart(dataLength); 
    setInterval(function(){ updateChart() }, updateInterval); 
    
}

function buttontest(){
    console.log("AAAAAAAAAA");
}