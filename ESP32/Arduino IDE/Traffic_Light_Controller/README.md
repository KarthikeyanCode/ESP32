# Traffic Light Controller with HTTP server

- In this project we use ESP32 and FreeRTOS SMP changes to make a traffic light controller with HTTP server.

- We run the HTTP server in first core (PRO_CPU(0)) and the traffic light tasks in the second core (APP_CPU(1)), so both are independent of each other but the information is shared.

- HTTP server part is still under development.