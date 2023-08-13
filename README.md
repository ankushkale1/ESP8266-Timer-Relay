# ESP8266-Timer-Relay

This code has the following features. 
 *  If no wifi is set it will startup as an AP to which you can connect to via a web browser and using the gateway address. This can be used to configure the wifi SSID and password
 *  If you do not connect to the device while it is in AP mode it will timeout after 120 seconds and start a default program where it turns the relay on and off in 2 hour cycles.
 *  If you do connect and setup the device to connect to the wifi it will startup and poll an NTP server to get the current time. it will turn on and off the relay output with respect to a configured time based program
 *  The time based program can be configured via the web portal which will be accessible via the IP address of the device. Check your AP gateway for connected devices to find the IP address of the timer relay

Here is a preview of the web portal for configuring the device

![image](https://user-images.githubusercontent.com/86655396/156122741-a09e63b1-ef0d-4a91-a019-b27a5063a6d4.png)
