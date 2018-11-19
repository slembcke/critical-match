
var CANVAS_ID="cnvs";

function BUTTON() {
}

BUTTON.A = 0;
BUTTON.B = 1;
BUTTON.SELECT = 2;
BUTTON.START = 3;
BUTTON.UP = 4;
BUTTON.DOWN = 5;
BUTTON.LEFT = 6;
BUTTON.RIGHT = 7;

function KEYCODE() {
}

KEYCODE.LEFT = 37;
KEYCODE.UP = 38;
KEYCODE.RIGHT = 39;
KEYCODE.DOWN = 40;

KEYCODE.A = 70;
KEYCODE.B = 68;
KEYCODE.SELECT = 65;
KEYCODE.START = 83;

function RingBuffer(capacity, evictedCb) {
  this._elements = new Array(capacity || 50);
  this._first = 0;
  this._last = 0;
  this._size = 0;
  this._evictedCb = evictedCb;
}

RingBuffer.prototype.capacity = function() {
  return this._elements.length;
};

RingBuffer.prototype.isEmpty = function() {
  return this.size() === 0;
};

RingBuffer.prototype.isFull = function() {
  return this.size() === this.capacity();
};

RingBuffer.prototype.peek = function() {
  if (this.isEmpty()) throw new Error('RingBuffer is empty');

  return this._elements[this._first];
};

RingBuffer.prototype.peekN = function(count) {
  if (count > this._size) throw new Error('Not enough elements in RingBuffer');

  var end = Math.min(this._first + count, this.capacity());
  var firstHalf = this._elements.slice(this._first, end);
  if (end < this.capacity()) {
    return firstHalf;
  }
  var secondHalf = this._elements.slice(0, count - firstHalf.length);
  return firstHalf.concat(secondHalf);
};

RingBuffer.prototype.deq = function() {
  var element = this.peek();

  this._size--;
  this._first = (this._first + 1) % this.capacity();

  return element;
};

RingBuffer.prototype.deqN = function(count) {
  var elements = this.peekN(count);

  this._size -= count;
  this._first = (this._first + count) % this.capacity();

  return elements;
};

RingBuffer.prototype.enq = function(element) {
  this._end = (this._first + this.size()) % this.capacity();
  var full = this.isFull()
  if (full && this._evictedCb) {
    this._evictedCb(this._elements[this._end]);
  }
  this._elements[this._end] = element;

  if (full) {
    this._first = (this._first + 1) % this.capacity();
  } else {
    this._size++;
  }

  return this.size();
};

RingBuffer.prototype.size = function() {
  return this._size;
};
//---
function onBufferUnderrun(actualSize, desiredSize) {
    /*
    if (!running || paused) {
        return;
    }
    */
      // Skip a video frame so audio remains consistent. This happens for
      // a variety of reasons:
      // - Frame rate is not quite 60fps, so sometimes buffer empties
      // - Page is not visible, so requestAnimationFrame doesn't get fired.
      //   In this case emulator still runs at full speed, but timing is
      //   done by audio instead of requestAnimationFrame.
      // - System can't run emulator at full speed. In this case it'll stop
      //    firing requestAnimationFrame.
      console.log(
        "Buffer underrun, running another frame to try and catch up"
      );
      nes.frame();
      // desiredSize will be 2048, and the NES produces 1468 samples on each
      // frame so we might need a second frame to be run. Give up after that
      // though -- the system is not catching up
      if (audioBuffer.size() < desiredSize) {
        console.log("Still buffer underrun, running a second frame");
        nes.frame();
      }
    }


var audioCtx;
var scriptNode;

function onAudioProcess( e ) {
    var left = e.outputBuffer.getChannelData(0);
    var right = e.outputBuffer.getChannelData(1);
    var size = left.length;

    // We're going to buffer underrun. Attempt to fill the buffer.
    if (audioBuffer.size() < size * 2 && onBufferUnderrun) {
      onBufferUnderrun(audioBuffer.size(), size * 2);
    }

    try {
      var samples = audioBuffer.deqN(size * 2);
    } catch (e) {
      // onBufferUnderrun failed to fill the buffer, so handle a real buffer
      // underrun

      // ignore empty buffers... assume audio has just stopped
      var bufferSize = audioBufferuffer.size() / 2;
      if (bufferSize > 0) {
        console.log(`Buffer underrun (needed ${size}, got ${bufferSize})`);
      }
      for (var j = 0; j < size; j++) {
        left[j] = 0;
        right[j] = 0;
      }
      return;
    }
    for (var i = 0; i < size; i++) {
      left[i] = samples[i * 2];
      right[i] = samples[i * 2 + 1];
    }

}

var SCREEN_WIDTH = 256;
var SCREEN_HEIGHT = 240;
var imageData = null;
var buf;
var buf8;
var buf32;
var canvas;
var ctx;

var audioBuffer;
var audioBufferSize = 8192;

function startAudio() {
    if (!window.AudioContext) {
        return;
      }
      audioCtx = new window.AudioContext();
      scriptNode = audioCtx.createScriptProcessor(1024, 0, 2);
      scriptNode.onaudioprocess = onAudioProcess;
      scriptNode.connect(audioCtx.destination);  

      audioBufferSize = 8192;
      audioBuffer = new RingBuffer(audioBufferSize * 2);
  
}

startAudio();

