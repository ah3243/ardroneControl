var arDrone = require('ar-drone');
var client = arDrone.createClient();
var fs = require('fs');

var pngStream = client.getPngStream();
var frameCounter = 0;
var period = 0; // Save a frame every 5000 ms.
var lastFrameTime = 0;
var saveDir = './Images/';

pngStream
  .on('error', console.log)
  .on('data', function(pngBuffer) {
    var now = (new Date()).getTime();
    if (now - lastFrameTime > period) {
      lastFrameTime = now;
      imageName = saveDir + 'frame' + frameCounter + '.png';
      console.log(imageName);
      fs.writeFile(imageName, pngBuffer, function(err) {
        if (err) {
          console.log('Error saving PNG: ' + err);
        }
      });
      frameCounter++; // increment the imageNumber
    }
  });
