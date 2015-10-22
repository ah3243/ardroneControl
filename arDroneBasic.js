    // Parent Program
    var iteration = 0;

    function startProgram(){
      var exec  = require('child_process').exec;
        exec('sh myscript.sh', function(error, stdout, stderr){
          console.log('stdout: ' + stdout);
          console.log('stderr: ' + stderr);
          if(error += null){
            console.log('exec ERROR: ' + error);
          }

          console.log('Getting Image');
          exec('node savePNG.js',function(error, stdout, stderr){
            console.log('IMage collected..');
          });
        });
    }
    setInterval(function(){
      startProgram();
    }, 5000);
