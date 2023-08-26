# ESP8266-Timer-Relay

This code is for controlling multiple relays with multiple times, you are only limited by number of GPIO pins 
and RAM ( as which relay to be on / off is pre-populated at start of ESP8266 for better performance )
To increase number of timers change number in common.h file.

Features:
mDNS support
Timezone support
Supports multiple relays with individual active low / high configuration
Multiple timers per relay, i.e for each realy you can specify multipe on, off timings


Steps after you flash this code:

Default hostname is relay so goto http://relay.local after flashing this code.
As web page is not part of code so need to manually post that html file to ESP via post call, once that is done just refresh http://relay.local page, this is one time activity.


Post Call:

![relay_html](https://github.com/ankushkale1/ESP8266-Timer-Relay/assets/5172482/afc8cc42-023c-4e4d-839d-9e59f97d19cb)



Homepage:

![relay_page](https://github.com/ankushkale1/ESP8266-Timer-Relay/assets/5172482/065bc1de-a286-4e0c-b16d-2eded1daf8e6)
