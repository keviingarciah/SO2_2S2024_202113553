export interface PageFaultsData {
  minor: number;
  major: number;
}

export const fetchPageFaultsData = async (): Promise<PageFaultsData> => {
  try {
    const response = await fetch("http://localhost:8888/page-faults"); // Reemplaza 'URL_DE_TU_API' con la URL real de tu API
    const textData = await response.text();

    // Parse the plain text data
    const lines = textData.split("\n");
    const data = lines.reduce(
      (acc: { [key: string]: number }, line: string) => {
        if (line.trim() && line.includes(":")) {
          const [key, value] = line.split(":");
          acc[key.trim()] = parseInt(value.trim(), 10);
        }
        return acc;
      },
      {}
    );

    return {
      minor: data["Minor Page Faults"],
      major: data["Major Page Faults"],
    };
  } catch (error) {
    console.error("Error fetching page faults data:", error);
    return { minor: 0, major: 0 };
  }
};
