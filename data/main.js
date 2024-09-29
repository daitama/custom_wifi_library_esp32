//mulai
import './icons.js';

console.log("Hello World");
let currentActiveDiv = null;

// const accessPoints = [{
//     ssid: "MERPATI 16",
//     securityType: "PSK",
//     networkStrengh: "80",
// },
// {
//     ssid: "MERPATI 16 LT2",
//     securityType: "open",
//     networkStrengh: "80",
// }];
const RENDER_EVENT = "render-ap";

const SAVED_EVENT = "saved-ap";

document.addEventListener("DOMContentLoaded", function () {
  const refreshButton = document.getElementById("refresh-btn");
  refreshButton.addEventListener("click", function () {
    loadAP();
  });

  loadAP();
});

function loadAP() {
  console.log(accessPoints);
  const accessPointList = document.getElementById("ap-list-container");
  accessPointList.innerHTML = "";

  for (const accessPointItem of accessPoints) {
    const accessPointElement = makeAP(accessPointItem);
    accessPointList.append(accessPointElement);
  }

  // Get all clickable divs
  const divs = document.querySelectorAll(".accessPoint");

  //Loop through each div and add a click event listener
  divs.forEach((div) => {
    div.addEventListener("click", () => {
      document.querySelectorAll(".APconnectButton").forEach((button) => {
        button.hidden = true;
      });
      div.querySelector(".APconnectButton").hidden = false;
    });
  });
}

function makeAP(accessPointObject) {
  //Create Container element
  const container = document.createElement("div");
  let index = accessPoints.indexOf(accessPointObject);
  container.setAttribute("id", `ap${index}`);
  container.setAttribute("class", "accessPoint");
  container.setAttribute("securityType", `${accessPointObject.securityType}`);

  //Create headerContainer element
  const labelContainer = document.createElement("div");
  labelContainer.setAttribute("class", "label-container");
  
  //Create WiFi Icon element based on signalStrength
  let wifiIcon;
switch (true) {
  case (accessPointObject.signalStrength > -60):
    wifiIcon = document.createElement("wifi-icon3");  // Custom element with hyphen
    // wifiIcon.setAttribute("class", "icon-wifi");
    break;
  case (accessPointObject.signalStrength > -70):
    wifiIcon = document.createElement("wifi-icon2");
    // wifiIcon.setAttribute("class", "icon-wifi");
    break;
  case (accessPointObject.signalStrength > -80):
    wifiIcon = document.createElement("wifi-icon1");
    // wifiIcon.setAttribute("class", "icon-wifi");
    break;
  default:
    wifiIcon = document.createElement("wifi-icon0");
    // wifiIcon.setAttribute("class", "icon-wifi");
}

  //Create lock Icon based on securityType
  let lockIcon;
  if(accessPointObject.securityType == 'open'){
    lockIcon = document.createElement("lock-icon1");
    // lockIcon.setAttribute("class", "icon-lock");
  }else{
    lockIcon = document.createElement("lock-icon0");
    // lockIcon.setAttribute("class", "icon-lock");
  }
 

  // Create the form element
  const form = document.createElement("form");
  form.setAttribute("class", "credentialAP");

  // Create the label and input for the Wi-Fi name
  const heading = document.createElement("h3");
  const label = document.createElement("label");
  label.innerText = `${accessPointObject.ssid}`;

  heading.append(label);
  labelContainer.append(wifiIcon);
  labelContainer.append(heading);
  labelContainer.append(lockIcon);

  // Create the password input field (hidden by default)
  const input = document.createElement("input");
  input.setAttribute("type", "password");
  input.setAttribute("class", "APpassField");
  input.setAttribute("placeholder", "Enter your Password");
  input.setAttribute("name", "passField");
  input.hidden = true;

  // Create the connect button (hidden by default)
  const button = document.createElement("button");
  button.setAttribute("class", "APconnectButton");
  button.setAttribute("type", "submit");
  button.innerText = "Connect";
  button.hidden = true;

  input.addEventListener("click", function (event) {
    event.stopPropagation();
  });

  //Add event listener for when the container(div) is clicked
  container.addEventListener("click", function () {
    // Hide all other connect buttons and password fields except for this container
    if (currentActiveDiv !== container) {
      document.querySelectorAll(".APpassField").forEach((field) => (field.hidden = true));
      document.querySelectorAll(".APconnectButton").forEach((btn) => (btn.hidden = true));

      // Show the connect button for the clicked div
      button.hidden = false;

      // Reset password field only if switching to a different container
      input.hidden = true;
      input.required = false;

      currentActiveDiv = container; // Update the currently active div
    }
  });

  // Append elements to the form
  form.append(labelContainer);
  form.append(input);
  form.append(button);

  form.addEventListener("submit", function (event) {
    event.preventDefault();
    event.stopPropagation(); // Prevents the container click event from firing
    if (accessPointObject.securityType != "open" && input.hidden == true) {
      input.hidden = false;
      input.required = true;
      console.log("input shown");
    } else {
      alert("Data Terkirim");

      //Logic untuk kirim data
      sendCredentialData(accessPointObject.ssid, input.value, typeof input.value);
    }
  });

  container.append(form);

  return container;
}

function sendCredentialData(ssid, pass, type) {
  console.log("SSID :" + ssid);
  console.log("Password : " + pass);
  console.log("Password type : " + type);
}
