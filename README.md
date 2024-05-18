# Bofu Blinds Controller Library

[![Bofu](https://www.ardu-badge.com/badge/Bofu.svg?)](https://www.ardu-badge.com/Bofu)

An arduino library to control wireless blinds.

This started as a fork of the [markisol](https://github.com/akirjavainen/markisol). project. Initially the goal was simply to add a checksum implementations but ended up being mostly a re-implementation. Differences include:

- The code is packaged in an arduino library.
- Protocol messages are stored as integers rather than strings.
- The bit order of messages is reversed. This allows easy checksum calculations.
- Adds the ability to generate and validate checksums.
- Uses interrupts to receive messages rather than polling.

This library takes inspiration and some code from the [rc-switch](https://github.com/sui77/rc-switch). library.