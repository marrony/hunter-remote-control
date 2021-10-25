## Hunter Ceiling RC library

This library allow you control Hunter Ceiling fan using Arduino. Only tested with RC model 99119.

It was tested in a very noisy environment, so I had to introduce many timing heuristics in order to be able to decode the values. Those heusristics was based on observations using a Hantek DSO2D15 oscilloscope.

## TODO

Make the receiver more resilient. Currently not all decoded values is valid to be transmitted, it's a trial and error process.
