// Assign requirements
var fs = require('fs'),
   process = require('child_process'),
   input = 0,
   iteration = 0;

while(true){
  try{
    var script = process.spawnSync('sh', ['myscript.sh']);
    console.log("the status code is:" + script.status);
    input = script.status;
    console.log("This is the savedStatus code: " + input);
    var imageCol = process.spawnSync('node', ['savePNG.js']);
    imageName = imageCol.status;
  }
  catch(err){
    console.error(err.stack);
  }
  console.log("Another loop iteration" + iteration);

iteration++;
}
