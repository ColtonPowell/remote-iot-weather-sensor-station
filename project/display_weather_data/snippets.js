
// Server-side file read
// Readfile location can be either relative or absolute
function readFile(file) {
    var rawFile = new XMLHttpRequest();
    rawFile.open("GET", file, false);
    rawFile.onreadystatechange = function ()
    {
	if(rawFile.readyState === 4)
	{
	    if(rawFile.status === 200 || rawFile.status == 0)
	    {
		var allText = rawFile.responseText;
		var lines = allText.split('\n');
		for (var line = 0; line < lines.length; line++) {
		    alert("Line: " + line + " = " + lines[line]);
		}
	    }
	}
    }
    rawFile.send(null);
}

readFile("test.txt");


// Client-side file read
// Note: Clients must manually supply a file for security purposes
document.getElementById('file').onchange = function() {
    var file = this.files[0];
    var reader = new FileReader();

    reader.onload = (function(progressEvent) {
	console.log(this.result);
    });

    reader.readAsText(file);
};
var ctx = document.getElementById("lineChart").getContext('2d');
var myChart = new Chart(ctx, {

    var MONTHS = ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December'];
    var config = {
	type: 'line',
	data: {
	    labels: ['January', 'February', 'March', 'April', 'May', 'June', 'July'],
	    datasets: [{
		label: 'My First dataset',
		backgroundColor: window.chartColors.red,
		borderColor: window.chartColors.red,
		data: [
		    randomScalingFactor(),
		    randomScalingFactor(),
		    randomScalingFactor(),
		    randomScalingFactor(),
		    randomScalingFactor(),
		    randomScalingFactor(),
		    randomScalingFactor()
		],
		fill: false,
	    }, {
		label: 'My Second dataset',
		fill: false,
		backgroundColor: window.chartColors.blue,
		borderColor: window.chartColors.blue,
		data: [
		    randomScalingFactor(),
		    randomScalingFactor(),
		    randomScalingFactor(),
		    randomScalingFactor(),
		    randomScalingFactor(),
		    randomScalingFactor(),
		    randomScalingFactor()
		],
	    }]
	},
	options: {
	    responsive: true,
	    title: {
		display: true,
		text: 'Chart.js Line Chart'
	    },
	    tooltips: {
		mode: 'index',
		intersect: false,
	    },
	    hover: {
		mode: 'nearest',
		intersect: true
	    },
	    scales: {
		xAxes: [{
		    display: true,
		    scaleLabel: {
			display: true,
			labelString: 'Month'
		    }
		}],
		yAxes: [{
		    display: true,
		    scaleLabel: {
			display: true,
			labelString: 'Value'
		    }
		}]
	    }
	}
    };
    window.onload = function() {
	var ctx = document.getElementById('canvas').getContext('2d');
	window.myLine = new Chart(ctx, config);
    };
    document.getElementById('randomizeData').addEventListener('click', function() {
	config.data.datasets.forEach(function(dataset) {
	    dataset.data = dataset.data.map(function() {
		return randomScalingFactor();
	    });
	});
	window.myLine.update();
    });
    var colorNames = Object.keys(window.chartColors);
    document.getElementById('addDataset').addEventListener('click', function() {
	var colorName = colorNames[config.data.datasets.length % colorNames.length];
	var newColor = window.chartColors[colorName];
	var newDataset = {
	    label: 'Dataset ' + config.data.datasets.length,
	    backgroundColor: newColor,
	    borderColor: newColor,
	    data: [],
	    fill: false
	};
	for (var index = 0; index < config.data.labels.length; ++index) {
	    newDataset.data.push(randomScalingFactor());
	}
	config.data.datasets.push(newDataset);
	window.myLine.update();
    });
    document.getElementById('addData').addEventListener('click', function() {
	if (config.data.datasets.length > 0) {
	    var month = MONTHS[config.data.labels.length % MONTHS.length];
	    config.data.labels.push(month);
	    config.data.datasets.forEach(function(dataset) {
		dataset.data.push(randomScalingFactor());
	    });
	    window.myLine.update();
	}
    });
    document.getElementById('removeDataset').addEventListener('click', function() {
	config.data.datasets.splice(0, 1);
	window.myLine.update();
    });
    document.getElementById('removeData').addEventListener('click', function() {
	config.data.labels.splice(-1, 1); // remove the label first
	config.data.datasets.forEach(function(dataset) {
	    dataset.data.pop();
	});
	window.myLine.update();
    });
    
    
    // BAR CHART
    // type: 'bar',
    // data: {
    //     labels: ["Red", "Blue", "Yellow", "Green", "Purple", "Orange"],
    //     datasets: [{
    //         label: '# of Votes',
    //         data: [12, 19, 3, 5, 2, 3],
    //         backgroundColor: [
    //             'rgba(255, 99, 132, 0.2)',
    //             'rgba(54, 162, 235, 0.2)',
    //             'rgba(255, 206, 86, 0.2)',
    //             'rgba(75, 192, 192, 0.2)',
    //             'rgba(153, 102, 255, 0.2)',
    //             'rgba(255, 159, 64, 0.2)'
    //         ],
    //         borderColor: [
    //             'rgba(255,99,132,1)',
    //             'rgba(54, 162, 235, 1)',
    //             'rgba(255, 206, 86, 1)',
    //             'rgba(75, 192, 192, 1)',
    //             'rgba(153, 102, 255, 1)',
    //             'rgba(255, 159, 64, 1)'
    //         ],
    //         borderWidth: 1
    //     }]
    // },
    // options: {
    //     scales: {
    //         yAxes: [{
    //             ticks: {
    //                 beginAtZero:true
    //             }
    //         }]
    //     }
    // }
});
