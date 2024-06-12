# Project OpenHeart

Welcome to Project OpenHeart, your one-stop-shop kernel operating table.
Modifying and examining the operating kernel (as well as non-operating)
has always been a job for the command line, reserved for the expert surgeons
of the GNU/Linux world. This project provides access to the kernel in a
user-friendly and guided way, so that the kernel is no longer different
from any other application.

## Features

OpenHeart (will) provide the following operations for the user:
- Examine running kernel diagnostics
- Examine active (running) and passive kernel data (such as build time, parameters)
- View kernel parameters
- Modify kernel parameters
- View loaded modules
- Modify modules and their parameters
- Active new kernel and provide reboot action

## Project structure

OpenHeart Core is the underlying library, that is implemented, using

| Library | Version |
| ------- | ------- |
| GLib    | 2.79    |
| Curl    | 8.1.0   |
| GoogleTest | undecided |
| GoogleMock | undecided |
| Meson      | 1.4.0     |

## Documentation

To be extended. For now, execute `doxygen .` while in project root

## Development setup and build