// Chart.js interop functions for pet status graphs

// Create a combined chart with all pet stats
function createCombinedChart(
  canvasId,
  labels,
  hungerData,
  happinessData,
  tirednessData
) {
  // Get the canvas element
  const ctx = document.getElementById(canvasId);

  // Destroy any existing chart on this canvas
  if (window.petCharts && window.petCharts[canvasId]) {
    window.petCharts[canvasId].destroy();
  }

  // Initialize charts storage if not exists
  if (!window.petCharts) {
    window.petCharts = {};
  }

  // Create new chart
  window.petCharts[canvasId] = new Chart(ctx, {
    type: "line",
    data: {
      labels: labels,
      datasets: [
        {
          label: "Hunger",
          data: hungerData,
          borderColor: "rgb(255, 99, 132)",
          backgroundColor: "rgba(255, 99, 132, 0.1)",
          tension: 0.1,
          fill: true,
        },
        {
          label: "Happiness",
          data: happinessData,
          borderColor: "rgb(75, 192, 192)",
          backgroundColor: "rgba(75, 192, 192, 0.1)",
          tension: 0.1,
          fill: true,
        },
        {
          label: "Tiredness",
          data: tirednessData,
          borderColor: "rgb(255, 159, 64)",
          backgroundColor: "rgba(255, 159, 64, 0.1)",
          tension: 0.1,
          fill: true,
        },
      ],
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        y: {
          beginAtZero: true,
          max: 100,
          title: {
            display: true,
            text: "Value",
          },
        },
        x: {
          title: {
            display: true,
            text: "Time",
          },
        },
      },
      plugins: {
        title: {
          display: true,
          text: "Pet Status Over Time",
          font: {
            size: 16,
          },
        },
        legend: {
          position: "top",
        },
        tooltip: {
          mode: "index",
          intersect: false,
        },
      },
    },
  });
}

// Create a single stat chart (for individual metrics)
function createSingleStatChart(canvasId, labels, data, color) {
  // Get the canvas element
  const ctx = document.getElementById(canvasId);

  // Destroy any existing chart on this canvas
  if (window.petCharts && window.petCharts[canvasId]) {
    window.petCharts[canvasId].destroy();
  }

  // Initialize charts storage if not exists
  if (!window.petCharts) {
    window.petCharts = {};
  }

  // Create new chart
  window.petCharts[canvasId] = new Chart(ctx, {
    type: "line",
    data: {
      labels: labels,
      datasets: [
        {
          data: data,
          borderColor: color,
          backgroundColor: color.replace(")", ", 0.1)").replace("rgb", "rgba"),
          tension: 0.1,
          fill: true,
        },
      ],
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        y: {
          beginAtZero: true,
          max: 100,
        },
        x: {
          display: false,
        },
      },
      plugins: {
        legend: {
          display: false,
        },
        tooltip: {
          callbacks: {
            label: function (context) {
              return `Value: ${context.raw}`;
            },
          },
        },
      },
    },
  });
}
