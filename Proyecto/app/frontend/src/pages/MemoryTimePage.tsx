import { useState, useEffect } from "react";
import HistoryChart from "../charts/MemoryHistoryChart";
import { fetchUsedMemory, fetchUsedSwap } from "../api/memory_history.api";

function MemoryTimePage() {
  const [usedMemoryList, setUsedMemoryList] = useState<number[]>([]);
  const [usedSwapList, setUsedSwapList] = useState<number[]>([]);
  const [timeList, setTimeList] = useState<string[]>([]);

  useEffect(() => {
    const fetchData = async () => {
      const usedMemory = await fetchUsedMemory();
      const usedSwap = await fetchUsedSwap();
      const currentTime = new Date().toLocaleTimeString();

      setUsedMemoryList((prev) => [...prev, usedMemory]);
      setUsedSwapList((prev) => [...prev, usedSwap]);
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
          Historial de Memoria
        </h1>
        <HistoryChart
          physicalMemory={usedMemoryList}
          swapMemory={usedSwapList}
          time={timeList}
        />
      </div>
    </div>
  );
}

export default MemoryTimePage;
