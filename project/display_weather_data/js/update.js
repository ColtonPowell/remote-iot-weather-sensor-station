// Initialize data for a given device
function stats_init(device_name) {
    var device_index = devices.indexOf(device_name);
    Temperature_stats.push([]);
    Light_stats.push([]);
    Humidity_stats.push([]);
    POT_stats.push([]);
    for (var i = num_readings - 1; i >= 0; i--) {
	Temperature_stats[device_index].push(0);
	Light_stats[device_index].push(0);
	Humidity_stats[device_index].push(0);
	POT_stats[device_index].push(0);
    }
}

// Update temperature stats
function update_temp(device_name, new_temp) {
    var device_index = devices.indexOf(device_name);
    Temperature_stats[device_index].shift();
    Temperature_stats[device_index].push(new_temp);
    
}
// Update humidity stats
function update_humidity(device_name, new_humidity) {
    var device_index = devices.indexOf(device_name);
    Humidity_stats[device_index].shift();
    Humidity_stats[device_index].push(new_humidity);
}
// Update light stats
function update_light(device_name, new_light) {
    var device_index = devices.indexOf(device_name);
    Light_stats[device_index].shift();
    Light_stats[device_index].push(new_light);
}
// Update pot stats
function update_pot(device_name, new_pot) {
    var device_index = devices.indexOf(device_name);
    POT_stats[device_index].shift();
    POT_stats[device_index].push(new_pot);
}

// Create graphs for a given device
function create_graphs(device_name) {
    var device_index = devices.indexOf(device_name);
    // We want num_stats graphs (4 for this project, but could be increased if needed)
    for (var i = 0; i < num_stats; i++) {
	// Create a canvas first
	var new_canvas = document.createElement('canvas');
	new_canvas.id = "canvas-" + device_index + "_" + i;
	new_canvas.width = 200;
	new_canvas.height = 200;

	// Then create a div to place the canvas on
	var new_div = document.createElement('div');
	new_div.setAttribute("style", "position: relative; height: 40vh; width: 50vw");

	// Record div and canvas creation for later access
	divs.push(new_div);	
	canvases.push(new_canvas);

	// Put the canvas + div onto the document
	new_div.appendChild(new_canvas);
	document.body.appendChild(new_div);
    }
    update_graphs();
}

// update_stats(message) will update stats based on an MQTT message received
// Messages should be in the following format to be parsed:
// ID:<Value> (Note: Value here is the device ID, NOT the device_index)
// Temperature:<Value>
// Humidity:<Value>
// Light:<Value>
// POT:<Value>
function update_stats(message) {
    var lines = message.split('\n');
    var device_name;
    var device_index = -1;

    // Parse each of the lines for information
    for (var i = 0; i < lines.length; i++) {
	var values = lines[i].split(':');

	// First need the device ID for identification
	if (values[0] == 'ID') {
	    // Find the index of the sender device in the devices array
	    device_name = values[1];
	    device_index = devices.findIndex(function(some_device_id) {
		return some_device_id == values[1];
	    });

	    // If no index found: Create a new device and graphs for the device
	    if (device_index == -1) {
		console.log("Creating new graphs");
		device_index = devices.length;
		devices.push(values[1]);
		stats_init(device_name);
		
		// Maintain timeout for the device
		devices_timeouts.push(window.setTimeout(function() {
    		    remove_graphs(device_name);
		}, timeout_duration));

		// Finally create graphs for the device
		create_graphs(device_name);
	    }
	    // If the device already exists, be sure that it's graphs are visible
	    else {
		for (var j = 0; j < num_stats; j++) {
	    	    var div = divs[device_index * num_stats + j];
	    	    var canvas = canvases[device_index * num_stats + j];
	    	    if (!(document.body.contains(canvas))) {
	    		div.appendChild(canvas);
	    		document.body.appendChild(div);
	    	    }
		}
	    }
	    // console.log("New value: " + values[0] + " = " + values[1]);
	}
	// Update temperature stats
	else if (values[0] == 'Temperature') {
	    update_temp(device_name, parseFloat(values[1]));
	    // console.log("New value: " + values[0] + " = " + values[1]);
	}
	// Update humidity stats
	else if (values[0] == 'Humidity') {
	    update_humidity(device_name, parseFloat(values[1]));
	    // console.log("New value: " + values[0] + " = " + values[1]);
	}
	// Update light stats
	else if (values[0] == 'Light') {
	    update_light(device_name, parseFloat(values[1]));
	    // console.log("New value: " + values[0] + " = " + values[1]);
	}
	// Update pot stats
	else if (values[0] == 'POT') {
	    update_pot(device_name, parseFloat(values[1]));
	    // console.log("New value: " + values[0] + " = " + values[1]);
	}
	// Throw warning for invalid message fields
	else {
	    console.log("Error: Invalid msg component \"" +
			values[0] + " : " + values[1] + "\"" + " cannot be handled");
	}
    }

    // Finally, reset the timeout for the device so that it's graphs don't disappear
    window.clearTimeout(devices_timeouts[device_index]);
    devices_timeouts[device_index] = window.setTimeout(function() {
    	remove_graphs(device_name);
    }, timeout_duration);
}

// Update all graphs
function update_graphs() {
    for (var i = 0; i < canvases.length; i++) {
	var canvas = canvases[i];
	var config = get_chart_config(devices[parseInt(i/num_stats)], i % num_stats);
	var new_context = canvas.getContext("2d");
	canvas = new Chart(new_context, config);
    }
}

// Remove graphs for a certain device with id = device_name
function remove_graphs(device_name) {
    var device_index = devices.indexOf(device_name);
    window.clearTimeout(devices_timeouts[device_index]);         // Erase timeout for device
    for (var i = 0; i < num_stats; i++) {
	var canvas = canvases[device_index * num_stats + i];	 // Find canvas
	var div = divs[device_index * num_stats + i];		 // Find div containing canvas
	div.removeChild(canvas);				 // Remove canvas from div
	document.body.removeChild(div);				 // Remove div from page
    }
}
