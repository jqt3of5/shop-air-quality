# Shop Controller

Since moving my workshop is going into the basement. Because it's an enslcosed area, inside the house, I want my dust collection game on point. That means dust collection on every tool, air filtration, and purification. And all of this needs to be convenient to use. It's also very possible that air filtration will need to run for some time after leaving the shop. And might even consist of multiple stages (As an example, an air filter for large particles, and a carbon/HEPA filter for smaller ones). 


Most of this firmware will run on an ESP32 connecting to a home assistant instance. 

This project will consist of the following components: Air Quality Monitor, temperature and humidity, presence detection, and dust collection 

## Air quality monitor

I bought this Particular Matter Sensor: https://www.sparkfun.com/products/15103 It communicates over I2C or UART, though the UART supports more features. I haven't yet figured out what the best placement for this thing in the shop yet. But the goal is to measure the particulate matter in the shop, and automatically turn on/off the air filtration. Once the levels reach a certain point, the HEPA filter can switch on to finish up. This can prolong the life of the HEPA filter. 

## Temperature and Humidity 

A simple DHT22 will take measurements periodically. The basement doesn't have a separate thermostat, and the shop side doesn't have vents. So knowing the temperature in the workshop helps to make it easier to ensure it's comfortable. 

## Presence Detection

for this I have two options, bluetooth, or PIR sensor. I'll likely start out with the PIR sensor, because it's easy to use, and I have one. I'll be able to configure lights to turn on for as long as I'm down there. Using bluetooth would be a cool thing to learn andplay with. But I'm not as convinced that I won't get false positives (Like my phoneconnecting through the floor of the house). 

## Dust collection

The dust collector is connected to each large tool. It's pretty inconvenient to turn it on and off every time I want to use it. It is possible to turn the dust collector on only when the tool is actually being used. This doesn't necessarily have to work through the ESP32, but maybe I'll find a way.
Additionally, each tool should be fitted with a blast gate, so that we don't loose valuable suction to tool that aren't being used. And these blast gates should automatically open when the tool is used.  


