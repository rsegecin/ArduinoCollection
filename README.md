# ArduinoCollections

Add Lua Regexp library from [nickgammon/Regexp](https://github.com/nickgammon/Regexp)

To add new serial command don't forget to add the command's name in the **enum eSerialCommands** inside the file **SerialInterpreter**.h.

This serial library interprets messages with the following format:
```
    \r\n[name of the command], [parametes separated by coma ',', ';' or '=']\r\n 
```

eg:
```
    \r\nsetdate,2017-10-08T08:33:49.659Z\r\n
```
