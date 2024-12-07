// src/api/pages.api.ts
export interface PageData {
  active: number;
  inactive: number;
}

export const fetchPageData = async (): Promise<PageData> => {
  try {
    const response = await fetch("http://localhost:8888/active-inactive-pages"); // Reemplaza 'URL_DE_TU_API' con la URL real de tu API
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
      active: data["Active Pages"],
      inactive: data["Inactive Pages"],
    };
  } catch (error) {
    console.error("Error fetching page data:", error);
    return { active: 0, inactive: 0 };
  }
};
