# PiPlateRelay
PiPlate Relay Source - For product https://pi-plates.com/product/relaykit/

Intro:

This simple library can be referenced from NuGet or Copy the .cs file(s) direct from this repo into your program

Usage:

Simple usage - this is a class with static functions.  Initlize the GPIO first by calling .Initilize();

After that, call the public API calls. They should be self explanatory. 

Calls end in Async only because there are some Task Delays. Later, the API may support a more Async API.
