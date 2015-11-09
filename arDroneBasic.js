    // Parent Program
    var iteration = 0;
    var exec  = require('child_process').exec;

    // for emergancy
    var keypress = require('keypress');

    keypress(process.stdin);

    var keys = {
      'space': function(){
        console.log('Takeoff!');
       client.takeoff();
      },
      'l': function(){
        console.log('Land!');
        client.stop();
        client.land();
      },
      'up': function(){
        console.log('Move Forward!');
        client.front(0.1);
      },
      'down': function(){
        console.log('Move Backwards!');
        client.back(0.1);
      },
      'right': function(){
        console.log('Move Left!');
        client.right(0.1);
      },
      'left': function(){
        console.log('Move Right!');
        client.left(0.1);
      },
      'pageup': function(){
        console.log('Move Down!');
        // client.up(0.5);
      },
      'pagedown': function(){
        console.log('Move Up!');
        client.down(0.5);
      }
    }
    // In emergancy quit land drone and exit
    var quit = function(client){
      console.log('Quitting');
      process.stdin.pause();

      client.stop();
      client.land();
      client._udpControl.close();
    }


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
              fwdSpeed:0.02, // Param to govern speed of forward movement
              rotSpeedLeft:0.1, // Param to govern left speed of lateral movement
              rotSpeedRight:0.02,  // Param to govern right speed or lateral movement
              imgFrameNum:0, // Current frame number (used for naming)
              // methods to increase or decrease cont flag value
              incre:function(){
                this.cont +=1;
              },
              decre:function(){
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

    // Execute specific drone maneuvers depending on analysis output
    function setNav(droneState, output, client){
      droneState.nav = output;
      var forwardSpeed = droneState.fwdSpeed;
      var rotSpeedL = droneState.rotSpeedLeft;
      var rotSpeedR = droneState.rotSpeedRight;
      console.log('This is the Lateral movement Right: ' + rotSpeedR + ' Lateral movement Left: ' + rotSpeedL);
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
            this.front(forwardSpeed);
          })
          .after(3000, function() {
            this.stop();
          });
        }
      }
      else if(output==3){
        console.log("\n\nGoing LEFT..\n\n");
        if(!droneState.test){
          client.after(1000, function(){
            this.left(rotSpeedL);
          })
          .after(3000, function() {
            this.stop();
          });
        }
      }
      else if(output==4){
        console.log("\n\nGoing RIGHT..\n\n");
        if(!droneState.test){
          client.after(1000, function(){
            this.right(rotSpeedR);
          })
          .after(3000, function() {
            this.stop();
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

    function manNav(client, keypress){
      process.stdin.on('keypress', function (ch, key) {
        // console.log('got "keypress"', key.name);
        if(key && keys[key.name]){ keys[key.name](); }
        if(key && key.ctrl && key.name == 'c') { quit(); }
      });

      process.stdin.setRawMode(true);
      process.stdin.resume();
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
        var battery;
        // Output nav Data
        getHeight(client, battery);
        // Start opencv Script
        opencvScript(exec, imageName, droneState, client);
      });
    };

    // Process exit callback function
    function end(){
      process.exit(1);
    }
    // Check that forward speed is below certain value
    function speedCheck(droneState, callback){
      if(droneState.fwdSpeed>0.1){
        console.log("\nWARNING: Forward value exceeding 0.1 limit.\nExiting.\n\n");
        client.after(3000, function(){
          this.stop();
        })
        .after(1000, function(){
          this.land();
        });
        callback();
      }
    }
  function getHeight(client, x){
    // Return ar drone parameters
    client.on('navdata', function(d) {
      // console.log(d);
      // Use if statement to allow access to ar drone parameters
      if (d.demo) {
        if(d.demo){
          // console.log('Battery Percentage: ' + d.demo.batteryPercentage);
          // console.log('Altitude: ' + d.demo.altitude);
          // console.log('Altitude(m): ' + d.demo.altitudeMeters);
        }
      }
    });
  }

    // Run main program
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

    // Manual Control
    manNav(client, keypress);

    // Validate that forward speed is within safe range
    speedCheck(droneState, end);

    // Takeoff if not a static test
    if(!droneState.test){
      console.log('TAKING OFF!!\n');
      client.takeoff();
      client.after(3000, function(){
        this.stop();
      });
    }

     // Run main program
   RunProgram(exec, pngStream, droneState, client, startProgram);
