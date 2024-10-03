console.log("Hello World");
let currentActiveDiv = null;
let accessPoints;
// accessPoints = [{
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
    handleRefreshButton();
  });
  //handleGetStatus();//tambahkan method untuk ambil status wifi connected!
  handleGetNetwork(); //uncomment ketika pake ESP32
  // document.dispatchEvent(new Event(RENDER_EVENT));
});

document.addEventListener(RENDER_EVENT, () => {
  loadAP();
});

function handleGetNetwork() {
  fetch("/getNetworks")
    .then((response) => response.text())
    .then((data) => {
      accessPoints = JSON.parse(data);
      document.dispatchEvent(new Event(RENDER_EVENT));
    });
}

function handleRefreshButton() {
  fetch("/", {
    method: "POST",
    headers: { "Content-type": "application/x-www-form-urlencoded" },
    body: "message=go",
  })
    .then((response) => response.text())
    .then((data) => {
      accessPoints = JSON.parse(data);
      document.dispatchEvent(new Event(RENDER_EVENT));
    });
}

function loadAP() {
  console.log(accessPoints);
  const accessPointList = document.getElementById("ap-list-container");
  accessPointList.innerHTML = "";

  for (const accessPointItem of accessPoints) {
    const accessPointElement = makeAP(accessPointItem);
    accessPointList.append(accessPointElement);
  }
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
  const wifiIcon = document.createElement("object");
  wifiIcon.setAttribute("type", "image/svg+xml");
  wifiIcon.setAttribute("class", "icon-wifi");
  switch (true) {
    case parseInt(accessPointObject.signalStrength, 10) > -60:
      wifiIcon.setAttribute("data", "wifi-0.svg"); // Custom element with hyphen
      // wifiIcon.setAttribute("class", "icon-wifi");
      break;
    case parseInt(accessPointObject.signalStrength, 10) > -70:
      wifiIcon.setAttribute("data", "wifi-3.svg");
      // wifiIcon.setAttribute("class", "icon-wifi");
      break;
    case parseInt(accessPointObject.signalStrength, 10) > -80:
      wifiIcon.setAttribute("data", "wifi-2.svg");
      // wifiIcon.setAttribute("class", "icon-wifi");
      break;
    default:
      wifiIcon.setAttribute("data", "wifi-1.svg");
    // wifiIcon.setAttribute("class", "icon-wifi");
  }

  //Create lock Icon based on securityType
  const lockIcon = document.createElement("object");
  lockIcon.setAttribute("type", "image/svg+xml");
  lockIcon.setAttribute("class", "icon-lock");
  if (accessPointObject.securityType == "open") {
    lockIcon.setAttribute("data", "lock-open.svg");
    // lockIcon.setAttribute("class", "icon-lock");
  } else {
    lockIcon.setAttribute("data", "lock.svg");
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
  input.setAttribute("placeholder", "Enter WiFi Password");
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
  const apData = new URLSearchParams();
  apData.append("ssid", ssid);
  apData.append("password", pass);
  fetch("/", {
    method: "POST",
    headers: {
      "Content-Type": "application/x-www-form-urlencoded", // Specify the content type
    },
    body: apData.toString(),
  })
    .then((response) => {
      if (!response.ok) {
        throw new Error("Network response was not ok");
      }
      return response.text(); // Assuming the server responds with text
    })
    .then((data) => {
      alert(data);
    });
}
