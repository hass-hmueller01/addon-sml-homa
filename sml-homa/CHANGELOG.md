<!-- https://developers.home-assistant.io/docs/add-ons/presentation#keeping-a-changelog -->
# Changelog

All changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/),
and this project adheres to [Semantic Versioning](https://semver.org/).

## [Unreleased]
### Changed
- Updated log output of sml2mqtt

## [1.0.0] - 2025-12-08
- Removed armhf, armv7, i386 architectures as removed in home-assistant/builder 2025.11.0
- Removed sml.transport_listen debug outputs
- Added error output if sml.transport_listen() fails

## [0.1.5] - 2025-10-02
- Updated sml.transport_listen behavior to reconnect without stopping sml2mqtt
- Added and updated sml_read() function to avoid 100% CPU load in case of disconnect

## [0.1.4] - 2025-07-21

## [0.1.3] - 2025-07-18

## [0.1.2] - 2025-07-17

## [0.1.0] - 2025-07-13
- Initial release
