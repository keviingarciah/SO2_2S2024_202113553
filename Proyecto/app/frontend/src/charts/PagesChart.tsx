import { Chart as ChartJS, ArcElement, Tooltip, Legend } from "chart.js";
import { Pie } from "react-chartjs-2";

ChartJS.register(ArcElement, Tooltip, Legend);

interface LiveChartProps {
  active: number;
  inactive: number;
}

function MemoryChart({ active, inactive }: LiveChartProps) {
  const data = {
    labels: [`Páginas Activas`, `Páginas Inactivas`],
    datasets: [
      {
        label: "Memoria en Kilobytes",
        data: [active, inactive],
        backgroundColor: ["rgba(255, 99, 132, 0.6)", "rgba(54, 162, 235, 0.6)"],
        borderColor: ["rgba(255, 99, 132, 1)", "rgba(54, 162, 235, 1)"],
        borderWidth: 1,
      },
    ],
  };

  return <Pie data={data} />;
}

export default MemoryChart;
