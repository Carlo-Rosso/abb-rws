# abb-rws

[![license - bsd 3 clause](https://img.shields.io/:license-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

A C++ library for interfacing with ABB robot controllers through *Robot Web Services* (RWS) `1.0` (RobotWare `6.x`) and `2.0` (RobotWare `7.x`).

> **This is a hard fork of [ros-industrial/abb_librws](https://github.com/ros-industrial/abb_librws).**
> The original library (RWS 1.0) was written by **Jon Tjerngren** (ABB) and maintained by **G.A. vd. Hoorn** and the ROS-Industrial community. RWS 2.0 support was developed by **JOiiNT LAB** (IIT), **Gianluca Correnti** (IIT-Danieli Joint Lab) and **Carlo Rosso**, who maintains this fork.
> This fork supports both RWS 1.0 and 2.0 with a selectable protocol version, renames the package to `abb_rws` and packages it for conda-forge. See [AUTHORS.md](AUTHORS.md) for the full list of authors and contributors.
> This fork is not affiliated with or endorsed by ABB or ROS-Industrial.

## Installation

### conda-forge / pixi

```sh
pixi add abb-rws
# or
conda install -c conda-forge abb-rws
```

### From source

Dependencies: CMake >= 3.16, a C++17 compiler and [POCO](https://pocoproject.org) >= 1.4.3 with the `Foundation`, `Net`, `Util`, `XML`, `Crypto` and `NetSSL` components (OpenSSL).

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix <install-prefix>
```

With [pixi](https://pixi.sh), all dependencies come from conda-forge:

```sh
pixi run build            # configure + build (examples included)
pixi run -e ros colcon    # build the package with colcon, as ROS/RoboStack does
```

### Using the library from CMake

The CMake package and the target are named `abb_rws`:

```cmake
find_package(abb_rws REQUIRED)
target_link_libraries(my_target PRIVATE abb_rws::abb_rws)
```

The headers are under `abb_rws/`, e.g. `#include <abb_rws/rws_interface.h>`.

### ROS / RoboStack

The repository contains a ROS [`package.xml`](package.xml) (format 3, `build_type` `cmake`) for the package `abb_rws`, so it can be built in a ROS 1 or ROS 2 workspace with `colcon` or `catkin_tools`. The dependencies are declared with standard rosdep keys (`libpoco-dev`, `libssl-dev`), which RoboStack maps to the conda-forge `poco` and `openssl` packages.

In a [RoboStack](https://robostack.github.io) environment the conda-forge `abb-rws` package can be installed alongside the ROS packages, and other ROS packages can use it with `find_package(abb_rws)`.

## Important Notes

RobotWare main versions `6.x` and `7.x` are supported through RWS `1.0` and `2.0`, respectively. RWS 1.0 uses HTTP; RWS 2.0 uses HTTPS.

The `RWSInterface`, `RWSClient`, and `RWSStateMachineInterface` constructors accept a `POCOClient::RWSVersion` when the protocol should be selected explicitly:

```cpp
using RWSVersion = abb::rws::POCOClient::RWSVersion;

// RobotWare 6.x / RWS 1.0 / HTTP
abb::rws::RWSInterface legacy("192.168.125.1", 80, RWSVersion::RWS1);

// RobotWare 7.x / RWS 2.0 / HTTPS
abb::rws::RWSInterface modern("192.168.125.1", 443, RWSVersion::RWS2);
```

`RWSVersion::AUTO` selects RWS 2.0 when the port is `443`, and RWS 1.0 for any other port. This makes the usual virtual-controller configurations work without changing application code:

```cpp
abb::rws::RWSInterface controller("192.168.125.1", 443, RWSVersion::AUTO);
```

For HTTPS, the library creates a client SSL context with certificate verification disabled when no context is supplied. This is suitable for the self-signed certificates commonly used by RobotWare virtual controllers. For production use, pass a configured `Poco::Net::Context::Ptr` to the constructor to validate the controller certificate. The explicit context form is:

```cpp
Poco::Net::Context::Ptr context =
    new Poco::Net::Context(
        Poco::Net::Context::CLIENT_USE,
        "", "", "",
        Poco::Net::Context::VERIFY_NONE);

abb::rws::RWSInterface controller(
    "192.168.125.1", 443, "Default User", "robotics",
    RWSVersion::RWS2, context);
```

The old constructors remain available: constructors without a version or SSL context select RWS 1.0, while constructors receiving a `Poco::Net::Context::Ptr` select RWS 2.0.

The default ports are `80` for RWS 1.0 and `443` for RWS 2.0. If a RobotWare 7.x controller uses another secure port, pass `RWSVersion::RWS2` explicitly (or configure the secure listener, for example with `ListenSecure 443` in `appweb.conf`).
For RWS 2.0, mastership is required for write operations (and must be released immediately after the write).
When the RobotWare controller is in **Automatic** mode, an RWS 2.0 client can request mastership directly. In **Manual** mode, the remote client must first obtain Remote Mastership Management Privilege (RMMP) and the request must be approved on the FlexPendant. The example test therefore requires Automatic mode unless the RMMP workflow is implemented and approved.
To check in detail which operations require mastership, refer to the [RWS 2.0 documentation](https://developercenter.robotstudio.com/api/RWS), look for the **“Mastership is required”** note within the function descriptions.

Please note that this package has not been productized, it is provided "as-is" and only limited support can be expected.

### Sketch

The following is a conceptual sketch of how this RWS library can be viewed, in relation to an ABB robot controller as well as the EGM companion library mentioned above. The optional *StateMachine Add-In* is related to the robot controller's RAPID program and system configuration.

![RWS sketch](docs/images/rws_sketch.png)

### Running the RWS test on Windows

The CMake build copies the POCO, OpenSSL, and zlib runtime DLLs beside `rws_test.exe`. Therefore the test can be launched directly from `build/bin` after `pixi run build`, without manually editing `PATH`.
### Requirements

* RobotWare version `6.x` (for RWS `1.0`) or `7.x` (for RWS `2.0`).

### Dependencies

* [POCO C++ Libraries](https://pocoproject.org) (`>= 1.4.3` due to WebSocket support)

### Limitations

RWS provides access to several services and resources in the robot controller, and this library currently support the following:

* Reading/writing of IO-signals.
* Reading/writing of RAPID data.
* Reading of RAPID data properties.
* Starting/stopping/resetting the RAPID program.
* Subscriptions (i.e. receiving notifications when resources are updated).
* Uploading/downloading/removing files.
* Checking controller state (e.g. motors on/off, auto/manual mode and RAPID execution running/stopped).
* Reading the joint/Cartesian values of a mechanical unit.
* Register as a local/remote user (e.g. for interaction during manual mode).
* Turning the motors on/off.
* Reading of current RobotWare version and available tasks in the robot system.
*	Enable/disable lead-through.
*	Access to SmartGripper functionality.

### Recommendations

* This library has been verified to work with RobotWare `6.15.01` and `7.18.2`. Other versions are expected to work, but this cannot be guaranteed at the moment.
* It is a good idea to perform RobotStudio simulations before working with a real robot.
* It is prudent to familiarize oneself with general safety regulations (e.g. described in ABB manuals).
* Consider cyber security aspects, before connecting robot controllers to networks.

## Usage Hints

This is a generic library, which can be used together with any RAPID program and system configuration. The library's primary classes are:

* [POCOClient](include/abb_rws/rws_poco_client.h): Sets up and manages HTTP and WebSocket communication and is unaware of the RWS protocol.
* [RWSClient](include/abb_rws/rws_client.h): Inherits from `POCOClient` and provides interaction methods for using the RWS services and resources.
* [RWSInterface](include/abb_rws/rws_interface.h): Encapsulates an `RWSClient` instance and provides more user-friendly methods for using the RWS services and resources.
* [RWSStateMachineInterface](include/abb_rws/rws_state_machine_interface.h): Inherits from `RWSInterface` and has been designed to interact with the aforementioned *StateMachine Add-In*. The interface knows about the custom RAPID variables and routines, as well as system configurations, loaded by the RobotWare Add-In.

The optional *StateMachine Add-In* for RobotWare can be used in combination with any of the classes above, but it works especially well with the `RWSStateMachineInterface` class.

### StateMachine Add-In [Optional]

The purpose of the RobotWare Add-In is to *ease the setup* of ABB robot controllers. It is made for both *real controllers* and *virtual controllers* (simulated in RobotStudio). If the Add-In is selected during a RobotWare system installation, then the Add-In will load several RAPID modules and system configurations based on the system specifications (e.g. number of robots and present options).

The RAPID modules and configurations constitute a customizable, but ready to run, RAPID program which contains a state machine implementation. Each motion task in the robot system receives its own state machine instance, and the intention is to use this in combination with external systems that require interaction with the robot(s). The following is a conceptual sketch of the RAPID program's execution flow.

<p align="center">
  <img src="docs/images/statemachine_addin_sketch.png" width="500">
</p>

To install the Add-In:

1. Go to the *Add-Ins* tab in RobotStudio.
2. Search for *StateMachine Add-In* in the *RobotApps* window.
3. Select the desired Add-In version and retrieve it by pressing the *Add* button.
4. Verify that the Add-In was added to the list *Installed Packages*.
5. The Add-In should appear as an option during the installation of a RobotWare system.

See the Add-In's user manual ([1.0](https://robotapps.blob.core.windows.net/appreferences/docs/27e5bd15-b5ec-401d-986a-30c9d2934e97UserManual.pdf) or [1.1](https://robotapps.blob.core.windows.net/appreferences/docs/cd504500-80e2-4cb6-9419-c60ea4ad6d56UserManual.pdf)) for more details, as well as for install instructions for RobotWare systems. The manual can also be accessed by right-clicking on the Add-In in the *Installed Packages* list and selecting *Documentation*.

## Acknowledgements

This work is based on the [abb_librws](https://github.com/ros-industrial/abb_librws) classes developed by Jon Tjerngren for ABB IRC5 controllers (for RWS `1.0`, running RobotWare `6.x`) and on the [abb_librws](https://github.com/JOiiNT-LAB/abb_wrapper/tree/master/abb_librws) classes developed by JOiiNT-LAB for ABB controllers (for RWS `2.0`, running RobotWare `7.x`).

The following acknowledgements are from the original `abb_librws` library:

> The **core development** has been supported by the European Union's Horizon 2020 project [SYMBIO-TIC](http://www.symbio-tic.eu/).
> The SYMBIO-TIC project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreement no. 637107.
>
> <img src="docs/images/symbio_tic_logo.png" width="250">
>
> The **open-source process** has been supported by the European Union's Horizon 2020 project [ROSIN](http://rosin-project.eu/).
> The ROSIN project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreement no. 732287.
>
> <img src="docs/images/rosin_logo.png" width="250">
>
> The opinions expressed reflects only the author's view and reflects in no way the European Commission's opinions.
> The European Commission is not responsible for any use that may be made of the contained information.
>
> Special thanks to [gavanderhoorn](https://github.com/gavanderhoorn) for guidance with open-source practices and ROS-Industrial conventions.

## License

BSD 3-Clause, see [LICENSE](LICENSE). The copyright notices of ABB Schweiz AG and of the RWS 2.0 contributors must be kept in redistributions.
