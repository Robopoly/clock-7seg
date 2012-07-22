# Seven Segment Clock
The 7 segment clock consists of a PRisme2 and 7 segment driving chips for each digit that take impulses to increment the number by one.

Since the clock isn't absolutely accurate and must be set when daylight savings time occurs or whenever the power goes out a fast setting method has been implemented.

## Setting the time
To set the time one must send the time in HHMMSS format by UART at 9600 baud. A non valid time isn't accepted.
