# Ajatation

Prototype bindings to link [Node.js](http://nodejs.org/) and the Aja Kona NTV2 SDK, enabling asynchronous capture and playback to and from [Aja Kona IP](https://www.aja.com/products/kona-ip) devices via a simple Javascript API.

This is prototype software and is not yet suitable for production use. Currently supported platforms are Mac and Windows.

## Installation

Ajatation has a number of prerequisites:

1. Install [Node.js](http://nodejs.org/) for your platform. This software has been developed against the long term stable (LTS) release.
2. Install the latest version of the Aja NTV2 SDKs for your platform, available from https://sdksupport.aja.com.
3. Install [node-gyp](https://github.com/nodejs/node-gyp) and make sure that you have the prerequisites for compiling Javascript addons for your platform. This requires a C/C++ development kit and python v2.7.

Ajatation is designed to be used as a module included into another project. To include ajatation into your project:

    npm install --save ajatation (TODO: validate this works)

## Building

TODO: write better build instructions - these instructions have holes in, but are written to at least point you in the right direction...

1) Place the AJA SDK in a directory 'aja' under the ajatation root directory, so that the directory structure looks as follows:

ajatation
|-aja
|	|-ntv2sdkwin_13.0.0.18
|-build
|-scratch
|-src

2) Build the AJA SDK solution using Visual Studio 2013 Community Edition

3) From the ajatation root directory run the following:

	node-gyp configure --msvs_version=2013
	node-gyp build

## Using Ajatation

Currently the only tested way to use this is to run:

	cd scratch
	node play.js

TODO: Add further details

## License

This software is released under the Apache 2.0 license. Copyright 2017 Streampunk Media Ltd.

The software links to the BlackMagic Desktop Video libraries. Include files and examples from which this code is derived include the BlackMagic License in their respective header files. The BlackMagic DeckLink SDK can be downloaded from https://www.blackmagicdesign.com/support.
