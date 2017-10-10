# ArduinoCollections
This's some libraries that I've built and worked recurrently though out the years and I hope you find them selves useful as its for me. It includes a serial interpreter and a RTC for Arduino. For the ISO date time parsing to work add Lua Regexp library from [nickgammon/Regexp](https://github.com/nickgammon/Regexp). The libraries are very simple so they practically explain them selves.

## To insert a new serial command: 

   - add the command's name in the **enum eSerialCommands** inside the file **SerialInterpreter**.h
   - while setting up execute the following command ```SerialInterpreter.AddCommand("print", SerialInterpreterClass::eSerialCommands::[]);```
   - add a new case on the switch inside SerialHandler function as you can see in **test.ino**.

This serial library interprets messages with the following format:
```
    \r\n[name of the command], [parameters separated by coma ',', ';' or '=']\r\n 
```

eg:
```
    \r\nsetdate,2017-10-08T08:33:49.659Z\r\n
```

## RTC 
   
   It started as a timer library that uses the microcontroller's timer0 interruption that can break the delay if some flag occurred. It's really useful when communicating with some other device like ETRX that takes some time to process a request and you'd like to keep doing some other stuff as soon the request is fulfilled. I included a sDateTime structure and a method for parsing ISO date time string into sDateTime, as timer0 ticks 1000 times it increases ```uint32_t Time``` that counts the seconds since the beginning of 2000.
