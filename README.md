# cpp-transport-catalogue
**Transport catalogue.**

All about bus routes in big city. 

Buiding all bus routes with bus-stopes. Each route has bus stops. Each bus stop has gps-geo-coodrdinates. Calculates of time mooving bus between bus stops. 

Building the shortes way from one bus stopes to any others stops. Uses graph for routing.

Uses 2 modes: make_base and requests. 
Serialises database using protobuf library. For in/out operations uses JSON


Builded with cmake. C++17

Tested with 3000+ bus routes.


