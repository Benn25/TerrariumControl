 ## Terrarium controller
#A controller and monitoring unit for terrariums.
light and hygrometry are monitored. The outputs are mist maker (or rain), pump, main light, secondary light and fan. a SPI LCD screen (I use a 320*240 ILI9341) in landscape orientation is used for the display.

-Based on ESP32
-ILI9341 LCD screen
-DHT22 for the hygro and temp

#libraries needed
-ESP32Time.h
-Preferences.h (included by default)
-FS.h (included by default)
-SPI.h (included by default)
-TFT_eSPI.h
-TFT_eWidget.h
-"Free_Fonts.h" (comes with the TFT lib, you may need to move the file)

In the TFT_eSPI library, dont forget to modify the User_setup.h to match your setup