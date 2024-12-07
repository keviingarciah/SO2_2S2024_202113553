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
  physicalMemory: number[];
  swapMemory: number[];
  time: string[];
}

function HistoryChart({ physicalMemory, swapMemory, time }: HistoryChartProps) {
  console.log(physicalMemory, swapMemory, time);

  const options = {
    responsive: true,
    plugins: {
      legend: {
        position: "top" as const,
      },
      title: {
        display: true,
        text: "Gráfica de memoria usada a lo largo del tiempo.",
      },
    },
  };

  const data = {
    labels: time,
    datasets: [
      {
        label: "Uso de Memoria Física",
        data: physicalMemory,
        borderColor: "rgba(75, 192, 192)",
        backgroundColor: "rgba(75, 192, 192, 0.5)",
      },
      {
        label: "Uso de Memoria Swap",
        data: swapMemory,
        borderColor: "rgba(255, 159, 64)",
        backgroundColor: "rgba(255, 159, 64, 0.5)",
      },
    ],
  };

  return <Line options={options} data={data} />;
}

export default HistoryChart;
