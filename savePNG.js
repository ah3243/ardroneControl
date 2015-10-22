
    var arDrone = require('ar-drone');
    var client = arDrone.createClient();
    var fs = require('fs');

    var pngStream = client.getPngStream();
    var frameCounter = 0;
    var saveDir = './Images/'; // Image Dir

    pngStream
      .on('error', console.log)
      .on('data', function(pngBuffer) {
        // Exit after first image saved
        if(frameCounter!=0){
          process.exit();
        }

        // Create ImageName and save to dir
        var imageName = saveDir + 'frame' + '.png';
        fs.writeFile(imageName, pngBuffer, function(err) {
          if (err) {
            console.log('Error saving PNG: ' + err);
          }
        });
        console.log(imageName); // Output imagePath for use in parent

        frameCounter++;
      });
