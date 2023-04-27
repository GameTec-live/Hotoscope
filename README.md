# Hotoscope
Hotoscope, Merge a hotplate and a microscope

The Hotoscope is a DIY hotplate combined with a cheap mircoscope and stand to do reflow/Hotplate soldering.

It Features 2 Defineable curves for Soldering and Desoldering, it also supports a Manual Temperature setting.

![Finished thing](https://raw.githubusercontent.com/GameTec-live/Hotoscope/main/resources/finished.jpg)

## Guide

### Requirements
* Software:
    * [This Repository](https://github.com/GameTec-live/Hotoscope/archive/refs/heads/main.zip)
    * [VS Code](https://code.visualstudio.com/)
    * [PlatformIO](https://platformio.org/install/ide?install=vscode)
* Hardware:
    * [D1mini]
     https://www.aliexpress.com/item/1005004281296544.html?pdp_npi=2%40dis%21GBP%21%EF%BF%A1%202.68%21%EF%BF%A1%201.72%21%21%21%21%21%40211b813b16826152959361434ec5ad%2112000028614259615%21btf&_t=pvid:d8ff26ae-16b9-448e-90b2-6528580a151d&afTraceInfo=1005004281296544__pc__pcBridgePPC__xxxxxx__1682615296&spm=a2g0o.ppclist.product.mainProduct
    * [SSR, 2AMP Rated for mains voltage](aliexpress.com/item/1005004650376762.html?pdp_npi=2%40dis%21GBP%21￡%200.98%21￡%200.69%21%21%21%21%21%40211b613916811404841866479ec512%2112000032063843810%21btf&_t=pvid:782a5978-c6d1-453b-b5dd-098ef49f97f2&afTraceInfo=1005004650376762__pc__pcBridgePPC__xxxxxx__1681140484&spm=a2g0o.ppclist.product.mainProduct)
    * [Mains to 5V PSU](https://www.ebay.co.uk/itm/255861034090?chn=ps&norover=1&mkevt=1&mkrid=710-134428-41853-0&mkcid=2&mkscid=101&itemid=255861034090&targetid=1404115578893&device=c&mktype=pla&googleloc=9045382&poi=&campaignid=17218284410&mkgroupid=142217514411&rlsatarget=pla-1404115578893&abcId=9300867&merchantid=507156551&gclid=CjwKCAjw586hBhBrEiwAQYEnHa10j7ie6Lr-P5FX5cmvnE6lMhkyvXqVWfJETd_cgqhWmba2id9gyhoC67oQAvD_BwE)
    * [Termistor, b3950](https://www.ebay.co.uk/itm/191094757881?var=490279055890)
    * [Passive buzzer 9mmx6mm](https://www.alibaba.com/product-detail/Small-MANORSHI-small-12mm-pitch-6_1600509940899.html?spm=a2700.7724857.0.0.5eb77604znyH4q&s=p)
    * [4 5x5x9mm Buttons](https://www.futureelectronics.com/p/electromechanical--switches--tactile/1-1825910-4-te-connectivity-3151637?loc=2&gclid=CjwKCAjw586hBhBrEiwAQYEnHfQA5l15RXz4ExHQ1LEOUpsWuGnWCzuNjYq9FmcAGPYYVYgsb8T6IhoC5MIQAvD_BwE)
    * [Oled Screen ssd1306 i2c](https://www.ebay.co.uk/itm/191094757881?var=490279055890)
    * [Hotplate](https://www.aliexpress.com/item/1005004388602432.html?pdp_npi=2%40dis%21GBP%21%EF%BF%A1%204.60%21%EF%BF%A1%202.07%21%21%21%21%21%40211b423c16811413812528468e7dda%2112000030335623574%21btf&_t=pvid:e18354bb-ce08-4744-afd5-6f7a5f524432&afTraceInfo=1005004388602432__pc__pcBridgePPC__xxxxxx__1681141381&spm=a2g0o.ppclist.product.mainProduct)
    * [Stand](https://www.aliexpress.com/item/1005003345032593.html?pdp_npi=2%40dis%21GBP%21%EF%BF%A1%209.52%21%EF%BF%A1%206.95%21%21%21%21%21%40211b440316811416319908974ea292%2112000025326513821%21btf&_t=pvid:71ae821b-caed-4ccb-b58d-75416c87dfba&afTraceInfo=1005003345032593__pc__pcBridgePPC__xxxxxx__1681141632&spm=a2g0o.ppclist.product.mainProduct)
    * [Microscope](https://www.aliexpress.com/item/1005005302044086.html?pdp_npi=2%40dis%21GBP%21%EF%BF%A1%2011.36%21%EF%BF%A1%205.45%21%21%21%21%21%40211b440316811416758631636ea292%2112000032551569481%21btf&_t=pvid:14721063-a627-4c5d-8040-eadb4e22b49a&afTraceInfo=1005005302044086__pc__pcBridgePPC__xxxxxx__1681141676&spm=a2g0o.ppclist.product.mainProduct)
    * USB TTL Converter for programming
    * 2.2k resistor
    * 10uf Capacitor
    * 4 50mm m4 nuts and bolts
    * (Optional) 2 10k resistors for the i2c lines
    * [(Optional) 3D Printed Case](https://github.com/GameTec-live/Hotoscope/tree/main/resources)

### Hardware
Connect the components according to the following diagram:
![Wiring Diagram](https://raw.githubusercontent.com/GameTec-live/Hotoscope/main/resources/wring.png)

To secure it to the housing you may use a glue of your choice.

### Software
Download install VS Code and PlatformIO, also Download this repository and unpack it into a folder of you choosing.
After the Downloads are Complete, Open VS Code and go to File -> Open Folder... and choose the folder you unpacked this Repository into.
PlatformIO should let you know, that it is a PlatformIO project by popping up PlatformIO Home and platformio.ini. Close and ignore those.
Connect your arduino to your pc, click on the alien (PlatformIO) in VS Codes sidebar and Press "Upload". Done, PlatformIO will do the rest and thats the software part.
<details>
    <summary>Advanced Configuration</summary>
    Lets say you cant get your hands on a Pro Mini, you can change for what processor PIO builds by modifying lines 11 and 13 in platformio.ini (Refer to PIO Docs for more info)
    Lets say you want to customize your Curves. Open main.cpp and look at lines 18 - 28. Those lines are the solder and desolder curves.
    the left side defines at what second it switches to the next stage, the right side is the temperature in °C.
    Adjust those Curvse according to your needs.

    NOTE: Depending on your Hotplate and Heating Pad you may need to fiddle around with the PID values.
</details>

## Usage
Now that you have your Hotoscope, lets go over how to use it:

When starting up you will be greeted with the main menu. At the bottom you find the current SSR dutycycle and current Temperature sensors.
You navigate the menu by pressing the right button (right top -> up, right bottom -> down)
You Select the entry by pressing the bottom left button. This button acts as a "back" button in any Menu not the main Menu. Hold this button for a while and you will be greeted with a shortcut to manual mode.
The top left buttons is a shortcut to solder mode and immidately starts the Soldering cycle.

In Manual mode (either selected from the menu or long press) you set the temperature via the up and down buttons.

In Solder mode (Menu or shortcut button) the temperature follows the predetermined curve.

In Desolder mode its the same as solder mode.

Cooldown just sets everything to 0 and cools down.

And thats all there is to it :p

Have fun soldering, youll figure it out!