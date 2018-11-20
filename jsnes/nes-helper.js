var BUTTON = {}

BUTTON.A = 0;
BUTTON.B = 1;
BUTTON.SELECT = 2;
BUTTON.START = 3;
BUTTON.UP = 4;
BUTTON.DOWN = 5;
BUTTON.LEFT = 6;
BUTTON.RIGHT = 7;

var KEYCODE = {}

KEYCODE.LEFT = 37;
KEYCODE.UP = 38;
KEYCODE.RIGHT = 39;
KEYCODE.DOWN = 40;

KEYCODE.A = 70;
KEYCODE.B = 68;
KEYCODE.SELECT = 65;
KEYCODE.START = 83;


var scriptNode;

var SCREEN_WIDTH = 256;
var SCREEN_HEIGHT = 240;
var FRAMEBUFFER_SIZE = SCREEN_WIDTH*SCREEN_HEIGHT;

var imageData = null;
var buf;
var buf8;
var buf32;
var canvas;
var ctx;

var nes = new jsnes.NES({
	onFrame: function(frameBuffer) {
		buf = frameBuffer;
		for(var i = 0; i < FRAMEBUFFER_SIZE; i++) buf32[i] = 0xFF000000 | buf[i];
	},
	onAudioSample: function(left, right) {}
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

function handleLoaded(data){
	// INITIALIZE CANVAS
	canvas = document.getElementById(CANVAS_ID);
	ctx = canvas.getContext("2d");
	imageData = ctx.getImageData(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	ctx.fillStyle = "black";
	// set alpha to opaque
	ctx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// buffer to write on next animation frame
	buf = new ArrayBuffer(imageData.data.length);
	// Get the canvas buffer in 8bit and 32bit
	buf8 = new Uint8ClampedArray(buf);
	buf32 = new Uint32Array(buf);

	for(var i = 0; i < buf32.length; ++i) buf32[i] = 0xff000000;

	nes.loadROM(data);
	window.requestAnimationFrame(onAnimationFrame);
}

function handleProgress( e ) {
	if (e.lengthComputable) {
		//this.setState({ loadedPercent: e.loaded / e.total * 100 });
		console.log("loadPct=" + (e.loaded / e.total * 100) );
	}
}

function keyboardDown(event) {
	if (event.keyCode == KEYCODE.LEFT) {
		nes.buttonDown( 1, BUTTON.LEFT);
	}
	if (event.keyCode == KEYCODE.UP) {
		nes.buttonDown( 1, BUTTON.UP);
	}
	if (event.keyCode == KEYCODE.RIGHT) {
		nes.buttonDown( 1, BUTTON.RIGHT);
	}
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

 function keyboardUp(event){
	if (event.keyCode == KEYCODE.LEFT) {
		nes.buttonUp( 1, BUTTON.LEFT);
	}
	if (event.keyCode == KEYCODE.UP) {
		nes.buttonUp( 1, BUTTON.UP);
	}
	if (event.keyCode == KEYCODE.RIGHT) {
		nes.buttonUp( 1, BUTTON.RIGHT);
	}
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
