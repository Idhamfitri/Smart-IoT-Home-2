const labels = [];
const values = [];

const ctx = document.getElementById("airChart").getContext("2d");
const chart = new Chart(ctx, {
  type: "line",
  data: {
    labels,
    datasets: [{
      label: "Air Quality",
      data: values,
      borderWidth: 2
    }]
  }
});

async function refreshData() {
  const res = await fetch("/api/latest");
  const d = await res.json();
  if (!d.timestamp) return;

  document.getElementById("presence").innerText =
    d.presence ? "Detected" : "Not Detected";
  document.getElementById("temperature").innerText =
    d.temperature + " °C";
  document.getElementById("air").innerText = d.air_quality;
  document.getElementById("fan").innerText = d.fan_status;

  labels.push(new Date(d.timestamp).toLocaleTimeString());
  values.push(d.air_quality);

  if (labels.length > 10) {
    labels.shift();
    values.shift();
  }

  chart.update();
}

setInterval(refreshData, 5000);
