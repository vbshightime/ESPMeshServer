# ESPMeshServer
## Mesh Network

A mesh network (or simply Meshnet) is a local network topology in which the infrastructure nodes (i.e. bridges, switches and other infrastructure devices) connect directly, dynamically and non-hierarchically to as many other nodes as possible and cooperate with one another to efficiently route data from/to clients. Mesh networks dynamically self-organize and self-configurable. Mesh networks can relay messages using either a flooding technique or a routing technique. With routing, the message is propagated along a path by hopping from node to node until it reaches its destination.

## Introduction 

We have created a Mesh network or a star network. Our architecture consists of two cients and one server. Clients or nodes continuously broadcast the temperature and humidity values to the different nodes in the network. Values relayed by clients are fetched by server. The server parse these values and post it to cloud. We are making use of [painlessMesh](https://gitlab.com/BlackEdder/painlessMesh) library. This library provide us easy and user friendly ways to create a mesh network. There are different ways by which we can create a energy efficient and reliable mesh nework. This library uses different other libraries like [TaskScheduler](https://github.com/arkhipenko/TaskScheduler), [Arduino Json](https://github.com/bblanchon/ArduinoJson) and [ESPAsyncTCP](https://github.com/me-no-dev/ESPAsyncTCP) to serve this operation.

## Hardware Specification
 - ESP8266-01 (2 No.s)
 - NodeMcu Esp8266 12E
 - DHT 11 (2 No.s)
 
## How it works?

 - All the nodes in the mesh network share the same ssid and pass. All the nodes communicate using these credentials.
 - There ary many network topologies we can implement with using clients and servers, We are using star topology where there are two clients and one server
 - In our case we are using two clients deployed on ESP01. We are using DHT11 for temperature and humidity readings. These clients read the temprature and humidity values and send these values to server.
 - Nodes do not set any TCP or UDP link or any kind of HTTP link between them. These node have there unique id's they communicate using these ids
 - The Node in mesh listen to all other nodes in mesh using a call back called mesh.onReceiveCallback(from, &msg). The first parameter is id from where we are recieving the data and msg is the message we got from the node in JSON format.
 - There are two ways to send a message to other nodes
   1. mesh.sendSingle(nodeId, message)- Send messages to the single node with unique id
   2. mesh.sendBroadcast(message)- Broadcast the message to all other nodes in the mesh
 - So, The clients available, first receives the server id broadcasted by the server and then send the message to server
 - The message is delivered in JSON Format, The JSON works with key and value pair. We have created the JSON in this way
   1. The first value is name of client
   2. The second value will be temperature readings
   3. And the third will be humidity readings
 - On the server end, We areparsing the JSON values and getting the temperature and humidity values out of it
 - These values are then sent to cloud
 - We are using NodeRed server for this operation
 - [pubSubClient](https://pubsubclient.knolleary.net/) is used to subscribe to the particular topic and publishing the values to that topic created by MQTTBroker
 - So in this way we are sendin the values to cloud
 - For this purpose we are using TaskScheduler to schedule the task. This task will send the serverId to respective clients and publish the values to mqtt broker. We are running this task for 20sec. This task will perform N number of iteration in the span of 20 seconds.
 - mesh.init() method is used to initialise the mesh which takes ssid of mesh, pass of mesh, callback to scheduler, WiFI MODE(AP_STA), channel id
 - Painles mesh has one limitation that the mesh and the wifi router's channel id should be same, For this purpose we are hosting a werserver where user has to feed its credentials like ssid and password.
 - We are connecting to the wifi once fetching the WiFi's channel id and using the same channel id for mesh.
 - We are using mesh.stationManual(Userssid,Userpass) to connect to the WiFi Station.
 
## How the client works?
On the client side, We are getting temperature and humidity values from DHT 11 and putting these values as key and value pair in to JSON. These values are then sent to server. More on this can be found here [ESPMEshClient1](https://github.com/vbshightime/ESPMeshClient1) and [ESPMeshClient](https://github.com/vbshightime/ESPMeshClient2)

## Usage

# Coming Soon

## Profile

<img src="https://github.com/vbshightime/ESP32-Captive-Portal/blob/master/FZP1BLDJSCG1ZDD.LARGE.jpg">

<img src="https://github.com/vbshightime/ESPMeshServer/blob/master/Capture6.PNG">

<img src="https://github.com/vbshightime/ESPMeshServer/blob/master/Capture7.PNG">

<img src="https://github.com/vbshightime/ESPMeshServer/blob/master/Capture5.PNG">

## Limitations

- painless mesh is designed in such a way that to connect to local WiFi, The mesh channel id and local WiFi channel id should be same. it doesn't have any method for this task.
- Pianless mesh uses Task Sheduler library. But, they have modified it. So, all the methods found in Task Scheduler can't be performed here like setting callback to other method, setting timeout for the task etc.
- You must not use the delay() function. delay() hinders the background operation. Instead create delays using millis() only if it is necessary
- Don't run long running task it will hinder the receiveCallback operation

## Credits

- [Painless Mesh](https://gitlab.com/BlackEdder/painlessMesh)
- [DHT LIB](https://github.com/adafruit/DHT-sensor-library)
- [TaskScheduler](https://github.com/arkhipenko/TaskScheduler)
- [Arduino JSON](https://github.com/bblanchon/ArduinoJson)
- [NodeRed](https://nodered.org/)
- [PubSubClient](https://pubsubclient.knolleary.net/)

 
