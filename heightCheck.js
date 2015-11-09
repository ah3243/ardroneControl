var exec = require('child_process').exec;

// for emergancy
var keypress = require('keypress');

keypress(process.stdin);

var setHeight = function (targetAlt, client){
  console.log("Entering SetHeight\n\n");
  // Get current height
  client.on('navdata', function(a){
    var curAlt = a.demo.altitude;
    // If target height is below current height decend until false
    console.log("curAlt: " + curAlt + " targetAlt: " + targetAlt);
    if(curAlt>targetAlt){
      console.log("\nNavigating Down!\nCurrAlt: " + curAlt + "\nTarget Alt: " + targetAlt + "\n");
      var counter = 0;
      // Decend until false
      while(curAlt>targetAlt && counter<1000){
        console.log("curAlt: " + curAlt + " targetAlt: " + targetAlt);
        client.down(0.3);
//        client.stop(500);
        counter++;
      }
    }
  });
}

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
  'g': function(){
    console.log('reducing altitude to 0.3m!');
    setHeight(0.3, client);
  },
  // // Height 1
  // 'q': function(){
  //   console.log('Move Forward!');
  //   client.front(0.1);
  // },
  // // Height 2
  // 'w': function(){
  //   console.log('Move Forward!');
  //   client.front(0.1);
  // },
  // // Height 3
  // 'e': function(){
  //   console.log('Move Forward!');
  //   client.front(0.1);
  // },
  // 'up': function(){
  //   console.log('Move Forward!');
  //   client.front(0.1);
  // },
  // 'down': function(){
  //   console.log('Move Backwards!');
  //   client.back(0.1);
  // },
  // 'right': function(){
  //   console.log('Move Left!');
  //   client.right(0.1);
  // },
  // 'left': function(){
  //   console.log('Move Right!');
  //   client.left(0.1);
  // },
  // 'pageup': function(){
    // console.log('Move Up!');
  //   // client.up(0.5);
  // },
  'pagedown': function(){
    console.log('Move Down!');
    client.down(0.1);
  }
}
// In emergancy quit land drone and exit
var quit = function(client){
  console.log('Quitting');
  process.stdin.pause();

  client.stop();
  client.land();
//  client._udpControl.close();
}

// PNG Vars
var arDrone = require('ar-drone');
var client = arDrone.createClient();
var fs = require('fs');

client.on('navdata', function(d) {
// console.log(d);
  // console.log(d.droneState);
  // Use if statement to allow access to ar drone parameters
  if (d.demo) {
    if(d.demo){
      // console.log('Battery Percentage: ' + d.demo.batteryPercentage);
      if(d.demo.altitude!=0){
        console.log('Altitude: ' + d.demo.altitude);
        console.log('Altitude(m): ' + d.demo.altitudeMeters);
      }
    }
  }
});

process.stdin.on('keypress', function (ch, key) {
  console.log('got "keypress"', key.name);
  if(key && keys[key.name]){ keys[key.name](); }
  if(key && key.ctrl && key.name == 'c') { quit(); }
});

process.stdin.setRawMode(true);
process.stdin.resume();
