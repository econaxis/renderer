// PRE

// obtains all obj files
// !!!
function fetchObjs(){
	var files = ["yeetus", "feetus", "reetus", "idk", "placeholder"];
	var objList = document.getElementById("obj-list");
	document.getElementById("item-rendered").innerHTML = files[0];
	files.forEach(function (i){
		objList.innerHTML += "<div class='dropdown-item choice'>" + i + "</div>";
	});
}

// ACTIONS
function togglePanel() {
	var togglePanelBtn = document.getElementById('toggle-panel');
	var clopen = togglePanelBtn.className;
	var controlPanel = document.getElementById("control-panel");
	var panelState = controlPanel.className;
	
	if(clopen.includes("closed")){
		togglePanelBtn.className = clopen.replace("closed", "opened");
		controlPanel.className = panelState.replace("compact", "expanded");
		document.querySelectorAll('#control-panel > .hidden').forEach(function(i) {
			i.className = i.className.replace("hidden", "shown");
		});
	} else if (clopen.includes("opened")){
		togglePanelBtn.className = clopen.replace("opened", "closed");
		controlPanel.className = panelState.replace("expanded", "compact");
		document.querySelectorAll('#control-panel > .shown').forEach(function(i) {
			i.className = i.className.replace("shown", "hidden");
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

