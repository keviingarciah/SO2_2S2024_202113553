import { useEffect, useState } from "react";
import PagesChart from "../charts/PagesChart";
import { fetchPageData, PageData } from "../api/pages.api";

function PagesPage() {
  const [pageData, setPageData] = useState<PageData>({
    active: 0,
    inactive: 0,
  });

  useEffect(() => {
    const getData = async () => {
      const data = await fetchPageData();
      setPageData(data);
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
          Páginas Activas e Inactivas
        </h1>
        <PagesChart active={pageData.active} inactive={pageData.inactive} />
      </div>
    </div>
  );
}

export default PagesPage;
