display_weather_data contains the web application used for anyone in the world to request a web page and receive weather data charts in real-time.
We hosted this application on a server. Ideally, you would want to configure this server to return index.html whenever a user tries to access your website.

gather_weather_data contains the application used for collecting weather data with the Cypress CYW943907AEVAL1F + The PSoC AFE Shield to then send to any
instances of the web application using MQTT. Setup involves simply copying the folder into your IoT project workspace location in WICED Studio, properly 
configuring a make target, and then flashing the application to the board.

mosquitto just contains an example mosquitto configuration file that may be useful in setting up the mosquitto MQTT broker (as it must allow WebSockets communications
in order to communicate with instances of the web application via MQTT).