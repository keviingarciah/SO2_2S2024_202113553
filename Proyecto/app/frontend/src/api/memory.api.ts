// src/api/memory.api.ts
export interface MemoryData {
  free: number;
  used: number;
  cached: number;
}

export const fetchMemoryData = async (): Promise<MemoryData> => {
  try {
    const response = await fetch("http://localhost:8888/memory-usage"); // Reemplaza 'URL_DE_TU_API' con la URL real de tu API
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
      free: data["Free Memory"],
      used: data["Used Memory"],
      cached: data["Cached Memory"],
    };
  } catch (error) {
    console.error("Error fetching memory data:", error);
    return { free: 0, used: 0, cached: 0 };
  }
};
