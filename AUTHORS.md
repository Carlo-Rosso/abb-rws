# Authors

`abb-rws` is a hard fork of [ros-industrial/abb_librws](https://github.com/ros-industrial/abb_librws).
The RWS 1.0 library was written by the authors of `abb_librws`; RWS 2.0 support was developed in this fork and in the projects it is based on, as listed below.

## Original library: `abb_librws` (RWS 1.0)

Copyright (c) 2015, ABB Schweiz AG.

* **Jon Tjerngren** (ABB): original author and main developer
* **G.A. vd. Hoorn** (TU Delft Robotics Institute): maintainer of `ros-industrial/abb_librws`, CI and packaging

Other contributors to `ros-industrial/abb_librws`, from its git history:

* Maarten de Vries
* Michael Ripperger
* Martijn Vegter
* Aris Synodinos
* Silvio Traversaro

## RWS 2.0 support

* **JOiiNT LAB**, Fondazione Istituto Italiano di Tecnologia, Intellimech Consorzio per la Meccatronica: RWS 2.0 classes in [JOiiNT-LAB/abb_wrapper](https://github.com/JOiiNT-LAB/abb_wrapper/tree/master/abb_librws) (2021)
* **Gianluca Correnti**, IIT-Danieli Joint Lab, Fondazione Istituto Italiano di Tecnologia: initial RWS 2.0 port of this library (HTTPS sessions, RWS 2.0 resources, mastership, lead-through, SmartGripper IO control) (2025)
* **Carlo Rosso** (maintainer of this fork): RWS 1.0 / 2.0 / automatic version selection API and default SSL context, RWS 2.0 mastership and endpoint fixes (IO signals, subscriptions, speed ratio, robtarget), thread-safety and SmartGripper fixes, RWS test example, package rename (`abb_librws` → `abb_rws`), CMake and conda-forge / RoboStack packaging (2026)

The complete list of commits is in the git history of this repository, which keeps the upstream history.
