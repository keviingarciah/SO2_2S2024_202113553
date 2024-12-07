import { useEffect, useState } from "react";
import ProcessesChart from "../charts/ProcessesChart";
import { fetchTopMemoryProcesses } from "../api/processes.api";

function ProcessesPage() {
  const [processValues, setProcessValues] = useState<number[]>([]);
  const [processLabels, setProcessLabels] = useState<string[]>([]);

  useEffect(() => {
    const fetchData = async () => {
      const processes = await fetchTopMemoryProcesses();
      setProcessValues(processes.map((process) => process.memory));
      setProcessLabels(processes.map((process) => process.command));
    };

    // Llamar a fetchData inmediatamente y luego cada segundo
    fetchData();
    const intervalId = setInterval(fetchData, 1000);

    // Limpiar el intervalo cuando el componente se desmonte
    return () => clearInterval(intervalId);
  }, []);

  return (
    <div className="flex flex-col items-center w-auto gap-32 mt-10">
      <div className="flex flex-col items-center  w-3/5 bg-white shadow-lg rounded-lg px-10 pt-8 pb-4 ">
        <h1 className="text-4xl font-bold text-docker-text mb-4">
          Top Procesos en ejecución
        </h1>
        <ProcessesChart values={processValues} labels={processLabels} />
      </div>
    </div>
  );
}

export default ProcessesPage;
