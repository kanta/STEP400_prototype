// file-input
window.pressed = function(){
	var input = document.getElementById('file');
	if(input.value == "") {
		fileLabel.innerHTML = "Choose file";
	} else {
		var theSplit = input.value.split('\\');
		fileLabel.innerHTML = theSplit[theSplit.length-1];
	}
};