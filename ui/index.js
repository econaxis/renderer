// PRE

// obtains all obj files
// !!!
function fetchObjs(){
	var files = ["head", "teapot", "teddy"];
	var objList = document.getElementById("obj-list");
	document.getElementById("item-rendered").innerHTML = files[0];
	files.forEach(function (i){
		objList.innerHTML += "<div class='dropdown-item choice'>" + i + "</div>";
	});
}

// control tooltips
var tooltips = {
	"reflectance": "how reflective the object is",
	"shadows": "toggle object shadows on/off",
	"highlights": "toggle object highlights on/off"
}

window.onmouseover = function(event) {
	if(event.target.matches('.has-tooltip')){
		let key = event.target.innerHTML;
		event.target.innerHTML = "<span class='tooltiptext'>" + tooltips[key] + "</span>" + key;
	}
}

// keyEvents
// !!!
var keyEvents = new Map();
keyEvents.set("W", "camera back"); // writing "position" because its the longest word, fix as needed
keyEvents.set("S", "camera forwards");
keyEvents.set("A", "rotate camera");
keyEvents.set("D", "rotate camera");
keyEvents.set("spc", "rotate camera");
keyEvents.set("Q", "rotate camera");
keyEvents.set("up", "rotate object");
keyEvents.set("down", "rotate object");
keyEvents.set("left", "rotate object");
keyEvents.set("right", "rotate object");
keyEvents.set("shift Q", "stop rendering");


function getLegend(){
	console.log(keyEvents);
	var leg = document.getElementById("legend");
	keyEvents.forEach(function (key, value) {
		leg.innerHTML += 
			"<div class='legend-entry'> \
				<div class='highlight key'>" + value + "</div> \
				<div class='secondary description'>" + key + "</div> \
			</div>";
	});
}

// ACTIONS
function togglePanel() {
	var togglePanelBtn = document.getElementById('toggle-panel');
	var clopen = togglePanelBtn.className;
	var controlPanel = document.getElementById("control-panel");
	var panelState = controlPanel.className;
	var contentList = document.querySelectorAll('#control-panel > *:not(#bottom-controls)');
	
	if(clopen.includes("closed")){
		togglePanelBtn.className = clopen.replace("closed", "opened");
		controlPanel.className = panelState.replace("compact", "expanded");
		contentList.forEach(function(i) {
			if(!(i.id == "#bottom-controls")){
				i.style.marginBottom = "6%";
			}
			if(i.classList.contains("hidden")){
				i.className = i.className.replace("hidden", "shown");
			}
		});
	} else if (clopen.includes("opened")){
		togglePanelBtn.className = clopen.replace("opened", "closed");
		controlPanel.className = panelState.replace("expanded", "compact");
		contentList.forEach(function(i) {
			if(!(i.id == "#bottom-controls")){
				i.style.marginBottom = "3%";
			}
			if(i.classList.contains("shown")){
				i.className = i.className.replace("shown", "hidden");
			}
		});
	}
}

function showDropdown() {
	document.getElementById("obj-list").classList.toggle("shown");
}

// Close the dropdown if the user clicks outside of it
window.onclick = function(event) {
	var dropdownList = document.getElementById("obj-list");
	if (!event.target.matches('.dropdown-item')) {
		if (!event.target.matches('#item-rendered') && !event.target.matches('.dropdown-icon')) {
			if (dropdownList.classList.contains('shown')) {
				dropdownList.classList.remove('shown');
			}
		}
	} else if (!event.target.matches('#dropdown-item-main')) {
		swapObject(event.target.innerHTML);
		if (dropdownList.classList.contains('shown')) {
				dropdownList.classList.remove('shown');
		}
	}
}

// changes obj to be rendered
// !!!	
// @henry
function swapObject(name){
	var previous = document.getElementById("item-rendered").innerHTML;
	document.getElementById("item-rendered").innerHTML = name;
	
	// now acutally swap the objects
}

// reflectance parameter stuff
// !!!
function setReflectanceParameter(){
	var rp = document.getElementById("reflectance-slider").value;
}

// !!! 
function toggleShadows(){
	var shadowState = document.getElementById("shadow-toggle").checked;
	// now toggle shadow
}

// !!!
function toggleHighlights(){
	var highlightState = document.getElementById("shadow-toggle").checked;
	// now toggle highlights
}

// Full screen handling
function toggleFS(){
	document.getElementById("control-panel").classList.toggle("fs");
	document.querySelectorAll('.fs-icon').forEach(function (i){
		i.classList.toggle("hidden");
	});
}

// mobile begging
function prettyPrettyPlease(){
	let cats = document.getElementsByClassName("hidden cat");
	if (cats.length > 0){
		cats[0].classList.remove("hidden");
	} else {
		window.open('https://github.com/econaxis/renderer/');
	}
}