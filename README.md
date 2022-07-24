# Shop Controller

Since moving my workshop is going into the basement. Because it's an enslcosed area, inside the house, I want my dust collection game on point. That means dust collection on every tool, air filtration, and purification. And all of this needs to be convenient to use. It's also very possible that air filtration will need to run for some time after leaving the shop. And might even consist of multiple stages (As an example, an air filter for large particles, and a carbon/HEPA filter for smaller ones). 


Most of this firmware will run on an ESP32 connecting to a home assistant instance. 

This project will consist of the following components: Air Quality Monitor, temperature and humidity, presence detection, and dust collection 

## Air quality monitor

I bought this Particular Matter Sensor [Sparkfun](https://www.sparkfun.com/products/15103) It communicates over I2C or UART, though the UART supports more features. I haven't yet figured out what the best placement for this thing in the shop yet. But the goal is to measure the particulate matter in the shop, and automatically turn on/off the air filtration. Once the levels reach a certain point, the HEPA filter can switch on to finish up. This can prolong the life of the HEPA filter. 

## Temperature and Humidity 

A simple DHT22 will take measurements periodically. The basement doesn't have a separate thermostat, and the shop side doesn't have vents. So knowing the temperature in the workshop helps to make it easier to ensure it's comfortable. 

## Presence Detection

For this I'm using a PIR sensor, because it's easy to use, and I have one. I'll be able to configure lights and air filtration to turn on for as long as I'm down there. Using bluetooth would be a cool thing to learn andplay with. But I'm not as convinced that I won't get false positives (Like my phoneconnecting through the floor of the house). 

## Dust collection

The dust collector is connected to each large tool. It's pretty inconvenient to turn it on and off every time I want to use it, even if I had a remote. So I wanted to devise a system where it will turn on automatically when a dust collected tool is being used. The easiest way to do this was using split core current sensors [DigiKey](https://www.digikey.com/short/mbn82cdr). 

Duplex circuits on each outlet. Allowing 240V outlets on same circuit. Current sensors placed on tool hot, and neutral to reject 240V usages. 


