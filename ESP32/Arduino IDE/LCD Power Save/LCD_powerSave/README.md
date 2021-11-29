# LCD Power Save using Software Timers FreeRTOS

- This code displays the serial monitor input text to LCD 

- Once it is done displaying it dims the backlights of lcd to save power

- No text will be visible when the lcd is in dim state

- Only when text is entered the lcd backlights will be turned on and will display
  the text for a time period, then lcd backlights will turn off. This will help in
  saving power when lcd is not used for long time.

- To change the display time period, change the time delay argument of xTimerCreate.
  This is the second argument. Here, change the delay time in milliseconds. It is 
  initialized to 5000 (5 seconds). 


