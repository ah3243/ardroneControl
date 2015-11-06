var keypress = require('keypress');
var arDrone = require('ar-drone');

var client = arDrone.createClient();

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
    client.left(0.1);
  },
  'left': function(){
    console.log('Move Right!');
    client.right(0.1);
  },
  'pageup': function(){
    console.log('Move Down!');
    client.down(0.3);
  },
  'pagedown': function(){
    console.log('Move Up!');
    // client.up(0.5);
  }
}

var quit = function(){
  console.log('Quitting');
  process.stdin.pause();

  client.stop();
  client.land();
  client._udpControl.close();
}

process.stdin.on('keypress', function (ch, key) {
  // console.log('got "keypress"', key.name);
  if(key && keys[key.name]){ keys[key.name](); }
  if(key && key.ctrl && key.name == 'c') { quit(); }
});

process.stdin.setRawMode(true);
process.stdin.resume();
