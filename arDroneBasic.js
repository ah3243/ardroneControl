    // Parent Program
    var iteration = 0;
    var exec  = require('child_process').exec;

    // for emergancy
    var keypress = require('keypress');

    // PNG Vars
    var arDrone = require('ar-drone');
    var client = arDrone.createClient();
    var fs = require('fs');

    var pngStream = client.getPngStream();


    // Object to hold main vars and parameters
    var droneState = {
              cont: 0, // Flag to ensure only one image test at a time
              nav:0, // Var to store nav return values
              test:true, // True == Drone will not fly, False == Drone will fly
              fwdSpeed:0, // Param to govern speed of forward movement
              rotSpeed:0,  // Param to govern speed of rotation
              imgFrameNum:0, // Current frame number (used for naming)
              // methods to increase or decrease cont flag value
              incre:function(){
                console.log("cont Incrementing");
                this.cont +=1;
              },
              decre:function(){
                console.log("cont Decrementing");
                this.cont -=1;
              },
              // Increment current Frame Number by 1
              frameNumIncre:function(){
                this.imgFrameNum +=1;
              }
            };

    //////////////////////////////////////////////////////
    //////////////////// FUNCTIONS ///////////////////////
    //////////////////////////////////////////////////////
    function setNav(droneState, output, client){
      droneState.nav = output;
      console.log('This is the output:..' + output + ' and DroneState.nav: ' + droneState.nav);
      if(output==0){
        console.log("This is the Value with 0: " + output);
        if(!droneState.test){
          client.after(2000, function(){
            this.stop();
            this.land();
          });
        }
      }
      else if(output==1){
        console.log("\n\nVALIDATING..\n\n");
        if(!droneState.test){
          client.after(2000, function(){
            this.stop();
            this.land();
          });
        }
      }
      else if(output==2){
        console.log("\n\nGoing FORWARD..\n\n");
        if(!droneState.test){
          client.after(1000, function(){
            this.forward(droneState.fwdSpeed);
          });
        }
      }
      else if(output==3){
        console.log("\n\nGoing LEFT..\n\n");
        if(!droneState.test){
          client.after(500, function(){
            this.left(droneState.rotSpeed);
          });
        }
      }
      else if(output==4){
        console.log("\n\nGoing RIGHT..\n\n");
        if(!droneState.test){
          client.after(500, function(){
            this.right(droneState.rotSpeed);
          });
        }
      }
      else{
        console.log("\n\nUNKOWN LANDING..\n\nTHIS is the output:" + output);
        if(!droneState.test){
          client.after(2000, function(){
            this.stop();
            this.land();
          });
        }
      }
    }

    // Set number of processes flag after Opencv script completion
    function extraArgs(droneState, stdout, stderr, client, callback){
      setNav(droneState, stdout, client);
      console.log('Script stderr:..' + stderr);
      console.log('Script stdout: ' + stdout);
      droneState.decre();
    }
    // Run Opencv Script decrementing the number of current processes running by 1 when completed
    function opencvScript(exec, imagePath, droneState, client, callback){
      // Call the shell script
      exec('sh ~/Desktop/MyFilterbankCode/ARDRONE/OpenCVMod/9_Testing.sh ' + imagePath, function(error, stdout, stderr){
        if(error += null){
          console.log('exec ERROR: ' + error);
        }

        // Run callback when script is done
        extraArgs(droneState ,stdout, stderr, client, callback);
      });
    }

    // Get a current png image from stream and pass path to and run opencv script
    function startProgram(exec, pngBuffer, droneState, client){
      // increment number of processes running by one
      droneState.incre();

      // Generate imgFileName
      var imageDir = '/home/james-tt/Desktop/MyFilterbankCode/ARDRONE/'; // Image FolderPath partial
      var imageName = imageDir + 'Images/frame' + droneState.imgFrameNum + '.png';
      // Write file to dir
      fs.writeFile(imageName, pngBuffer, function(err) {
        if (err) {
          console.log('Error saving PNG: ' + err);
        }
        // Start opencv Script
        opencvScript(exec, imageName, droneState, client);
      });
    };

    function RunProgram(exec, pngStream, droneState, client, callback){
      exec('sh ~/Desktop/MyFilterbankCode/ARDRONE/prepScript.sh ', function(err, stdout, stderr){
        console.log('PREPSCRIPT_stdout: ' + stdout);
        if(err){
          console.log('PREPSCRIPT_ERROR: ' + err);
        }
        pngStream
          .on('error', console.log)
          .on('data', function(pngStream, something) {
              if(droneState.cont==0){
                console.log('\nNEWTEST, cont= ' + droneState.cont);
                droneState.frameNumIncre();
                startProgram(exec, pngStream, droneState, client);
            }
          });
      });
    }

    //////////////////////////////////////////////////////
    //////////////////// PROGRAM /////////////////////////
    //////////////////////////////////////////////////////

    // Takeoff
    if(!droneState.test){
      console.log('TAKING OFF!!\n');
      client.takeoff();
      client.after(3000, function(){
        this.stop();
      });
    }

    RunProgram(exec, pngStream, droneState, client, startProgram);