var nes = new jsnes.NES({
        onFrame: function(frameBuffer) {
            buf = frameBuffer;
            var i = 0;
            for (var y = 0; y < SCREEN_HEIGHT; ++y) {
                for (var x = 0; x < SCREEN_WIDTH; ++x) {
                    i = y * SCREEN_WIDTH + x;
                    // Convert pixel from NES BGR to canvas ABGR
                    buf32[i] = 0xff000000 | buf[i]; // Full alpha
                }
            }
        },
        onAudioSample: function(left, right) {
            if (audioBuffer.size() / 2 >= audioBufferSize) {
                console.log(`Buffer overrun`);
            }
            audioBuffer.enq(left);
            audioBuffer.enq(right);        
        }

    });        
    
function onAnimationFrame() {
    window.requestAnimationFrame(onAnimationFrame);
    imageData.data.set(buf8);
    ctx.putImageData(imageData, 0, 0);
    nes.frame();
}


function loadBinary(path, callback, handleProgress) {

    var req = new XMLHttpRequest();
    req.open("GET", path);
    req.overrideMimeType("text/plain; charset=x-user-defined");
    req.onload = function() {
        if (this.status === 200) {
            callback(null, this.responseText);
        } else if (this.status === 0) {
        // Aborted, so ignore error
        } else {
            callback(new Error(req.statusText));
        }
    };
    req.onerror = function() {
        callback(new Error(req.statusText));
    };
    req.onprogress = handleProgress;
    req.send();
    return req;
}

function handleLoaded( data ) {

    // INITIALIZE CANVAS
    canvas = document.getElementById(CANVAS_ID);
    ctx = canvas.getContext("2d");
    imageData = ctx.getImageData(   0,
                                    0,
                                    SCREEN_WIDTH,
                                    SCREEN_HEIGHT
                                    );

    ctx.fillStyle = "black";
    // set alpha to opaque
    ctx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // buffer to write on next animation frame
    buf = new ArrayBuffer(imageData.data.length);
    // Get the canvas buffer in 8bit and 32bit
    buf8 = new Uint8ClampedArray(buf);
    buf32 = new Uint32Array(buf);

    // Set alpha
    for (var i = 0; i < buf32.length; ++i) {
    buf32[i] = 0xff000000;
    }

    nes.loadROM(data);
    window.requestAnimationFrame(onAnimationFrame);
    //this.start();
}

function handleProgress( e ) {
    if (e.lengthComputable) {
        //this.setState({ loadedPercent: e.loaded / e.total * 100 });
        console.log("loadPct=" + (e.loaded / e.total * 100) );
    }
}

function keyboardDown(event) {
    // PRESS LEFT ARROW
    if (event.keyCode == KEYCODE.LEFT) {
       nes.buttonDown( 1, BUTTON.LEFT);
    }
    // PRESS UP ARROW
    if (event.keyCode == KEYCODE.UP) {
        nes.buttonDown( 1, BUTTON.UP);
    }
    // PRESS RIGHT ARROW
    if (event.keyCode == KEYCODE.RIGHT) {
        nes.buttonDown( 1, BUTTON.RIGHT);
    }
    // PRESS DOWN ARROW
    if (event.keyCode == KEYCODE.DOWN) {
        nes.buttonDown( 1, BUTTON.DOWN);
    }

    if (event.keyCode == KEYCODE.START) {
        nes.buttonDown( 1, BUTTON.START);
    }
    if (event.keyCode == KEYCODE.SELECT) {
        nes.buttonDown( 1, BUTTON.SELECT);
    }
    if (event.keyCode == KEYCODE.A) {
        nes.buttonDown( 1, BUTTON.A);
    }
    if (event.keyCode == KEYCODE.B) {
        nes.buttonDown( 1, BUTTON.B);
    }
 }

 function keyboardUp(event) {
    // PRESS LEFT ARROW
    if (event.keyCode == KEYCODE.LEFT) {
        nes.buttonUp( 1, BUTTON.LEFT);
    }
    // PRESS UP ARROW
    if (event.keyCode == KEYCODE.UP) {
        nes.buttonUp( 1, BUTTON.UP);
    }
    // PRESS RIGHT ARROW
    if (event.keyCode == KEYCODE.RIGHT) {
        nes.buttonUp( 1, BUTTON.RIGHT);
    }
    // PRESS DOWN ARROW
    if (event.keyCode == KEYCODE.DOWN) {
        nes.buttonUp( 1, BUTTON.DOWN);
    }

    if (event.keyCode == KEYCODE.START) {
        nes.buttonUp( 1, BUTTON.START);
    }
    if (event.keyCode == KEYCODE.SELECT) {
        nes.buttonUp( 1, BUTTON.SELECT);
    }
    if (event.keyCode == KEYCODE.A) {
        nes.buttonUp( 1, BUTTON.A);
    }
    if (event.keyCode == KEYCODE.B) {
        nes.buttonUp( 1, BUTTON.B);
    }
 }

function initNES( nes_file, canvas_id ) {
    CANVAS_ID = canvas_id;
    var currentRequest = loadBinary(
        nes_file,
        (err, data) => {
            if (err) {
                window.alert(`Error loading ROM: ${err.toString()}`);
            } else {
                handleLoaded(data);
            }
        },
        handleProgress
    );
}

document.addEventListener('keydown', keyboardDown);
document.addEventListener('keyup', keyboardUp);
