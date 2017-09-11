var ajatation = require('../');

var device = ajatation.getFirstDevice();
var driver = ajatation.deviceVersion();

console.log("First device:   " + device);
console.log("Driver version: " + driver);
