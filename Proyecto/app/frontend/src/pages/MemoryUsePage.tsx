import { useEffect, useState } from "react";
import MemoryChart from "../charts/MemoryChart";
import { fetchMemoryData } from "../api/memory.api";

function MemoryUsePage() {
  const [memoryData, setMemoryData] = useState({ free: 0, used: 0, cached: 0 });

  useEffect(() => {
    const getData = async () => {
      const data = await fetchMemoryData();
      setMemoryData(data);
    };

    // Llamar a getData inmediatamente y luego cada segundo
    getData();
    const intervalId = setInterval(getData, 1000);

    // Limpiar el intervalo cuando el componente se desmonte
    return () => clearInterval(intervalId);
  }, []);

  return (
    <div className="flex flex-col items-center w-auto gap-32 mt-10">
      <div className="flex flex-col items-center  w-2/6 bg-white shadow-lg rounded-lg px-10 pt-8 pb-4 ">
        <h1 className="text-4xl font-bold text-docker-text mb-4">
          Uso de Memoria
        </h1>
        <MemoryChart
          free={memoryData.free}
          used={memoryData.used}
          cached={memoryData.cached}
        />
      </div>
    </div>
  );
}

export default MemoryUsePage;
