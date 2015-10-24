    // Parent Program
    var iteration = 0;
    var exec  = require('child_process').exec;

    // for emergancy
    var keypress = require('keypress');

    // PNG Vars
    var arDrone = require('ar-drone');
    var client = arDrone.createClient();
    var fs = require('fs');

    client.disableEmergency();

    var pngStream = client.getPngStream();
    var frameCounter = 0;


    var ref = {value: 0,
              nav:0,
              test:true,
              fwdDis:0,
              rotAng:0};

    //////////////////////////////////////////////////////
    //////////////////// FUNCTIONS ///////////////////////
    //////////////////////////////////////////////////////
    function setNav(x, output){
      x.nav = output;
      console.log('This is the output:..' + x.nav);
    }

    // Increment input objects value by 1
    function incre(x){
      x.value++;
    }
    // Decrement input objects value by 1
    function decre(x){
      x.value--;
    }

    // Set number of processes flag after Opencv script completion
    function extraArgs(ref, stdout, stderr, callback){
//      setNav(ref, stdout);
      console.log('Script stderr:..' + stderr);
      console.log('Script stdout: ' + stdout);
      decre(ref);

    }
    // Run Opencv Script decrementing the number of current processes running by 1 when completed
    function opencvScript(exec, imagePath, ref, callback){
      // Call the shell script
      exec('sh ~/Desktop/MyFilterbankCode/ARDRONE/OpenCVMod/9_Testing.sh ' + imagePath, function(error, stdout, stderr){
        if(error += null){
          console.log('exec ERROR: ' + error);
        }

        // Run callback when script is done
        extraArgs(ref ,stdout, stderr, callback);
      });
    }

    // Get a current png image from stream and pass path to and run opencv script
    function startProgram(exec, pngBuffer, frameCounter, ref){
      // increment number of processes running by one
      incre(ref);

      // Generate imgFileName
      var imageDir = '/home/james-tt/Desktop/MyFilterbankCode/ARDRONE/'; // Image FolderPath partial
      var imageName = imageDir + 'Images/frame' + frameCounter + '.png';
      // Write file to dir
      fs.writeFile(imageName, pngBuffer, function(err) {
        if (err) {
          console.log('Error saving PNG: ' + err);
        }
        // Start opencv Script
        opencvScript(exec, imageName, ref);
      });
    };

    //////////////////////////////////////////////////////
    //////////////////// PROGRAM /////////////////////////
    //////////////////////////////////////////////////////


    exec('sh ~/Desktop/MyFilterbankCode/ARDRONE/prepScript.sh ', function(err, stdout, stderr){
      console.log(stdout);
      if(err){
        console.log('ERROR with prepScript: ' + err);
      }
      console.log(stdout);
      pngStream
        .on('error', console.log)
        .on('data', function(pngBuffer, something) {
            if(ref.value==0){
              console.log('\nNEWTEST');
              startProgram(exec, pngBuffer, frameCounter, ref);
              frameCounter++;
          }
        });
    });
