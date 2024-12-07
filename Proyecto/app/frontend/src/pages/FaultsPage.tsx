import { useState, useEffect } from "react";
import { fetchPageFaultsData } from "../api/pages_faults.api";
import FaultsChart from "../charts/FaultsChart";

function FaultsPage() {
  const [minorFaultsList, setMinorFaultsList] = useState<number[]>([]);
  const [majorFaultsList, setMajorFaultsList] = useState<number[]>([]);
  const [timeList, setTimeList] = useState<string[]>([]);

  useEffect(() => {
    const fetchData = async () => {
      const pageFaultsData = await fetchPageFaultsData();
      const currentTime = new Date().toLocaleTimeString();

      setMinorFaultsList((prev) => [...prev, pageFaultsData.minor]);
      setMajorFaultsList((prev) => [...prev, pageFaultsData.major]);
      setTimeList((prev) => [...prev, currentTime]);
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
          Fallos de Página
        </h1>
        <FaultsChart
          major={majorFaultsList}
          minor={minorFaultsList}
          time={timeList}
        />
      </div>
    </div>
  );
}

export default FaultsPage;
