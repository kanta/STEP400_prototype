// file-input
const formInputs = document.getElementsByClassName("file");
const length = formInputs.length;
for (let i = 0; i < length; i++) {
    formInputs[i].onchange = function () {
        const file = this.files[0].name;
        const label = this.nextElementSibling;
        if (!label.classList.contains("changed")) {
            const span = document.createElement("span");
            span.className = "filename";
            this.parentNode.appendChild(span);
            label.classList.add("changed");
        }
        label.nextElementSibling.innerHTML = file;
    };
}