import { Chart as ChartJS, ArcElement, Tooltip, Legend } from "chart.js";
import { Pie } from "react-chartjs-2";

ChartJS.register(ArcElement, Tooltip, Legend);

interface LiveChartProps {
  used: number;
  free: number;
  cached: number;
}

function MemoryChart({ used, free, cached }: LiveChartProps) {
  const data = {
    labels: [`Memoria en Uso`, `Memoria Libre`, `Memoria Caché`],
    datasets: [
      {
        label: "Memoria en Kilobytes",
        data: [used, free, cached],
        backgroundColor: [
          "rgba(255, 99, 132, 0.6)",
          "rgba(54, 162, 235, 0.6)",
          "rgba(255, 206, 86, 0.6)",
        ],
        borderColor: [
          "rgba(255, 99, 132, 1)",
          "rgba(54, 162, 235, 1)",
          "rgba(255, 206, 86, 1)",
        ],
        borderWidth: 1,
      },
    ],
  };

  return <Pie data={data} />;
}

export default MemoryChart;
