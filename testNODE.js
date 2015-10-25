// Parent Program
var exec  = require('child_process').exec;

// for emergancy
var keypress = require('keypress');

// PNG Vars
var fs = require('fs');

var ref = {value:4,
          nav:0,
          test:false,
          fwdDis:0,
          rotAng:0,
          increment: function(){
            this.value+=1;
          }};

//////////////////////////////////////////////////////
//////////////////// FUNCTIONS ///////////////////////
//////////////////////////////////////////////////////

function setNav(err, stdout, stderr, ref, callback){
  var output = ref.value;
  if(output=='0'){
    console.log('This is the output:..' + output);
  }
  else if(output=='1'){
    console.log("\n\nVALIDATING..\n\n");
  }
  else if(output=='2'){
    console.log("\n\nGoing FORWARD..\n\n");
  }
  else if(output=='3'){
    console.log("\n\nGoing LEFT..\n\n");
  }else{
    console.log("\n\nUNKOWN LANDING..\n\nTHIS is the output:" + output);
  }
}
function myFunc(exec, ref, callback){
  exec('sh ~/Desktop/MyFilterbankCode/ARDRONE/myScript.sh ', function(err, stdout, stderr){
    setNav(err, stdout, stderr, ref, callback);
  });
}

myFunc(exec, ref, setNav);
