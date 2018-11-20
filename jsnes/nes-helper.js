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

var SCREEN_WIDTH = 256;
var SCREEN_HEIGHT = 240;
var FRAMEBUFFER_SIZE = SCREEN_WIDTH*SCREEN_HEIGHT;

var ctx, image;
var framebuffer_u8, framebuffer_u32;

var nes = new jsnes.NES({
	onFrame: function(framebuffer){for(var i = 0; i < FRAMEBUFFER_SIZE; i++) framebuffer_u32[i] = 0xFF000000 | framebuffer[i];},
	onAudioSample: function(left, right){}
});

function onAnimationFrame(){
	window.requestAnimationFrame(onAnimationFrame);
	
	image.data.set(framebuffer_u8);
	ctx.putImageData(image, 0, 0);
	nes.frame();
}

function loadBinary(path, callback) {
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
	
	req.send();
	
	return req;
}

function handleLoaded(data){
	var canvas = document.getElementById("nes-canvas");
	ctx = canvas.getContext("2d");
	image = ctx.getImageData(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	ctx.fillStyle = "black";
	ctx.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	// Allocate framebuffer array.
	var buf = new ArrayBuffer(image.data.length);
	framebuffer_u8 = new Uint8ClampedArray(buf);
	framebuffer_u32 = new Uint32Array(buf);

	nes.loadROM(data);
	window.requestAnimationFrame(onAnimationFrame);
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

function initNES(nes_file, canvas_id){
	var currentRequest = loadBinary(
		nes_file,
		(err, data) => {
			if (err) {
				window.alert(`Error loading ROM: ${err.toString()}`);
			} else {
				handleLoaded(data);
			}
		},
	);
}

document.addEventListener('keydown', keyboardDown);
document.addEventListener('keyup', keyboardUp);
