// Get a statistic type for a given number
function get_stat_type(num) {
    if (num == 0) return "Temperature_stats";
    else if (num == 1) return "Humidity_stats";
    else if (num == 2) return "Light_stats";
    else if (num == 3) return "POT_stats";
    else console.log("Error: cannot handle unknown stats value " + num);
}

// Get a color type for a given number
// (Used to map colors to different statistics)
function get_stat_color(num) {
    if (num == 0) return window.chartColors.green;
    else if (num == 1) return window.chartColors.blue;
    else if (num == 2) return window.chartColors.yellow;
    else if (num == 3) return window.chartColors.red;
    else console.log("Error: cannot handle unknown stats value " + num);
}

// Get a radius for a given statistic
// (Since some statistics may have higher variances than others, this is
//  used to prevent Chart.js'es graph resizing functionality from getting
//  out of hand)
function get_stat_radius(num) {
    if (num == 0) return 10;
    else if (num == 1) return 10;
    else if (num == 2) return 300;
    else if (num == 3) return 1;
    else console.log("Error: cannot handle unknown stats value " + num);
}

// This will generate a configuration for a chart, which we can then use to
// create a chart (see update_graphs() in update.js) Note: The functions above
// are used here in this function to keep the code compact and managable. We use
// the device_name and stat_num to grab the appropriate colors, stat type
// strings, and chart radius configuration from them.
function get_chart_config(device_name, stat_num)  {
    var device_index = devices.indexOf(device_name);
    var config = {
	type: 'line',
	data: {
	    labels: index_strings,
	    datasets: [{
		label: get_stat_type(stat_num).split('_')[0],
		backgroundColor: get_stat_color(stat_num),
		borderColor: get_stat_color(stat_num),
		data: window[get_stat_type(stat_num)][device_index],
		fill: false,
	    }]
	},
	options: {
	    responsive: true,
	    maintainAspectRatio: false,
	    animation: false,
	    title: {
		display: true,
		text: get_stat_type(stat_num).split('_')[0] + " from device: " + device_name
	    },
	    tooltips: {
		enabled: false,
		mode: null
	    },
	    hover: {
		mode: null
	    },
	    scales: {
		xAxes: [{
		    display: true,
		    scaleLabel: {
			display: true,
			labelString: 'Readings'
		    },
		    ticks: {
			autoSkip:true
		    }
		}],
		yAxes: [{
		    display: true,
		    scaleLabel: {
			display: true,
			labelString: get_stat_type(stat_num).split('_')[0]
		    },
		    ticks: {
			beginAtZero: true,
			autoSkip:true
		    }
		}]
	    }
	}
    };
    return config;
}
