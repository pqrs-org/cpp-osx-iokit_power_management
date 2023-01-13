[![Build Status](https://github.com/pqrs-org/cpp-osx-iokit_power_management/workflows/CI/badge.svg)](https://github.com/pqrs-org/cpp-osx-iokit_power_management/actions)
[![License](https://img.shields.io/badge/license-Boost%20Software%20License-blue.svg)](https://github.com/pqrs-org/cpp-osx-iokit_power_management/blob/main/LICENSE.md)

# cpp-osx-iokit_power_management

A wrapper class of IOPMLib.

## Requirements

cpp-osx-iokit_power_management depends the following classes.

- [Nod](https://github.com/fr00b0/nod)
- [pqrs::cf::run_loop_thread](https://github.com/pqrs-org/cpp-cf-run_loop_thread)
- [pqrs::dispatcher](https://github.com/pqrs-org/cpp-dispatcher)
- [pqrs::osx::kern_return](https://github.com/pqrs-org/cpp-osx-kern_return)
- [type_safe](https://github.com/foonathan/type_safe)

## Install

### Using package manager

You can install `include/pqrs` by using [cget](https://github.com/pfultz2/cget).

```shell
cget install pqrs-org/cget-recipes
cget install pqrs-org/cpp-osx-iokit_power_management
```

### Manual install

Copy `include/pqrs` directory into your include directory.
