export interface ProcessData {
  pid: number;
  memory: number;
  command: string;
}

export const fetchTopMemoryProcesses = async (): Promise<ProcessData[]> => {
  try {
    const response = await fetch("http://localhost:8888/top-memory-processes"); // Reemplaza 'URL_DE_TU_API' con la URL real de tu API
    const textData = await response.text();

    console.log("Raw text data:", textData);

    // Parse the plain text data
    const lines = textData.split("\n").slice(1); // Skip the first line
    console.log("Lines after split and slice:", lines);

    const data = lines
      .filter((line) => line.trim() !== "") // Filtrar líneas vacías
      .map((line) => {
        console.log("Processing line:", line);
        const parts = line.split(", ");
        if (parts.length !== 3) {
          console.error(`Unexpected line format: ${line}`);
          return null;
        }

        const [pidPart, memoryPart, commandPart] = parts;
        console.log("Parts:", { pidPart, memoryPart, commandPart });

        const pid = parseInt(pidPart.split(": ")[1], 10);
        const memory = parseInt(memoryPart.split(": ")[1], 10);
        const command = commandPart.split(": ")[1];

        console.log("Parsed values:", { pid, memory, command });

        if (isNaN(pid) || isNaN(memory) || !command) {
          console.error(`Invalid data in line: ${line}`);
          return null;
        }

        return { pid, memory, command };
      })
      .filter((process) => process !== null) as ProcessData[];

    console.log("Parsed data:", data);
    return data;
  } catch (error) {
    console.error("Error fetching top memory processes:", error);
    return [];
  }
};
