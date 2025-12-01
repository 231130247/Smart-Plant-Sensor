document.getElementById("loginBtn").addEventListener("click", () => {
  const email = document.getElementById("email").value;
  const pass = document.getElementById("password").value;

  signInWithEmailAndPassword(auth, email, pass)
    .then(() => {
      loginForm.style.display = "none";
      dashboard.style.display = "block";
      listenToSensors();
    })
    .catch(err => alert("Login gagal: " + err.message));
});


document.getElementById("logoutBtn").addEventListener("click", () => {
  signOut(auth).then(() => {
    dashboard.style.display = "none";
    loginForm.style.display = "block";
  });
});


function listenToSensors() {
  const sensorRef = ref(db, "greenhouse/sensors");

  onValue(sensorRef, (snapshot) => {
    const d = snapshot.val() || {};

    updateSoil(d.soilMoisture);
    updateLight(d.lightLevel);
    updateMotion(d.motion);
  });
}

function updateSoil(value) {
  document.getElementById("soilMoisture").innerText = value + "%";
  const status = value < 30 ? "Kering" : value < 70 ? "Cukup" : "Basah";
  document.getElementById("soilStatus").innerText = status;
}

function updateLight(value) {
  document.getElementById("lightLevel").innerText = value + "%";
  const status = value < 30 ? "Redup" : value < 70 ? "Cukup" : "Terang";
  document.getElementById("lightStatus").innerText = status;
}

function updateMotion(value) {
  document.getElementById("motion").innerText = value ? "Terdeteksi" : "Tidak Terdeteksi";
  const status = value ? "Ada Gerakan" : "Aman";
  document.getElementById("motionStatus").innerText = status;
}
