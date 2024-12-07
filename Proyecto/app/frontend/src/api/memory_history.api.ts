export const fetchUsedMemory = async (): Promise<number> => {
  try {
    const response = await fetch("http://localhost:8888/memory-usage");
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

    return data["Used Memory"];
  } catch (error) {
    console.error("Error fetching used memory:", error);
    return 0;
  }
};

export const fetchUsedSwap = async (): Promise<number> => {
  try {
    const response = await fetch("http://localhost:8888/swap-info");
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

    return data["Used Swap"];
  } catch (error) {
    console.error("Error fetching used swap:", error);
    return 0;
  }
};
