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

function keyboard(callback, event){
	// console.log(event);
	
	var player = 1;
	switch(event.keyCode){
		case 38: // UP
			callback(player, jsnes.Controller.BUTTON_UP); break;
		case 40: // Down
			callback(player, jsnes.Controller.BUTTON_DOWN); break;
		case 37: // Left
			callback(player, jsnes.Controller.BUTTON_LEFT); break;
		case 39: // Right
			callback(player, jsnes.Controller.BUTTON_RIGHT); break;
		case 65: // 'a' - qwerty, dvorak
		case 81: // 'q' - azerty
			callback(player, jsnes.Controller.BUTTON_A); break;
		case 83: // 's' - qwerty, azerty
		case 79: // ';' - dvorak
			callback(player, jsnes.Controller.BUTTON_B); break;
		case 9: // Tab
			callback(player, jsnes.Controller.BUTTON_SELECT); break;
		case 13: // Return
			callback(player, jsnes.Controller.BUTTON_START); break;
		default: break;
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

document.addEventListener('keydown', (event) => {keyboard(nes.buttonDown, event)});
document.addEventListener('keyup', (event) => {keyboard(nes.buttonUp, event)});
