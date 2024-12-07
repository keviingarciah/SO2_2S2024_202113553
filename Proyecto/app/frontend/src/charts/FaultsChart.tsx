import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
} from "chart.js";
import { Line } from "react-chartjs-2";

ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend
);

interface HistoryChartProps {
  minor: number[];
  major: number[];
  time: string[];
}

function HistoryChart({ minor, major, time }: HistoryChartProps) {
  console.log(minor, major, time);

  const options = {
    responsive: true,
    plugins: {
      legend: {
        position: "top" as const,
      },
      title: {
        display: true,
        text: "Registro de los fallos de página a lo largo del tiempo.",
      },
    },
  };

  const data = {
    labels: time,
    datasets: [
      {
        label: "Fallos de Página Menores",
        data: minor,
        borderColor: "rgba(75, 192, 192)",
        backgroundColor: "rgba(75, 192, 192, 0.5)",
      },
      {
        label: "Fallos de Página Mayores",
        data: major,
        borderColor: "rgba(255, 159, 64)",
        backgroundColor: "rgba(255, 159, 64, 0.5)",
      },
    ],
  };

  return <Line options={options} data={data} />;
}

export default HistoryChart;
