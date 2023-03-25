window.onload = function () {

    var alts = [];
    var altChart = new CanvasJS.Chart("altitude", {
        title :{
            text: "Altitude"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: alts 
        }]
    });
    var tempChart = new CanvasJS.Chart("temperature", {
        title :{
            text: "Temperature"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: alts
        }]
    })
    var presChart = new CanvasJS.Chart("pressure", {
        title :{
            text: "Pressure"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: alts
        }]
    })
    var tiltChart = new CanvasJS.Chart("tilt", {
        title :{
            text: "Tilt"
        },
        data: [{
            type: "spline",
            markerSize: 0,
            dataPoints: alts
        }]
    })
    
    var xVal = 0;
    var yVal = 100;
    var updateInterval = 1000;
    var dataLength = 50; // number of dataPoints visible at any point
    
    var updateChart = function (count) {
        count = count || 1;
        // count is number of times loop runs to generate random dataPoints.
        for (var j = 0; j < count; j++) {	
            yVal = yVal + Math.round(5 + Math.random() *(-5-5));
            alts.push({
                x: xVal,
                y: yVal
            });
            xVal++;
        }
        if (alts.length > dataLength) {
            alts.shift();
        }
        altChart.render();
        tempChart.render();
        presChart.render();
        tiltChart.render();
    };
    
    updateChart(dataLength); 
    setInterval(function(){ updateChart() }, updateInterval); 
    
}